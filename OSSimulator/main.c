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
#include <math.h>
#include "CircularLinkedList.h"

#include "fifoscheduler.h"
#include "priorityscheduler.h"
#include "o1scheduler.h"

#define MAX_RAND_GEN_PROCS 10
#define LOAD_PROCESSES_FROM_FILE 0
#define IS_ROUND_ROBIN 1

//#define EXECUTION_CONDITION (sharedResource->time < 300)
#define EXECUTION_CONDITION (sharedResource->doneQSize != MAX_RAND_GEN_PROCS)

#define CPU_STATE_FINISHED 0
#define CPU_STATE_RUNNING  1
#define NUM_OF_MEM 128
#define PROBABILITY_INTERACTIVE 0.5

pthread_mutex_t lock;

typedef struct page{
    int startAddress;
    int size;
    struct page *next;
}page;

typedef struct resources{
    int nextPid;
    int time;
    priorityscheduler *scheduler;
    circularlistnode *doneQ;
    int doneQSize;
    circularlistnode *waitQ;
    circularlistnode *startData;
    int startDataSize;
    int numOfRandGenProcs;
    page *freeMemoryArray;
}sharedRes;

int allocate(sharedRes *sharedResource, process *proc){
    int originalIndex = ceil(log2(proc->requiredMemoryPages));
    int index = originalIndex;
    page *contigiousPages = &sharedResource->freeMemoryArray[index];
    while (contigiousPages == NULL) {
        index++;
        contigiousPages = &sharedResource->freeMemoryArray[index];
    }
    page* currentPage = contigiousPages;
    while(originalIndex <= index)
    {
        index--;
        int buddySize = contigiousPages->size / 2;
        page *buddy1 = (page *)malloc(sizeof(page));
        buddy1->startAddress = currentPage->startAddress;
        buddy1->size = buddySize;
        page *buddy2 = (page *)malloc(sizeof(page));
        buddy2->startAddress = buddy1->startAddress + buddy1->size;
        buddy2->size = buddySize;
        buddy1->next = buddy2;
        sharedResource->freeMemoryArray[index] = *buddy1;
        currentPage = buddy1;
    }
    if (&sharedResource->freeMemoryArray[originalIndex] != NULL) {
        proc->memoryPages = &sharedResource->freeMemoryArray[originalIndex];
        while (proc->memoryPages->next != NULL) {
            proc->memoryPages = proc->memoryPages->next;
        }
        proc->hasBeenAllocatedMemory = 1;
        return 1;
    }
    else
    {
        proc->hasBeenAllocatedMemory = 0;
        return 0;
    }
}


void synchronizedSchedule(sharedRes *sharedResource, process *proc){
    //if (!proc->hasBeenAllocatedMemory)
        //allocate(sharedResource, proc);
    pthread_mutex_lock(&lock);
    pr_schedule(sharedResource->scheduler, proc, sharedResource->time);
    pthread_mutex_unlock(&lock);
}

process* synchronizedNextProcess(priorityscheduler* scheduler){
    pthread_mutex_lock(&lock);
    process* nextProc = pr_nextProcess(scheduler);
    pthread_mutex_unlock(&lock);
    return nextProc;
}

int generateRandomNumberOfMemoryPages(){
    return ((int)rand() % 40) + 2;
    //return 31;
}

double generateProbabilityOfSystemCall(double probabilityInteractive)
{
    long randMiddle = RAND_MAX / 2;
    if ((double)rand() / RAND_MAX > probabilityInteractive)
        return (double)(rand() % randMiddle) / RAND_MAX;
    return (double)((rand() % randMiddle) + randMiddle) / RAND_MAX;
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
        // Randomly choose if we want an interactive process.
        newProc->probSystemCall = generateProbabilityOfSystemCall(PROBABILITY_INTERACTIVE);
        newProc->priority = rand() % MAX_PRIORITY;
        newProc->timeEnteredCPU = -1;
        newProc->timeEnteredReadyQ = -1;
        if (IS_ROUND_ROBIN)
            newProc->timeSlice = 4;
        else
            newProc->timeSlice = (MAX_PRIORITY - newProc->priority) * 2;
        newProc->requiredMemoryPages = generateRandomNumberOfMemoryPages();
        newProc->hasBeenAllocatedMemory = 0;
        return newProc;
    }
    return NULL;
}

void *cpu(void *arg){
    sharedRes *sharedResource = (sharedRes *) arg;
    process *currentProcess = NULL;
    int oldTime = sharedResource->time;
    while (EXECUTION_CONDITION)
    {
        // If the CPU is free...
        if (currentProcess == NULL) {
            // If we finished our last process, and there are still processes on the ready queue, schedule another one.
            if (sharedResource->scheduler->readyQueueSize > 0) {
                currentProcess = synchronizedNextProcess(sharedResource->scheduler);
                if (currentProcess != NULL) {
                    if (currentProcess->timeEnteredCPU == -1)
                        currentProcess->timeEnteredCPU = sharedResource->time;
                    pthread_mutex_lock(&lock);
                    pthread_mutex_unlock(&lock);
                    printf("Running pID: %d With Run Time: %d Time: %d \n", currentProcess->pID, currentProcess->runTime, sharedResource->time);
                }
            }
        }
        // Otherwise, if a process is running...
        else {
            // If the process's run time is up, remove it from the CPU.
            if (currentProcess->runTime <= (sharedResource->time - currentProcess->timeEnteredCPU)) {
                currentProcess->timeDone = sharedResource->time;
                pthread_mutex_lock(&lock);
                insertBack(sharedResource->doneQ, currentProcess);
                sharedResource->doneQSize++;
                pthread_mutex_unlock(&lock);
                printf("Added pID: %d to the doneQ \n", currentProcess->pID);
                currentProcess = NULL;
            }
            // Otherwise, we'll do some other things...
            else {
                // Randomly decide whether or not to generate a system call based on the process's probability of making a system call.
                if (oldTime != sharedResource->time) {
                    double randomValue = (double)rand() / RAND_MAX;
                    if (randomValue < currentProcess->probSystemCall) {
                        int updatedRunTime = (currentProcess->runTime - (sharedResource->time - currentProcess->timeEnteredCPU));
                        currentProcess->runTime = updatedRunTime < 0 ? 0 : updatedRunTime; //making sure the runTime cannot be negative
                        
                        currentProcess->timeInterrupt = ((int)rand() % 6) + 3;//setting the time it takes for the interrupt to complete
                        currentProcess->timeEnteredWaitQ = sharedResource->time;
                        cll_enqueue(sharedResource->waitQ, currentProcess);
                        printf("System call made by pID %d, added to the waitQ \n", currentProcess->pID);
                        currentProcess = NULL;
                    }
                }
                
                // Round robin
                if (IS_ROUND_ROBIN && currentProcess != NULL && currentProcess->timeSlice != -1  && currentProcess->timeSlice <= (sharedResource->time - currentProcess->timeEnteredCPU)){
                        currentProcess->runTime -= currentProcess->timeSlice;
                        if (currentProcess->runTime <= 0){
                            currentProcess->runTime = 0;
                            currentProcess->timeDone = sharedResource->time;
                            insertBack(sharedResource->doneQ, currentProcess);
                            sharedResource->doneQSize++;
                            printf("cpuRR Added pID: %d to the doneQ \n", currentProcess->pID);
                        } else {
                            synchronizedSchedule(sharedResource, currentProcess);
                            printf("cpuRR Added pID: %d back to the ReadyQ \n", currentProcess->pID);
                        }
                        currentProcess = NULL;
                    }
                }
            }

        // Update everything in the wait queue.
        circularlistnode *pointer = sharedResource->waitQ->next;
        while(pointer != sharedResource->waitQ)
        {
            if (pointer->current->timeInterrupt <= (sharedResource->time - pointer->current->timeEnteredWaitQ)) {
                removeNode(pointer);
                pointer->current->timeInterrupt = 0;
                synchronizedSchedule(sharedResource, pointer->current);
                printf("Added pID: %d back to the readyQ \n", pointer->current->pID);
            }
            pointer = pointer->next;
        }
        oldTime = sharedResource->time;
    }
    
    pthread_exit(NULL);
}

void *cpuClock(void *arg){
    sharedRes *sharedResource = (sharedRes *) arg;
    //sharedResource->numOfRandGenProcs = 5;
    while (EXECUTION_CONDITION) {
        pthread_mutex_lock(&lock);
        sharedResource->time++;
        pthread_mutex_unlock(&lock);
        if (!LOAD_PROCESSES_FROM_FILE) {
            // On every tick, randomly choose whether or not to create a process.
            process *proc = generateRandomProcess(0.3, 1, 10, sharedResource);//->time);
            if (proc != NULL && sharedResource->numOfRandGenProcs > 0) {
                printf("Scheduling process pID: %d entryTime: %d runTime: %d pages: %d \n", proc->pID, proc->entryTime, proc->runTime, proc->requiredMemoryPages);
                synchronizedSchedule(sharedResource, proc);
                sharedResource->numOfRandGenProcs--;
            }
        }
        usleep(500000);
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
            newProc->probSystemCall = .1; //give every process the same chance of generating a system call for now but should be random
            newProc->requiredMemoryPages = generateRandomNumberOfMemoryPages();
            newProc->hasBeenAllocatedMemory = 0;
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
    
    circularlistnode *waitQ;
    waitQ = (circularlistnode *)malloc(sizeof(circularlistnode));
    waitQ -> next = waitQ;
    waitQ -> prev = waitQ;
    
    priorityscheduler *scheduler = (priorityscheduler *)malloc(sizeof(priorityscheduler));
    pr_init_scheduler(scheduler);
    
    page *firstPage = (page *)malloc(sizeof(page));
    firstPage->startAddress = 0;
    firstPage->size = NUM_OF_MEM;
    page freeMemTable[((int)log2(NUM_OF_MEM)) + 1];
    for (int i = 0; i < ((sizeof(freeMemTable) / sizeof(page)) - 1); i++) {
        //freeMemTable[((int)log2(NUM_OF_MEM)) - 1] = NULL;
    }
    freeMemTable[((int)log2(NUM_OF_MEM)) - 1] = *firstPage;
    
    sharedRes *sharedResource = (sharedRes *) malloc(sizeof(sharedRes));
    sharedResource->scheduler = scheduler;
    sharedResource->time = 0;
    sharedResource->startDataSize = 0;
    sharedResource->doneQ = doneQ;
    sharedResource->doneQSize = 0;
    sharedResource->startData = start;
    sharedResource->nextPid = 0;
    sharedResource->waitQ = waitQ;
    sharedResource->numOfRandGenProcs = MAX_RAND_GEN_PROCS;
    sharedResource->freeMemoryArray = freeMemTable;
    
    //NOTE Ready to start generating the number of pages a process needs to execute and figure out hw to give it to them.
    
    pthread_mutex_init(&lock, NULL);

    // Seed the random function.
    srand((unsigned int)time(NULL));

    if (LOAD_PROCESSES_FROM_FILE)
        loadProcessesFromFile("/Users/Sean/Documents/College/Spring 2016/Advanced OS/OSSimulator/OSSimulator/dataFile.txt", sharedResource);
        //loadProcessesFromFile("/Users/cvrahimis/Documents/CSAdvOS/CProjects/OSSimulator/OSSimulator/dataFile.txt", sharedResource);
    
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

    rc = pthread_create(&cpuThread, NULL, cpu, (void *)sharedResource);
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
