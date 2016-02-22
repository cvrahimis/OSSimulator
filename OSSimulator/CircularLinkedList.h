//
//  CircularLinkedList.h
//  CircularLinkedList
//
//  Created by Costas Vrahimis on 1/31/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#ifndef CircularLinkedList_h
#define CircularLinkedList_h

#include <stdio.h>
#include <stdlib.h>

typedef struct Process
{
    int pID;
    int entryTime;
    int runTime;
    int timeEnteredReadyQ;
    int timeEnteredCUP;
    int timeDone;
    //int memory;
}process;


typedef struct Node
{
    struct Process *current;
    struct Node *next;
    struct Node *prev;
}node;

void print(node *start,node *pointer);
void insertBack(node *pointer, process *data);
void sort(node *start);
void insertSorted(node *start, process *data);
void insertFront(node *pointer, process *data);
int find(node *pointer, int key);
void delete(node *pointer, int pID);
void removeFront(node *start);
void removeBack(node *start);
node * locate(node *start, int key);
int size();

#endif 
