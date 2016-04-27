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
#include "PageLinkedList.h"
#include "CircularLinkedList.h"

#include "fifoscheduler.h"
#include "priorityscheduler.h"
#include "o1scheduler.h"

//#define MAX_RAND_GEN_PROCS 15
//#define IS_ROUND_ROBIN 0

//#define EXECUTION_CONDITION (sharedResource->time < 300)
#define EXECUTION_CONDITION (sharedResource->doneQSize != sharedResource->properties->maxRandGenProcs)

//#define NUM_OF_MEM 256
//#define PROBABILITY_INTERACTIVE 0.5
#define TIME_TO_SLEEP 100000
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

pthread_mutex_t lock;

enum scheduler {
    FIFO = 0,
    ROUND_ROBIN = 1,
    PRIORITY_SCHEDULER = 2
};

typedef struct systemProperties {
    unsigned int timeToSleep;
    int numMemoryBlocks;
    float probabilityInteractive;
    int maxRandGenProcs;
    int scheduler;
    int maxPriority;
    float interactiveThreshold;
} systemProperties;

typedef struct resources{
    int nextPid;
    int time;
    priorityscheduler *scheduler;
    circularlistnode *doneQ;
    int doneQSize;
    circularlistnode *waitQ;
    int numOfRandGenProcs;
    pagePointer *freeMemoryArray;
    systemProperties *properties;
}sharedRes;


/*debug method*/
void printMemoryTable(sharedRes *sharedResource){
    
    for (int i = 0; i < ((int)ceil(log2(sharedResource->properties->numMemoryBlocks))) + 1; i++) {
        page *page = sharedResource->freeMemoryArray[i];
        printPages(page, i);
    }
    printf("\n=====================================\n");
}

void synchronizedInsertPage(sharedRes *sharedResource, page *pages, int size){
    pthread_mutex_lock(&lock);
    insertPageSorted(sharedResource->freeMemoryArray[(int)ceil(log2(size))], pages);
    pthread_mutex_unlock(&lock);
}

void deallocate(sharedRes *sharedResource, process *proc){
    printf("\n");
    printMemoryTable(sharedResource);
    page *procPages = proc->memoryPages;
    int shifts = (int)ceil(log2(procPages->size));
    int comp = 1 << shifts;
    int buddyStartAddress = comp ^ procPages->startAddress;
    page *buddy = getBuddyFree(sharedResource->freeMemoryArray[shifts], buddyStartAddress);
    if (buddy != NULL) {
        buddy->startAddress = MIN(buddy->startAddress, procPages->startAddress);
        buddy->size = buddy->size + procPages->size;
        free(proc->memoryPages);
        proc->memoryPages = buddy;
        deallocate(sharedResource, proc);
    }
    else
    {
        synchronizedInsertPage(sharedResource, proc->memoryPages, proc->memoryPages->size);
        printf("Deallocating Mem for pID: %d and memStart: %d memSize: %d \n", proc->pID, proc->memoryPages->startAddress, proc->memoryPages->size);
        printMemoryTable(sharedResource);
    }
}

int allocate(sharedRes *sharedResource, process *proc){
    int originalIndex = ceil(log2(proc->requiredMemoryPages));
    int index = originalIndex;
    if (!proc->printedNotEnoughMem)
        printMemoryTable(sharedResource);
    
    page *contigiousPages = sharedResource->freeMemoryArray[index];
    while (index < (ceil(log2(sharedResource->properties->numMemoryBlocks)) + 1) && contigiousPages->next == contigiousPages && contigiousPages->prev == contigiousPages) {
        index++;
        contigiousPages = sharedResource->freeMemoryArray[index];
    }
    if (index == (ceil(log2(sharedResource->properties->numMemoryBlocks)) + 1)) {
        proc->hasBeenAllocatedMemory = 0;
        return 0;
    }

    while(originalIndex < index)
    {
        page *currentPage = removeFrontPage(contigiousPages);
        int buddySize = currentPage->size / 2;
        page *buddy1 = (page *)malloc(sizeof(page));
        buddy1->startAddress = currentPage->startAddress;
        buddy1->size = buddySize;
        page *buddy2 = (page *)malloc(sizeof(page));
        buddy2->startAddress = buddy1->startAddress + buddy1->size;
        buddy2->size = buddySize;
        synchronizedInsertPage(sharedResource, buddy1, buddy1->size);
        synchronizedInsertPage(sharedResource, buddy2, buddy2->size);
        free(currentPage);
        //printMemoryTable(sharedResource); DEBUG
        index--;
        contigiousPages = sharedResource->freeMemoryArray[index];
    }
    proc->memoryPages = removeFrontPage(sharedResource->freeMemoryArray[index]);
    proc->hasBeenAllocatedMemory = 1;
    return 1;
}

void synchronizedSchedule(sharedRes *sharedResource, process *proc){
    if (!proc->hasBeenAllocatedMemory)
    {
        if(allocate(sharedResource, proc))
        {
            pthread_mutex_lock(&lock);
            pr_schedule(sharedResource->scheduler, proc, sharedResource->time);
            pthread_mutex_unlock(&lock);
        }
        else
        {
            pthread_mutex_lock(&lock);
            cll_enqueue(sharedResource->waitQ, proc);
            pthread_mutex_unlock(&lock);
            if (!proc->printedNotEnoughMem) {
                printf("Not enough memory for pID: %d with requiredPages: %d, added to the waitQ \n", proc->pID, proc->requiredMemoryPages);
                proc->printedNotEnoughMem = 1;
            }
        }
    }
    else
    {
        pthread_mutex_lock(&lock);
        pr_schedule(sharedResource->scheduler, proc, sharedResource->time);
        pthread_mutex_unlock(&lock);
    }
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
        newProc->entryTime = sharedResource->time;
        int runTime = rand() % (maxRunTime - minRunTime) + minRunTime;
        newProc->runTime = runTime;
        newProc->runTimeRemaining = runTime;
        // Randomly choose if we want an interactive process.
        newProc->probSystemCall = generateProbabilityOfSystemCall(sharedResource->properties->probabilityInteractive);
        newProc->priority = rand() % sharedResource->properties->maxPriority;
        newProc->timeEnteredReadyQ = -1;
        if (sharedResource->properties->scheduler == ROUND_ROBIN)
            newProc->timeSlice = 4;
        else
            newProc->timeSlice = (sharedResource->properties->maxPriority - newProc->priority) * 2;
        newProc->requiredMemoryPages = generateRandomNumberOfMemoryPages();
        newProc->hasBeenAllocatedMemory = 0;
        newProc->printedNotEnoughMem = 0;
        newProc->isInteractive = false;
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
                    currentProcess->timeEnteredCPU = sharedResource->time;
                    pthread_mutex_lock(&lock);
                    pthread_mutex_unlock(&lock);
                    printf("Running pID: %d With Run Time Remaining: %d Time: %d Priority: %d \n", currentProcess->pID, currentProcess->runTimeRemaining, sharedResource->time, currentProcess->priority);
                }
            }
        }
        // Otherwise, if a process is running...
        else {
            // If the process's run time is up, remove it from the CPU.
            if (currentProcess->runTimeRemaining <= (sharedResource->time - currentProcess->timeEnteredCPU)) {
                currentProcess->timeDone = sharedResource->time;
                currentProcess->runTimeRemaining = 0;
                deallocate(sharedResource, currentProcess);
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
                        // Determine whether or not the process
                        // should be classified as interactive.
                        currentProcess->timeSystemCall++;
                        // Don't bother calculating anything unless
                        // it has already run a few times, because
                        // otherwise it doesn't mean much.
                        if (currentProcess->timeAllotted > 5) {
                            double systemCallRatio = (double)currentProcess->timeSystemCall / currentProcess->timeAllotted;
                            if (!currentProcess->isInteractive && systemCallRatio > sharedResource->properties->interactiveThreshold) {
                                currentProcess->isInteractive = true;
                                printf("pId %d flagged as interactive\n", currentProcess->pID);
                            } else if (currentProcess->isInteractive && systemCallRatio <= sharedResource->properties->interactiveThreshold) {
                                currentProcess->isInteractive = false;
                                printf("pId %d interactive flag removed\n", currentProcess->pID);
                            }
                        }
                        int updatedRunTime = (currentProcess->runTimeRemaining - (sharedResource->time - currentProcess->timeEnteredCPU));
                        currentProcess->runTimeRemaining = updatedRunTime < 0 ? 0 : updatedRunTime; //making sure the runTime cannot be negative
                        currentProcess->timeInterrupt = ((int)rand() % 6) + 3;//setting the time it takes for the interrupt to complete
                        currentProcess->timeEnteredWaitQ = sharedResource->time;
                        cll_enqueue(sharedResource->waitQ, currentProcess);
                        printf("System call made by pID %d, added to the waitQ \n", currentProcess->pID);
                        currentProcess = NULL;
                    }
                }
                
                // Round robin / Time slice
                // Time slice of -1 means no limit.
                if (currentProcess != NULL && currentProcess->timeSlice != -1  && currentProcess->timeSlice <= (sharedResource->time - currentProcess->timeEnteredCPU)){
                        currentProcess->runTimeRemaining -= currentProcess->timeSlice;
                        if (currentProcess->runTimeRemaining <= 0){
                            currentProcess->runTimeRemaining = 0;
                            currentProcess->timeDone = sharedResource->time;
                            deallocate(sharedResource, currentProcess);
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
            if ((pointer->current->timeInterrupt <= (sharedResource->time - pointer->current->timeEnteredWaitQ) && pointer->current->hasBeenAllocatedMemory) || (!pointer->current->hasBeenAllocatedMemory && allocate(sharedResource, pointer->current))) {
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
        // On every tick, randomly choose whether or not to create a process.
        process *proc = generateRandomProcess(0.3, 3, 100, sharedResource);//->time);
        if (proc != NULL && sharedResource->numOfRandGenProcs > 0) {
            printf("\nScheduling process pID: %d entryTime: %d runTime: %d pages: %d \n", proc->pID, proc->entryTime, proc->runTime, proc->requiredMemoryPages);
            synchronizedSchedule(sharedResource, proc);
            sharedResource->numOfRandGenProcs--;
        }
        usleep(TIME_TO_SLEEP);
    }
    pthread_exit(NULL);
}

systemProperties *loadPropertiesFromFile(const char *fileName) {
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == NULL)
        return NULL;
    
    char c[255];
    systemProperties *properties = (systemProperties*)malloc(sizeof(systemProperties));
    fscanf(fp, "%d", &properties->timeToSleep); fscanf(fp, "%s", c);
    fscanf(fp, "%d", &properties->numMemoryBlocks); fscanf(fp, "%s", c);
    fscanf(fp, "%f", &properties->probabilityInteractive); fscanf(fp, "%s", c);
    fscanf(fp, "%d", &properties->maxRandGenProcs); fscanf(fp, "%s", c);
    fscanf(fp, "%d", &properties->scheduler); fscanf(fp, "%s", c);
    fscanf(fp, "%d", &properties->maxPriority); fscanf(fp, "%s", c);
    fscanf(fp, "%f", &properties->interactiveThreshold);
    fclose(fp);
    
    return properties;
}

int main(int argc, const char * argv[]) {
    int rc;
    pthread_t clockThread;
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
    
    sharedRes *sharedResource = (sharedRes *) malloc(sizeof(sharedRes));
    sharedResource->properties = loadPropertiesFromFile(argv[1]);
    
    priorityscheduler *scheduler = (priorityscheduler *)malloc(sizeof(priorityscheduler));
    pr_init_scheduler(scheduler, sharedResource->properties->maxPriority);
    
    //int size = (int)ceil(log2(NUM_OF_MEM));
    pagePointer freeMemTable[((int)ceil(log2(sharedResource->properties->numMemoryBlocks))) + 1];
    
    for (int i = 0; i < ((int)ceil(log2(sharedResource->properties->numMemoryBlocks))) + 1; i++) {
        page *start = (page *) malloc(sizeof(page));
        start->next = start;
        start->prev = start;
        freeMemTable[i] = start;
    }
    
    page *firstPage = (page *)malloc(sizeof(page));
    firstPage->startAddress = 0;
    firstPage->size = sharedResource->properties->numMemoryBlocks;

    freeMemTable[((int)log2(sharedResource->properties->numMemoryBlocks))]->next = firstPage;
    freeMemTable[((int)log2(sharedResource->properties->numMemoryBlocks))]->prev = firstPage;
    firstPage->next = freeMemTable[((int)log2(sharedResource->properties->numMemoryBlocks))];
    firstPage->prev = freeMemTable[((int)log2(sharedResource->properties->numMemoryBlocks))];
    
    sharedResource->scheduler = scheduler;
    sharedResource->time = 0;
    sharedResource->doneQ = doneQ;
    sharedResource->doneQSize = 0;
    sharedResource->nextPid = 0;
    sharedResource->waitQ = waitQ;
    if (sharedResource->properties == NULL)
    {
        printf("Unable to read system properties. Cannot run simulation.\n");
        return 1;
    }
    sharedResource->numOfRandGenProcs = sharedResource->properties->maxRandGenProcs;
    sharedResource->freeMemoryArray = freeMemTable;
    
    //NOTE Ready to start generating the number of pages a process needs to execute and figure out hw to give it to them.
    
    pthread_mutex_init(&lock, NULL);

    // Seed the random function.
    srand((unsigned int)time(NULL));

    rc = pthread_create(&clockThread, NULL, cpuClock, (void *)sharedResource);
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
    
    pthread_mutex_destroy(&lock);
    return 0;
}
