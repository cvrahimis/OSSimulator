//
//  o1scheduler.h
//  OSSimulator
//
//  Created by Sean Campbell on 3/30/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#ifndef o1scheduler_h
#define o1scheduler_h

#include "CircularLinkedList.h"

typedef struct o1scheduler
{
    circularlistnode **activeProcessQueue;
    circularlistnode **expiredProcessQueue;
    int activeQueueBitMap;
    int expiredQueueBitMap;
    int readyQueueSize;
    int maxPriority;
} o1scheduler;

void o1_init_scheduler(o1scheduler *scheduler, int maxPriority);
void o1_schedule(o1scheduler *scheduler, process *data, int time);
process *o1_nextProcess(o1scheduler *scheduler);


#endif /* o1scheduler_h */
