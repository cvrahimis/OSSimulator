//
//  CircularLinkedList.h
//  CircularLinkedList
//
//  Created by Costas Vrahimis on 1/31/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#ifndef CircularLinkedList_h
#define CircularLinkedList_h

#include "process.h"

typedef struct CircularListNode
{
    struct Process *current;
    struct CircularListNode *next;
    struct CircularListNode *prev;
} circularlistnode;

void print(circularlistnode *start,circularlistnode *pointer);
void enqueue(circularlistnode *start, process *data);
process *dequeue(circularlistnode *start);
void insertBack(circularlistnode *pointer, process *data);
//void sort(circularlistnode *start);
//void insertSorted(circularlistnode *start, process *data);
void insertFront(circularlistnode *pointer, process *data);
//int find(circularlistnode *pointer, int key);
void del(circularlistnode *pointer, int pID);
//void removeFront(circularlistnode *start);
//void removeBack(circularlistnode *start);
//circularlistnode * locate(circularlistnode *start, int key);
//int size();
void printData(circularlistnode *doneStart, circularlistnode *pointer);

#endif 
