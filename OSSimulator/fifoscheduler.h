//
//  fifoscheduler.h
//  OSSimulator
//
//  Created by Sean Campbell on 3/4/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#ifndef fifoscheduler_h
#define fifoscheduler_h

#include "CircularLinkedList.h"

typedef struct fifoscheduler
{
    circularlistnode *readyQueueStart;
    int readyQueueSize;
} fifoscheduler;

void fifo_init_scheduler(fifoscheduler *scheduler);
void fifo_schedule(fifoscheduler *scheduler, process *proc, int time);
process *fifo_nextProcess();

#endif /* fifoscheduler_h */
