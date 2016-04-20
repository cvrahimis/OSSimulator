//
//  process.h
//  OSSimulator
//
//  Created by Sean Campbell on 3/4/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#ifndef process_h
#define process_h

#include <stdbool.h>
#define INTERACTIVE_THRESHOLD 0.5

typedef struct Process
{
    int pID;
    int entryTime;
    int runTime;
    int runTimeRemaining;
    int timeEnteredReadyQ;
    int timeEnteredCPU;
    int timeDone;
    double probSystemCall;
    int timeInterrupt;
    int timeEnteredWaitQ;
    int requiredMemoryPages;
    int hasBeenAllocatedMemory;
    struct page *memoryPages;
    int priority;
    int timeSlice;
    int printedNotEnoughMem;
    // For OS to determine if
    // process is interactive.
    bool isInteractive;
    int timeAllotted;
    int timeSystemCall;
} process;

int processDynamicPriority(process *proc);

#endif /* process_h */
