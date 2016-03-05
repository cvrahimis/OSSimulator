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

typedef struct fifoscheduler {
    circularlistnode *readyQueueStart;
    int readyQueueSize;
} fifoscheduler;

void init_scheduler(fifoscheduler *scheduler);
void schedule(fifoscheduler *scheduler, process *proc, int time);
process *nextProcess();

#endif /* fifoscheduler_h */
