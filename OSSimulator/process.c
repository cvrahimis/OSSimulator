//
//  process.c
//  OSSimulator
//
//  Created by Sean Campbell on 3/30/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#include "process.h"

int processDynamicPriority(process *proc)
{
    if (!proc)
        return -1;
    int dynamicPriority = proc->priority;
    if (proc->probSystemCall > INTERACTIVE_THRESHOLD)
        dynamicPriority = dynamicPriority > 0 ? dynamicPriority - 1 : dynamicPriority;
    return dynamicPriority;
}
