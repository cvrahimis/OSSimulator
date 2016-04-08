//
//  PageLinkedList.c
//  OSSimulator
//
//  Created by Costas Vrahimis on 4/7/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#include "PageLinkedList.h"


void insertPageSorted(page *start, page *newPage){
    if(start->next == start && start->prev == start)
    {
        start->next = newPage;
        start->prev = newPage;
        newPage->next = start;
        newPage->prev = start;
    }
    else
    {
        page *pointer = start->next;
        while(pointer != start || newPage->startAddress < pointer->startAddress){
            pointer= pointer->next;
        }
        page *temp = pointer->prev;
        temp->next = newPage;
        newPage->prev = temp;
        newPage->next = pointer;
        pointer->prev = newPage;
    }
}

void printPages(page *start, int index){
    page *pointer = start->next;
    int hasData = 0;
    while (pointer != start) {
        printf("index: %d startAddress: %d and size: %d || ", index, pointer->startAddress, pointer->size);
        pointer = pointer->next;
        hasData = 1;
    }
    if (hasData) {
        printf("\n");
    }
}

page* removeBackPage(page *start){
    if (start->next != start && start->prev != start) {
        page *pageToRemove = start->prev;
        page *last = pageToRemove->prev;
    
        start->prev = last;
        last->next = start;
    
        pageToRemove->next = pageToRemove;
        pageToRemove->prev = pageToRemove;
        return pageToRemove;
    }
    else
        return NULL;
}

page* removeFrontPage(page *start){
    if (start->next != start && start->prev != start) {
        page *pageToRemove = start->next;
        page *first = pageToRemove->next;
        
        start->next = first;
        first->prev = start;
        
        pageToRemove->next = pageToRemove;
        pageToRemove->prev = pageToRemove;
        return pageToRemove;
    }
    else
        return NULL;
}

void removePage(page *node){
    page *previous = node->prev;
    page *next = node->next;
    
    previous->next = next;
    next->prev = previous;
}

page* getBuddyFree(page *start, int buddyStartAddress){
    page *pointer = start->next;
    int found = 0;
    while(pointer != start && !found) {
        if (pointer->startAddress == buddyStartAddress) {
            found = 1;
        }
        pointer = pointer->next;
    }
    if(found)
        return pointer;
    else
        return NULL;
}