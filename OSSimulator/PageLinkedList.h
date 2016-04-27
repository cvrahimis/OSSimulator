//
//  PageLinkedList.h
//  OSSimulator
//
//  Created by Costas Vrahimis on 4/7/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#ifndef PageLinkedList_h
#define PageLinkedList_h

#include <stdio.h>

typedef struct page{
    int startAddress;
    int size;
    struct page *next;
    struct page *prev;
}page;

typedef page *pagePointer;

void printPages(page *start, int index);
//void cll_enqueue(page *start, page *data);
//page *cll_dequeue(page *start);
//void insertBack(page *pointer, page *data);
//void sort(circularlistnode *start);
void insertPageSorted(page *start, page *newPage);
//void insertFront(page *pointer, page *data);
//int find(circularlistnode *pointer, int key);
//void del(page *pointer, page pID);
page* removeFrontPage(page *start);
page* removeBackPage(page *start);
page* getBuddyFree(page *start, int buddyStartAddress);
void removePage(page *node);
//circularlistnode * locate(circularlistnode *start, int key);
//int size();
//page* getNextPage(page *start);
//void printData(page *doneStart, page *pointer);

#endif /* PageLinkedList_h */
