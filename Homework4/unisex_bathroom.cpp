#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "Monitor.cpp"

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
    int random = rand() % (UPPER_BOUND - LOWER_BOUND + 1) + LOWER_BOUND;
    delay(random);
    printf("%llu: thread %d: worked for %d cycles\n", Clock::now() - bathroom.start_time, thread_id, random);
}

void *male_worker(void *arg) {
    int thread_id = *((int *) arg);
    for (int i = 0; i < MAX_BATHROOM_USES; i++) {
        work(thread_id);
        bathroom.manEnter(thread_id);
        bathroom.manExit(thread_id);
    }
    printf("%llu: thread %d: CLOCKED OUT\n", Clock::now() - bathroom.start_time, thread_id);
    return NULL;
}

void *female_worker(void *arg) {
    int thread_id = *((int *) arg);    
    for (int i = 0; i < MAX_BATHROOM_USES; i++) {
        work(thread_id);
        bathroom.womanEnter(thread_id);
        bathroom.womanExit(thread_id);
    }
    printf("%llu: thread %d: CLOCKED OUT\n", Clock::now() - bathroom.start_time, thread_id);
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

    printf("%llu: Creating %d threads\n", Clock::now() - bathroom.start_time, thread_count);
    for (int i = 0; i < thread_count; i++) {
        thread_ids[i] = i;
        if (i % 2 == 0) {
            printf("%llu: Creating male thread %d\n", Clock::now() - bathroom.start_time, i);
            pthread_create(&thread[i], NULL, male_worker, &thread_ids[i]);
        } else {
            printf("%llu: Creating female thread %d\n", Clock::now() - bathroom.start_time, i);
            pthread_create(&thread[i], NULL, female_worker, &thread_ids[i]);
        }
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(thread[i], NULL);
    }
    if (bathroom.wrong)
    {   
        printf("%llu: BAD SOLUTION!", Clock::now() - bathroom.start_time);
    } else {
        printf("%llu: GOOD SOLUTION!", Clock::now() - bathroom.start_time);
    }
    
    printf("\n##### PROGRAM FINISHED (%llu) #####\n", Clock::now() - bathroom.start_time);
    return 0;
}

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    