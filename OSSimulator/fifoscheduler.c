//
//  fifoscheduler.c
//  OSSimulator
//
//  Created by Sean Campbell on 3/4/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#include "CircularLinkedList.h"
#include "fifoscheduler.h"
#include <stdlib.h>

void init_scheduler(fifoscheduler *scheduler)
{
    scheduler->readyQueueSize = 0;
    circularlistnode *readyQ = (circularlistnode *)malloc(sizeof(circularlistnode));
    readyQ->next = readyQ;
    readyQ->prev = readyQ;
    scheduler->readyQueueStart = readyQ;
}

void schedule(fifoscheduler *scheduler, process *proc, int time)
{
    proc->timeEnteredReadyQ = time;
    scheduler->readyQueueSize++;
    enqueue(scheduler->readyQueueStart, proc);
}

process *nextProcess(fifoscheduler *scheduler)
{
    scheduler->readyQueueSize--;
    return dequeue(scheduler->readyQueueStart);
}