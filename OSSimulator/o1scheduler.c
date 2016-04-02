//
//  o1scheduler.c
//  OSSimulator
//
//  Created by Sean Campbell on 3/30/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#include "o1scheduler.h"
#include <stdlib.h>

void o1_init_scheduler(o1scheduler *scheduler)
{
    scheduler->readyQueueSize = 0;
    
    scheduler->activeQueueBitMap  = 0;
    scheduler->expiredQueueBitMap = 0;
    
    scheduler->activeProcessQueue  = (circularlistnode**)malloc(sizeof(circularlistnode*)*MAX_PRIORITY);
    scheduler->expiredProcessQueue = (circularlistnode**)malloc(sizeof(circularlistnode*)*MAX_PRIORITY);
    
    for (int i = 0; i < MAX_PRIORITY; i++)
    {
        scheduler->activeProcessQueue[i] = (circularlistnode*)malloc(sizeof(circularlistnode));
        scheduler->activeProcessQueue[i]->next = scheduler->activeProcessQueue[i];
        scheduler->activeProcessQueue[i]->prev = scheduler->activeProcessQueue[i];
        scheduler->expiredProcessQueue[i] = (circularlistnode*)malloc(sizeof(circularlistnode));
        scheduler->expiredProcessQueue[i]->next = scheduler->expiredProcessQueue[i];
        scheduler->expiredProcessQueue[i]->prev = scheduler->expiredProcessQueue[i];
    }

}

void o1_schedule(o1scheduler *scheduler, process *proc, int time)
{
    scheduler->readyQueueSize++;
    proc->timeEnteredReadyQ = time;
    int dynamicPriority = processDynamicPriority(proc);
    printf("dp: %d\n", dynamicPriority);
    cll_enqueue(scheduler->activeProcessQueue[dynamicPriority], proc);
    scheduler->activeQueueBitMap |= (1 << dynamicPriority);
}


process *o1_nextProcess(o1scheduler *scheduler)
{
    process *proc = NULL;
    
    // Run through all the active queues and get the first
    // process from the lowest priority queue.
    for (int i = 0; i < MAX_PRIORITY; i++)
    {
        if (scheduler->activeProcessQueue[i]->next->current)
        {
            proc = cll_dequeue(scheduler->activeProcessQueue[i]);
            scheduler->readyQueueSize--;
            // If the queue is now empty, flip it to 0 in the bitmap.
            if (!scheduler->activeProcessQueue[i]->next->current)
                scheduler->activeQueueBitMap ^= (1 << i);
            break;
        }
    }
    
    // If there are no more active processes,
    // swap the active and expired queues.
    if (scheduler->activeQueueBitMap == 0)
    {
        circularlistnode **temp = scheduler->activeProcessQueue;
        scheduler->activeProcessQueue = scheduler->expiredProcessQueue;
        scheduler->expiredProcessQueue = temp;
    }
    
    return proc;
}

