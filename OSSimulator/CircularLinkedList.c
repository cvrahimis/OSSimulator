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
//int length = 0;

void print(circularlistnode *start, circularlistnode *pointer)
{
    if(pointer==start)
    {
        return;
    }
    printf("pid: %d entrytime: %d runtime: %d \n",(pointer->current)->pID, (pointer->current)->entryTime, (pointer->current)->runTime);
    print(start, pointer->next);
}

void enqueue(circularlistnode *start, process *data)
{
    insertBack(start, data);
}

process *dequeue(circularlistnode *start)
{
    if (start->next == start)
        return NULL;

    circularlistnode *pointer = start->next;
    circularlistnode *next = pointer->next;
    
    start->next = next;
    next->prev = start;
    
    process *proc = pointer->current;
    free(pointer);
    //length--;
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
    //length++;
}

void insertFront(circularlistnode *start, process *data)
{
    circularlistnode *addFront = start->next;
    
    addFront->prev = (circularlistnode *)malloc(sizeof(circularlistnode));
    (addFront->prev)->current = data;
    (addFront->prev)->next = addFront;
    (addFront->prev)->prev = start;
    start->next = addFront->prev;
    //length++;
}
/*
void sort(circularlistnode *start)
{
    int i;
    for (i = 0; i < size(); i++)
    {
        circularlistnode *pointer = start->next;
        circularlistnode *next = pointer->next;
        while(pointer->next != start)
        {
            if ((next->current)->entryTime < (pointer->current)->entryTime)
            {
                circularlistnode *hold = (circularlistnode *)malloc(sizeof(circularlistnode));
                hold->next = pointer->next;
                hold->prev = pointer->prev;
                
                pointer->prev = next->prev;
                pointer->next = next->next;
                
                next->prev = hold->prev;
                next->next = hold->next;
                
                free(hold);
            }
            next = pointer->next;
        }
    }
}

void insertSorted(circularlistnode *start, process *data)
{
    if (length == 0) {
        insertFront(start, data);
        return;
    }
    circularlistnode *pointer = start->next;
    circularlistnode *trail = start;
    
    while(1)
    {
        if (data->runTime <= (pointer->current)->runTime)
        {
            circularlistnode *insert = (circularlistnode *)malloc(sizeof(circularlistnode));
            insert->current = data;
            insert->next = pointer;
            insert->prev = trail;
            pointer->prev = insert;
            trail->next = insert;
            length++;
            return;
        }
        if (data->runTime > (pointer->current)->runTime && pointer->next == start)
        {
            circularlistnode *insert = (circularlistnode *)malloc(sizeof(circularlistnode));
            insert->current = data;
            insert->prev = pointer;
            insert->next = start;
            pointer->next = insert;
            start->prev = insert;
            length++;
            return;
        }
        trail = pointer;
        pointer = pointer->next;
        //next = pointer->next;
    }
}*/
/*
int find(circularlistnode *start, int key)
{
    circularlistnode *pointer = start->next;
    while(pointer!=start)
    {
        if((pointer->current)->pID == key)
        {
            return 1;
        }
        pointer = pointer -> next;
    }
    return 0;
}

circularlistnode * locate(circularlistnode *start, int key)
{
    //sort(start);
    circularlistnode *pointer = start->next;
    while(pointer!=start)
    {
        if((pointer->current)->pID == key)
        {
            return pointer;
        }
        pointer = pointer -> next;
    }
    return NULL;
}
*/
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
    //length--;
    
    return;
}
/*
void removeFront(circularlistnode *start){
    if (size() > 0) {
        circularlistnode *pointer = start->next;
        circularlistnode *next = pointer->next;
        
        start->next = next;
        next->prev = start;
        
        free(pointer);
        length--;
    }
    else
    {
        printf("\nList is Empty\n\n");
    }
}

void removeBack(circularlistnode *start){
    if (size() > 0) {
        circularlistnode *pointer = start->prev;
        circularlistnode *next = pointer->prev;
        
        start->prev = next;
        next->next = start;
        
        free(pointer);
        length--;
    }
    else
    {
        printf("\nList is Empty\n\n");
    }
}

int size(){
    return length;
}
*/
void printData(circularlistnode *doneStart, circularlistnode *pointer) {
    printf("\n");
    if(pointer==doneStart)
    {
        return;
    }
    printf("pid: %d entrytime: %d runtime: %d timeEnteredReadyQ: %d timeEnteredCPU: %d timeComplete: %d \n",(pointer->current)->pID, (pointer->current)->entryTime, (pointer->current)->runTime, (pointer->current)->timeEnteredReadyQ, (pointer->current)->timeEnteredCPU, (pointer->current)->timeDone);
    printData(doneStart, pointer->next);
}
