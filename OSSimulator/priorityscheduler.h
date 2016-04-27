//
//  priorityscheduler.h
//  OSSimulator
//
//  Created by Sean Campbell on 3/30/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#ifndef prscheduler_h
#define prscheduler_h

//#define MAX_PRIORITY 5

#include "CircularLinkedList.h"

typedef struct priorityscheduler
{
    circularlistnode **activeProcessQueue;
    int readyQueueSize;
    int maxPriority;
} priorityscheduler;

void pr_init_scheduler(priorityscheduler *scheduler, int maxPriority);
void pr_schedule(priorityscheduler *scheduler, process *data, int time);
process *pr_nextProcess(priorityscheduler *scheduler);


#endif /* prscheduler_h */
