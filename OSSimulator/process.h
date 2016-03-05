//
//  process.h
//  OSSimulator
//
//  Created by Sean Campbell on 3/4/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#ifndef process_h
#define process_h

typedef struct Process
{
    int pID;
    int entryTime;
    int runTime;
    int timeEnteredReadyQ;
    int timeEnteredCPU;
    int timeDone;
    //int memory;
} process;


#endif /* process_h */
