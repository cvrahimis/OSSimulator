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

pthread_mutex_t lock;

typedef struct resources{
    node *readyQstart;
    int time;
    int readyQsize;
    node *startData;
    int startDataSize;
    node *doneQ;
    int cpuState;
}sharedRes;

void *cpu(void *arg){
    sharedRes *sharedResource = (sharedRes *) arg;
    process *currentProcess = NULL;
    while (sharedResource->readyQsize > 0 || sharedResource->startDataSize > 0 || sharedResource->cpuState)
    {
        if (!sharedResource->cpuState && sharedResource->readyQsize > 0) {
            sharedResource->cpuState = 1;
            currentProcess = locate(sharedResource->readyQstart, (((sharedResource->readyQstart)->next)->current)->pID)->current;
            int timeEntered = sharedResource->time;
            currentProcess->timeEnteredCUP = timeEntered;
            delete(sharedResource->readyQstart, currentProcess->pID);
            sharedResource->readyQsize--;
            printf("Running pID: %d \n", currentProcess->pID);
        }
        if (currentProcess != NULL && currentProcess->runTime <= (sharedResource->time - currentProcess->timeEnteredCUP)) {
            currentProcess->timeDone = sharedResource->time;
            insertBack(sharedResource->doneQ, currentProcess);
            printf("Added pID: %d to the doneQ \n", currentProcess->pID);
            //delete(sharedResource->readyQstart, currentProcess->pID);
            sharedResource->cpuState = 0;
            currentProcess = NULL;
        }
    }
    
    pthread_exit(NULL);
}

void *cpuClock(void *arg){
    sharedRes *sharedResource = (sharedRes *) arg;
    while (sharedResource->readyQsize > 0 || sharedResource->startDataSize > 0 || sharedResource->cpuState) {
        pthread_mutex_lock(&lock);
        sharedResource->time++;
        pthread_mutex_unlock(&lock);
        //printf("Tick: %d \n", t);
        sleep(1);
    }
    
    pthread_exit(NULL);
}

void *addToReadyQ(void *arg){
    sharedRes *sharedResource = (sharedRes *) arg;
    node *initData = sharedResource->startData;
    while(sharedResource->readyQsize > 0 || sharedResource->startDataSize > 0) {
        while(initData->next != sharedResource->startData || sharedResource->startDataSize > 0){
            if (initData != sharedResource->startData && (initData->current)->entryTime <= sharedResource->time) {
                (initData->current)->timeEnteredReadyQ = sharedResource->time;
                insertBack(sharedResource->readyQstart, initData->current);
                printf("Added pID: %d to the ReadyQ \n", (initData->current)->pID);
                sharedResource->readyQsize++;
                //print(sharedResource->readyQstart, (sharedResource->readyQstart)->next);
                //printf("\n=================================\n");
                delete(sharedResource->startData, initData->current->pID);
                sharedResource->startDataSize--;
            }
            initData = initData->next;
        }
        initData = initData->next;
    }
    
    //print(sharedResource->readyQstart, (sharedResource->readyQstart)->next);
    pthread_exit(NULL);
}

/**
 * @param {double} probability - probability of creating a new process, double between 0 and 1
 * @param {sharedRes *} sharedResource - pointer to the shared resource for the OS
 * @return {process *} - a new random process or NULL, based on probability
 */
process *generateRandomProcess(double probability, int minRunTime, int maxRunTime, sharedRes *sharedResource)
{
    srand((unsigned int)time(NULL));
    double randomValue = (double)rand() / RAND_MAX;
    if (randomValue >= probability) {
        process *newProc = (process *)malloc(sizeof(process));
        //TODO: need to generate pID
        newProc->pID = 0;
        newProc->entryTime = sharedResource->time;
        int runTime = rand() % (maxRunTime - minRunTime) + minRunTime;
        newProc->runTime = runTime;
    }
    return NULL;
}

void loadProcessesFromFile(char *fileName, node *start, sharedRes *sharedResource) {
    FILE *fp;
    char buff[255];
    fp=fopen(fileName, "r");
    int i = 0;
    process *newProc = NULL;
    while (fscanf(fp, "%s", buff) != EOF) {
        /*DEBUG*///printf("%d \n", atoi(buff));
        if (i % 3 == 0)
        {
            newProc = (process *) malloc(sizeof(process));
            /*DEBUG*/   //printf("%d ", atoi(buff));
            newProc->pID = atoi(buff);
            /*DEBUG*/   //printf(" pID: %d ", newProc->pID);
        }
        else if (i % 3 == 1)
        {
            /*DEBUG*/   //printf("%d ", atoi(buff));
            newProc->entryTime = atoi(buff);
            /*DEBUG*/   //printf("entryTime: %d ", newProc->entryTime);
        }
        else
        {
            /*DEBUG*/   //printf("%d ", atoi(buff));
            newProc->runTime = atoi(buff);
            insertBack(start, newProc);
            sharedResource->startDataSize++;
            /*DEBUG*/   //printf("runTime: %d \n", newProc->runTime);
        }
        i++;
    }
}

void printData(node *doneStart, node *pointer) {
    printf("\n");
    if(pointer==doneStart)
    {
        return;
    }
    printf("pid: %d entrytime: %d runtime: %d timeEnteredReadyQ: %d timeEnteredCPU: %d timeComplete: %d \n",(pointer->current)->pID, (pointer->current)->entryTime, (pointer->current)->runTime, (pointer->current)->timeEnteredReadyQ, (pointer->current)->timeEnteredCUP, (pointer->current)->timeDone);
    printData(doneStart, pointer->next);
}

int main(int argc, const char * argv[]) {
    int rc;
    pthread_t clockThread;
    pthread_t readyQThread;
    pthread_t cpuThread;
    
    node *start;
    start = (node *)malloc(sizeof(node));
    start -> next = start;
    start -> prev = start;
    
    node *readyQ;
    readyQ = (node *)malloc(sizeof(node));
    readyQ -> next = readyQ;
    readyQ -> prev = readyQ;
    
    node *doneQ;
    doneQ = (node *)malloc(sizeof(node));
    doneQ -> next = doneQ;
    doneQ -> prev = doneQ;
    
    sharedRes *sharedResource = (sharedRes *) malloc(sizeof(sharedRes));
    sharedResource->readyQstart = readyQ;
    sharedResource->time = 0;
    sharedResource->startDataSize = 0;
    sharedResource->doneQ = doneQ;
    sharedResource->cpuState = 0;
    
    loadProcessesFromFile("/Users/Sean/Documents/College/Spring 2016/Advanced OS/OSSimulator/OSSimulator/dataFile.txt", start, sharedResource);
    //loadProcessesFromFile("/Users/cvrahimis/Documents/CSAdvOS/CProjects/OSSimulator/OSSimulator/dataFile.txt", start, sharedResource);
    
    print(start, start->next);
    printf("\n=================================\n");
    sharedResource->startData = start;
    
    rc = pthread_create(&clockThread, NULL, cpuClock, (void *)sharedResource);
    if(rc)
    {
        printf("ERROR. Return code from thread %d\n", rc);
        exit(-1);
    }
    rc = pthread_create(&readyQThread, NULL, addToReadyQ, (void *)sharedResource);
    if(rc)
    {
        printf("ERROR. Return code from thread %d\n", rc);
        exit(-1);
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
    rc = pthread_join(readyQThread, NULL);
    if(rc)
    {
        printf("Error, return code from thread_join() is %d\n", rc);
        exit(-1);
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
