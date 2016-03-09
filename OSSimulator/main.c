//
//  main.c
//  OSSimulator
//
//  Created by Costas Vrahimis on 2/17/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "CircularLinkedList.h"

#include "fifoscheduler.h"
#define SCHEDULER fifoscheduler

#define LOAD_PROCESSES_FROM_FILE 1
#define EXECUTION_CONDITION (sharedResource->scheduler->readyQueueStart->next != sharedResource->scheduler->readyQueueStart || sharedResource->startDataSize > 0 || sharedResource->cpuState == CPU_STATE_RUNNING)
#define EXECUTION_CONDITION_EMPTY (sharedResource->scheduler->readyQueueStart->next != sharedResource->scheduler->readyQueueStart || sharedResource->startDataSize > 0 || sharedResource->cpuState == CPU_STATE_FINISHED)
//#define EXECUTION_CONDITION (sharedResource->time < 100)


#define CPU_STATE_FINISHED 0
#define CPU_STATE_RUNNING  1

pthread_mutex_t lock;

typedef struct resources{
    SCHEDULER *scheduler;
    int time;
    circularlistnode *startData;
    int startDataSize;
    circularlistnode *doneQ;
    int cpuState;
    int nextPid;
    int timeSlice;
}sharedRes;

void synchronizedSchedule(sharedRes *sharedResource, process *proc){
    pthread_mutex_lock(&lock);
    schedule(sharedResource->scheduler, proc, sharedResource->time);
    pthread_mutex_unlock(&lock);
}

process* synchronizedNextProcess(fifoscheduler* scheduler){
    pthread_mutex_lock(&lock);
    process* nextProc = nextProcess(scheduler);
    pthread_mutex_unlock(&lock);
    return nextProc;
}

/**
 * @param {double} probability - probability of creating a new process, double between 0 and 1
 * @param {sharedRes *} sharedResource - pointer to the shared resource for the OS
 * @return {process *} - a new random process or NULL, based on probability
 */
process *generateRandomProcess(double probability, int minRunTime, int maxRunTime, sharedRes *sharedResource)//int currentTime)
{
    double randomValue = (double)rand() / RAND_MAX;
    if (randomValue >= (1 - probability)) {
        process *newProc = (process *)malloc(sizeof(process));
        pthread_mutex_lock(&lock);
        newProc->pID = sharedResource->nextPid++;
        pthread_mutex_unlock(&lock);
        newProc->entryTime = sharedResource->time;//currentTime;
        int runTime = rand() % (maxRunTime - minRunTime) + minRunTime;
        newProc->runTime = runTime;
        return newProc;
    }
    return NULL;
}

void *cpu(void *arg){
    sharedRes *sharedResource = (sharedRes *) arg;
    process *currentProcess = NULL;
    while (EXECUTION_CONDITION)
    {
        if (sharedResource->cpuState == CPU_STATE_FINISHED && sharedResource->scheduler->readyQueueSize > 0) {
            currentProcess = synchronizedNextProcess(sharedResource->scheduler);
            if (currentProcess != NULL) {
                currentProcess->timeEnteredCPU = sharedResource->time;
                pthread_mutex_lock(&lock);
                sharedResource->cpuState = CPU_STATE_RUNNING;
                pthread_mutex_unlock(&lock);
                printf("Running pID: %d \n", currentProcess->pID);
            }
        }
        if (currentProcess != NULL && currentProcess->runTime <= (sharedResource->time - currentProcess->timeEnteredCPU)) {
            currentProcess->timeDone = sharedResource->time;
            pthread_mutex_lock(&lock);
            insertBack(sharedResource->doneQ, currentProcess);
            sharedResource->cpuState = CPU_STATE_FINISHED;
            pthread_mutex_unlock(&lock);
            printf("Added pID: %d to the doneQ \n", currentProcess->pID);
            currentProcess = NULL;
        }
    }
    
    pthread_exit(NULL);
}

void *cpuRR(void *arg){
    sharedRes *sharedResource = (sharedRes *) arg;
    process *currentProcess = NULL;
    //NOTE readyQSize is grator than 0 at the end of execution
    while (EXECUTION_CONDITION)
    {
        if (sharedResource->cpuState == CPU_STATE_FINISHED && /*sharedResource->readyQsize > 0*/ sharedResource->scheduler->readyQueueStart != sharedResource->scheduler->readyQueueStart->next) {
            pthread_mutex_lock(&lock);
            sharedResource->cpuState = CPU_STATE_RUNNING;
            pthread_mutex_unlock(&lock);
            currentProcess = synchronizedNextProcess(sharedResource->scheduler);
            currentProcess->timeEnteredCPU = sharedResource->time;
            printf("cpuRRThread Running pID: %d \n", currentProcess->pID);
        }
        if (currentProcess != NULL && currentProcess->runTime <= (sharedResource->time - currentProcess->timeEnteredCPU) && sharedResource->cpuState) {
            currentProcess->timeDone = sharedResource->time;
            insertBack(sharedResource->doneQ, currentProcess);
            pthread_mutex_lock(&lock);
            sharedResource->cpuState = CPU_STATE_FINISHED;
            pthread_mutex_unlock(&lock);
            printf("cpuRRThread Added pID: %d to the doneQ \n", currentProcess->pID);
            currentProcess = NULL;
        }
        if(currentProcess != NULL && sharedResource->cpuState == CPU_STATE_RUNNING && sharedResource->timeSlice <= (sharedResource->time - currentProcess->timeEnteredCPU)){
            currentProcess->runTime -= sharedResource->timeSlice;
            if (currentProcess->runTime <= 0){
                currentProcess->runTime = 0;
                currentProcess->timeDone = sharedResource->time;
                insertBack(sharedResource->doneQ, currentProcess);
                printf("cpuRRThread Added pID: %d to the doneQ \n", currentProcess->pID);
            }
            else{
                synchronizedSchedule(sharedResource, currentProcess);
                printf("cpuRRThread Added pID: %d back to the ReadyQ \n", currentProcess->pID);
            }
            sharedResource->cpuState = CPU_STATE_FINISHED;
            currentProcess = NULL;
        }
    }
    pthread_exit(NULL);
}

void *cpuClock(void *arg){
    sharedRes *sharedResource = (sharedRes *) arg;
    while (EXECUTION_CONDITION) {
        pthread_mutex_lock(&lock);
        sharedResource->time++;
        pthread_mutex_unlock(&lock);
        //printf("Tick: %d \n", t);
        if (!LOAD_PROCESSES_FROM_FILE) {
            // On every tick, randomly choose whether or not to create a process.
            process *proc = generateRandomProcess(0.3, 1, 10, sharedResource);//->time);
            if (proc != NULL) {
                printf("Scheduling process %d\n", proc->pID);
                //pthread_mutex_lock(&lock);
                synchronizedSchedule(sharedResource, proc);
                //pthread_mutex_unlock(&lock);
            }
        }
        sleep(1);
    }
    
    pthread_exit(NULL);
}
//problem in this method initdata being null
void *scheduleInitialData(void *arg) {
    sharedRes *sharedResource = (sharedRes *) arg;
    circularlistnode *initData = sharedResource->startData;
    while (sharedResource->startDataSize > 0) {
        if (initData != NULL && initData != sharedResource->startData && initData->current && initData->current->entryTime <= sharedResource->time) {
            synchronizedSchedule(sharedResource, initData->current);
            del(sharedResource->startData, initData->current->pID);
            sharedResource->startDataSize--;
            printf("Added pID: %d to the ReadyQ \n", (initData->current)->pID);
        }
        if (initData != NULL && initData->next != NULL)
            initData = initData->next;
        else
            initData = sharedResource->startData;
    }
    //if (initData != NULL && initData->next != NULL)
        //initData = initData->next;

    //print(sharedResource->readyQstart, (sharedResource->readyQstart)->next);
    pthread_exit(NULL);
}

void loadProcessesFromFile(char *fileName, sharedRes *sharedResource) {
    circularlistnode *start = sharedResource->startData;
    
    FILE *fp;
    char buff[255];
    fp=fopen(fileName, "r");
    int i = 0;
    process *newProc = NULL;
    while (fscanf(fp, "%s", buff) != EOF) {
        if (i % 3 == 0)
        {
            newProc = (process *) malloc(sizeof(process));
            newProc->pID = atoi(buff);
        }
        else if (i % 3 == 1)
        {
            newProc->entryTime = atoi(buff);
        }
        else
        {
            newProc->runTime = atoi(buff);
            insertBack(start, newProc);
            sharedResource->startDataSize++;
        }
        i++;
    }

    print(start, start->next);
    printf("\n=================================\n");
    sharedResource->startData = start;
}

int main(int argc, const char * argv[]) {
    int rc;
    pthread_t clockThread;
    pthread_t readyQThread;
    pthread_t cpuThread;
    
    circularlistnode *doneQ;
    doneQ = (circularlistnode *)malloc(sizeof(circularlistnode));
    doneQ -> next = doneQ;
    doneQ -> prev = doneQ;
    
    circularlistnode *start;
    start = (circularlistnode *)malloc(sizeof(circularlistnode));
    start -> next = start;
    start -> prev = start;
    
    SCHEDULER *scheduler = (SCHEDULER *)malloc(sizeof(SCHEDULER));
    init_scheduler(scheduler);
    
    sharedRes *sharedResource = (sharedRes *) malloc(sizeof(sharedRes));
    sharedResource->scheduler = scheduler;
    sharedResource->time = 0;
    sharedResource->startDataSize = 0;
    sharedResource->doneQ = doneQ;
    sharedResource->cpuState = 0;
    sharedResource->startData = start;
    sharedResource->timeSlice = 4;
    
    pthread_mutex_init(&lock, NULL);

    // Seed the random function.
    srand((unsigned int)time(NULL));

    if (LOAD_PROCESSES_FROM_FILE)
        //loadProcessesFromFile("/Users/Sean/Documents/College/Spring 2016/Advanced OS/OSSimulator/OSSimulator/dataFile.txt", sharedResource);
        loadProcessesFromFile("/Users/cvrahimis/Documents/CSAdvOS/CProjects/OSSimulator/OSSimulator/dataFile.txt", sharedResource);
    
    rc = pthread_create(&clockThread, NULL, cpuClock, (void *)sharedResource);
    if(rc)
    {
        printf("ERROR. Return code from thread %d\n", rc);
        exit(-1);
    }
    if (LOAD_PROCESSES_FROM_FILE) {
        rc = pthread_create(&readyQThread, NULL, scheduleInitialData, (void *)sharedResource);
        if(rc)
        {
            printf("ERROR. Return code from thread %d\n", rc);
            exit(-1);
        }
    }
    //rc = pthread_create(&cpuThread, NULL, cpu, (void *)sharedResource);
    rc = pthread_create(&cpuThread, NULL, cpuRR, (void *)sharedResource);
    if(rc)
    {
        printf("ERROR. Return code from thread %d\n", rc);
        exit(-1);
    }

    rc = pthread_join(clockThread, NULL);
    if(rc)
    {
        printf("Error, return code from thread_join() is %d\n", rc);
        exit(-1);
    }
    if (LOAD_PROCESSES_FROM_FILE) {
        rc = pthread_join(readyQThread, NULL);
        if(rc)
        {
            printf("Error, return code from thread_join() is %d\n", rc);
            exit(-1);
        }
    }
    rc = pthread_join(cpuThread, NULL);
    if(rc)
    {
        printf("Error, return code from thread_join() is %d\n", rc);
        exit(-1);
    }
    printf("main completed join with thread clock thread \n");
    printf("main completed join with thread readyQ thread \n");
    printf("main completed join with thread cpu thread \n");
    
    printData(sharedResource->doneQ, sharedResource->doneQ->next);
    
    int fclose(FILE *fp);
    
    pthread_mutex_destroy(&lock);
    return 0;
}
