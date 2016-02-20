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

int t = 0;
pthread_mutex_t lock;

void *cpuClock(void *arg){
    
    while (t < 10) {
        pthread_mutex_lock(&lock);
        t++;
        pthread_mutex_unlock(&lock);
        printf("Tick: %d \n", t);
        sleep(1);
    }
    
    pthread_exit(NULL);
}

void *addToReadyQ(void *arg){
    node *start,*temp;
    start = (node *)malloc(sizeof(node));
    temp = start;
    temp -> next = start;
    temp -> prev = start;
    
    pthread_exit(NULL);
}

int main(int argc, const char * argv[]) {
    int rc;
    pthread_t clockThread;
    char buff[255];
    
    node *start;//,*temp;
    start = (node *)malloc(sizeof(node));
    //temp = start;
    start -> next = start;
    start -> prev = start;
    
    FILE *fp;
    //Sean you will probably need to change the path based on where you put the project
    fp=fopen("/Users/cvrahimis/Documents/CSAdvOS/CProjects/OSSimulator/OSSimulator/dataFile.txt", "r");
    int i = 0;
    process *newProc = NULL;
    while (fscanf(fp, "%s", buff) != EOF) {
        //printf("%d \n", atoi(buff));
        
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
/*DEBUG*/   //printf("runTime: %d \n", newProc->runTime);
        }
        i++;
    }
    print(start, start->next);
    //fscanf(fp,"%s", buff);
    //printf("%s\n", buff);
    /*
    rc = pthread_create(&clockThread, NULL, cpuClock, NULL);
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
    printf("main completed join with thread clock thread \n");
    */
    int fclose(FILE *fp);
    
    pthread_mutex_destroy(&lock);
    return 0;
}
