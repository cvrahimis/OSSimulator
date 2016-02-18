//
//  main.c
//  OSSimulator
//
//  Created by Costas Vrahimis on 2/17/16.
//  Copyright © 2016 Costas Vrahimis. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void *cpuClock(void *arg){
    int t = 0;
    while (t < 10) {
        t++;
        printf("Tick: %d \n", t);
        sleep(1);
    }
    
    pthread_exit(NULL);
}


int main(int argc, const char * argv[]) {
    int rc;
    pthread_t clockThread;
    
    rc = pthread_create(&clockThread, NULL, cpuClock, NULL);
    if(rc)
    {
        printf("ERROR. Return code from thread %d\n", rc);
        exit(-1);
    }
    
    rc = pthread_join(clockThread, NULL);
    if(rc)
    {
        printf("Error, return code from thread_join() is %d\n", rc);
        exit(-1);
    }
    printf("main completed join with thread clock thread \n");
    

    return 0;
}
