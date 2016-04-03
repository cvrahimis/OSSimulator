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
#include <pthread.h>
#include <unistd.h>

void fifo_init_scheduler(fifoscheduler *scheduler)
{
    scheduler->readyQueueSize = 0;
    circularlistnode *readyQ = (circularlistnode *)malloc(sizeof(circularlistnode));
    readyQ->next = readyQ;
    readyQ->prev = readyQ;
    scheduler->readyQueueStart = readyQ;
}

void fifo_schedule(fifoscheduler *scheduler, process *proc, int time)
{
    if (proc->timeEnteredReadyQ == -1)
        proc->timeEnteredReadyQ = time;
    scheduler->readyQueueSize++;
    cll_enqueue(scheduler->readyQueueStart, proc);
}

process *fifo_nextProcess(fifoscheduler *scheduler)
{
    scheduler->readyQueueSize--;
    return cll_dequeue(scheduler->readyQueueStart);
}