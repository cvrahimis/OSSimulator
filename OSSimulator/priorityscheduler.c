//
//  priorityscheduler.c
//  OSSimulator
//
//  Created by Sean Campbell on 3/30/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#include "priorityscheduler.h"
#include <stdlib.h>

void pr_init_scheduler(priorityscheduler *scheduler, int maxPriority)
{
    scheduler->maxPriority = maxPriority;
    scheduler->readyQueueSize = 0;
    scheduler->activeProcessQueue  = (circularlistnode**)malloc(sizeof(circularlistnode*)*maxPriority);
    
    for (int i = 0; i < maxPriority; i++)
    {
        scheduler->activeProcessQueue[i] = (circularlistnode*)malloc(sizeof(circularlistnode));
        scheduler->activeProcessQueue[i]->next = scheduler->activeProcessQueue[i];
        scheduler->activeProcessQueue[i]->prev = scheduler->activeProcessQueue[i];
    }
    
}

void pr_schedule(priorityscheduler *scheduler, process *proc, int time)
{
    scheduler->readyQueueSize++;
    if (proc->timeEnteredReadyQ == -1)
        proc->timeEnteredReadyQ = time;
    proc->timeAllotted++;
    int dynamicPriority = processDynamicPriority(proc);
    //printf("dp: %d\n", dynamicPriority);
    cll_enqueue(scheduler->activeProcessQueue[dynamicPriority], proc);
}


process *pr_nextProcess(priorityscheduler *scheduler)
{
    process *proc = NULL;
    
    // Run through all the active queues and get the first
    // process from the lowest priority queue.
    for (int i = 0; i < scheduler->maxPriority; i++)
    {
        if (scheduler->activeProcessQueue[i]->next->current)
        {
            proc = cll_dequeue(scheduler->activeProcessQueue[i]);
            scheduler->readyQueueSize--;
            break;
        }
    }
    
    return proc;
}

