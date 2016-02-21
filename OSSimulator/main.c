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
}sharedRes;

void *cpuClock(void *arg){
    sharedRes *sharedResource = (sharedRes *) arg;
    while (sharedResource->readyQsize != 5 || sharedResource->startDataSize > 0) {
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
    while(sharedResource->readyQsize != 5 || sharedResource->startDataSize > 0) {
        while(initData->next != sharedResource->startData || sharedResource->startDataSize > 0){
            if (initData != sharedResource->startData && (initData->current)->entryTime <= sharedResource->time) {
                insertBack(sharedResource->readyQstart, initData->current);
                sharedResource->readyQsize++;
                print(sharedResource->readyQstart, (sharedResource->readyQstart)->next);
                printf("\n=================================\n");
                delete(sharedResource->startData, initData->current->pID);
                sharedResource->startDataSize--;
            }
            initData = initData->next;
        }
        initData = initData->next;
    }
    
    print(sharedResource->readyQstart, (sharedResource->readyQstart)->next);
    pthread_exit(NULL);
}

int main(int argc, const char * argv[]) {
    int rc;
    pthread_t clockThread;
    pthread_t readyQThread;
    char buff[255];
    
    node *start;
    start = (node *)malloc(sizeof(node));
    start -> next = start;
    start -> prev = start;
    
    node *readyQ;
    readyQ = (node *)malloc(sizeof(node));
    readyQ -> next = readyQ;
    readyQ -> prev = readyQ;
    
    sharedRes *sharedResource = (sharedRes *) malloc(sizeof(sharedRes));
    sharedResource->readyQstart = readyQ;
    sharedResource->time = 0;
    sharedResource->startDataSize = 0;
    
    FILE *fp;
    //Sean you will probably need to change the path based on where you put the project
    fp=fopen("/Users/cvrahimis/Documents/CSAdvOS/CProjects/OSSimulator/OSSimulator/dataFile.txt", "r");
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
    printf("main completed join with thread clock thread \n");
    
    int fclose(FILE *fp);
    
    pthread_mutex_destroy(&lock);
    return 0;
}
