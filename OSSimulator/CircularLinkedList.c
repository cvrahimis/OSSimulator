//
//  CircularLinkedList.c
//  CircularLinkedList
//
//  Created by Costas Vrahimis on 1/31/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include "CircularLinkedList.h"

void print(circularlistnode *start, circularlistnode *pointer)
{
    if(pointer==start)
    {
        return;
    }
    printf("pid: %d entrytime: %d runtime: %d \n",(pointer->current)->pID, (pointer->current)->entryTime, (pointer->current)->runTime);
    print(start, pointer->next);
}

void cll_enqueue(circularlistnode *start, process *data)
{
    insertBack(start, data);
}

process *cll_dequeue(circularlistnode *start)
{
    if (start->next == start)
        return NULL;

    circularlistnode *pointer = start->next;
    circularlistnode *next = pointer->next;
    
    start->next = next;
    next->prev = start;
    
    process *proc = pointer->current;
    free(pointer);
    return proc;
}

void insertBack(circularlistnode *start, process *data)
{
    circularlistnode *addBack = start->prev;
    
    addBack->next = (circularlistnode *)malloc(sizeof(circularlistnode));
    addBack->next->current = data;
    addBack->next->prev = addBack;
    addBack->next->next = start;
    start->prev = addBack->next;
}

void insertFront(circularlistnode *start, process *data)
{
    circularlistnode *addFront = start->next;
    
    addFront->prev = (circularlistnode *)malloc(sizeof(circularlistnode));
    (addFront->prev)->current = data;
    (addFront->prev)->next = addFront;
    (addFront->prev)->prev = start;
    start->next = addFront->prev;
}

void removeNode(circularlistnode *node)
{
    circularlistnode *previous = node->prev;
    circularlistnode *next = node->next;
    
    previous->next = next;
    next->prev = previous;
}

void del(circularlistnode *start, int pID)
{
    if (start == start->next) {
        printf("List is empty \n\n");
        return;
    }
    circularlistnode *pointer = start->next;
    while(pointer->next!=start && (pointer->current)->pID != pID)
    {
        pointer = pointer -> next;
    }
    if(pointer->next==start && (pointer->current)->pID != pID)
    {
        printf("Element %d is not present in the list\n\n",pID);
        return;
    }
    
    (pointer->prev)->next = pointer->next;
    (pointer->next)->prev = pointer->prev;
    free(pointer);
    
    return;
}

void printData(circularlistnode *doneStart, circularlistnode *pointer) {
    printf("\n");
    if(pointer==doneStart)
    {
        return;
    }
    printf("pid: %d entrytime: %d runtime: %d timeEnteredReadyQ: %d timeComplete: %d totalTimeInSystem: %d\n",(pointer->current)->pID, (pointer->current)->entryTime, (pointer->current)->runTime, (pointer->current)->timeEnteredReadyQ, (pointer->current)->timeDone, pointer->current->timeDone - pointer->current->entryTime);
    printData(doneStart, pointer->next);
}
