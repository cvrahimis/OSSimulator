//
//  CircularLinkedList.c
//  CircularLinkedList
//
//  Created by Costas Vrahimis on 1/31/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#include "CircularLinkedList.h"
int size = 0;

void print(node *start,node *pointer)
{
    if(pointer==start)
    {
        return;
    }
    printf("pid: %d entrytime: %d runtime: %d \n",(pointer->current)->pID, (pointer->current)->entryTime, (pointer->current)->runTime);
    print(start, pointer->next);
}

void insertBack(node *start, process *data)
{
    node *addBack = start->prev;
    
    addBack->next = (node *)malloc(sizeof(node));
    (addBack->next)->current = data;
    (addBack->next)->prev = addBack;
    (addBack->next)->next = start;
    start->prev = addBack->next;
    size++;
}

void insertFront(node *start, process *data)
{
    node *addFront = start->next;
    
    addFront->prev = (node *)malloc(sizeof(node));
    (addFront->prev)->current = data;
    (addFront->prev)->next = addFront;
    (addFront->prev)->prev = start;
    start->next = addFront->prev;
    size++;
}

void sort(node *start)
{
    int i;
    for (i = 0; i < size; i++)
    {
        node *pointer = start->next;
        node *next = pointer->next;
        while(pointer->next != start)
        {
            if ((next->current)->entryTime < (pointer->current)->entryTime)
            {
                node *hold = (node *)malloc(sizeof(node));
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

void insertSorted(node *start, process *data)
{
    if (size == 0) {
        insertFront(start, data);
        return;
    }
    node *pointer = start->next;
    node *trail = start;
    
    while(1)
    {
        if (data->runTime <= (pointer->current)->runTime)
        {
            node *insert = (node *)malloc(sizeof(node));
            insert->current = data;
            insert->next = pointer;
            insert->prev = trail;
            pointer->prev = insert;
            trail->next = insert;
            size++;
            return;
        }
        if (data->runTime > (pointer->current)->runTime && pointer->next == start)
        {
            node *insert = (node *)malloc(sizeof(node));
            insert->current = data;
            insert->prev = pointer;
            insert->next = start;
            pointer->next = insert;
            start->prev = insert;
            size++;
            return;
        }
        trail = pointer;
        pointer = pointer->next;
        //next = pointer->next;
    }
}

int find(node *start, int key)
{
    node *pointer = start->next;
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

node * locate(node *start, int key)
{
    //sort(start);
    node *pointer = start->next;
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

void delete(node *start, int pID)
{
    if (size <= 0) {
        printf("List is empty \n\n");
        return;
    }
    node *pointer = start->next;
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
    size--;
    
    return;
}

void removeFront(node *start){
    if (size > 0) {
        node *pointer = start->next;
        node *next = pointer->next;
        
        start->next = next;
        next->prev = start;
        
        free(pointer);
        size--;
    }
    else
    {
        printf("\nList is Empty\n\n");
    }
}

void removeBack(node *start){
    if (size > 0) {
        node *pointer = start->prev;
        node *next = pointer->prev;
        
        start->prev = next;
        next->next = start;
        
        free(pointer);
        size--;
    }
    else
    {
        printf("\nList is Empty\n\n");
    }
}