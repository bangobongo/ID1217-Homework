#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include "Monitor.hpp"

int thread_count;

Monitor bathroom = Monitor();

pthread_t thread[MAX_THREAD_COUNT];
int thread_ids[MAX_THREAD_COUNT];

void delay(uint32_t iterations) {
    for (uint32_t i = 0; i < iterations; i++) {
        __asm__ volatile ("nop");
    }
}

void work(int thread_id) {
    int random = rand() % (500 - 250 + 1) + 250;
    delay(random);
    printf("thread %d: worked for %d cycles\n", thread_id, random);
}

void *male_worker(void *arg) {
    int thread_id = *((int *) arg);
    for (int i = 0; i < MAX_BATHROOM_USES; i++) {
        work(thread_id);
        bathroom.manEnter();
        bathroom.manExit();
    }
    printf("thread %d: CLOCKED OUT\n", thread_id);
    return NULL;
}

void *female_worker(void *arg) {
    int thread_id = *((int *) arg);
    for (int i = 0; i < MAX_BATHROOM_USES; i++) {
        work(thread_id);
        bathroom.womanEnter();
        bathroom.womanExit();
    }
    printf("thread %d: CLOCKED OUT\n", thread_id);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }
    thread_count = atoi(argv[1]);
    if (thread_count > MAX_THREAD_COUNT) {
        thread_count = MAX_THREAD_COUNT;
    }
    printf("Creating %d threads\n", thread_count);

    for (int i = 0; i < thread_count; i++) {
        thread_ids[i] = i;
        if (i % 2 == 0) {
            printf("Creating male thread %d\n", i);
            pthread_create(&thread[i], NULL, male_worker, &thread_ids[i]);
        } else {
            printf("Creating female thread %d\n", i);
            pthread_create(&thread[i], NULL, female_worker, &thread_ids[i]);
        }
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(thread[i], NULL);
    }
    if (bathroom.wrong)
    {   
        printf("BAD SOLUTION!");
    } else {
        printf("GOOD SOLUTION!");
    }
    
    printf("\n##### PROGRAM FINISHED #####\n");
    return 0;
}

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    