#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <time.h>

#define MAX_THREAD_COUNT 12
#define UPPER_BOUND 1000
#define LOWER_BOUND (UPPER_BOUND / 2)
#define MAX_BATHROOM_USES 50

int male_count = 0;
int female_count = 0;

int wrong = 0;

int waiting_males = 0;
int waiting_females = 0;

int thread_count;

pthread_t thread[MAX_THREAD_COUNT];
int thread_ids[MAX_THREAD_COUNT];

sem_t mutex;
sem_t wait_for_male;    
sem_t wait_for_female;  

void delay(uint32_t iterations) {
    for (uint32_t i = 0; i < iterations; i++) {
        __asm__ volatile ("nop");
    }
}

void use_bathroom(int thread_id) {
    int random = rand() % (UPPER_BOUND - LOWER_BOUND + 1) + LOWER_BOUND;
    delay(random);
    if(male_count > 0 && female_count > 0)
    {
        wrong = 1;
    }
    printf("thread %d: used bathroom for %d cycles\n", thread_id, random);
    printf("  Current bathroom state -> Males: %d, Females: %d\n", male_count, female_count);
}

void work(int thread_id) {
    int random = rand() % (500 - 250 + 1) + 250;
    delay(random);
    printf("thread %d: worked for %d cycles\n", thread_id, random);
}

void male_wants_to_enter(int thread_id) {
    printf("thread %d: male wants to enter\n", thread_id);
    sem_wait(&mutex);
    while (female_count > 0) {
        waiting_males++;
        sem_post(&mutex);
        printf("thread %d: male waiting (females present)\n", thread_id);
        sem_wait(&wait_for_female);
        sem_wait(&mutex);
        waiting_males--;
    }

    printf("thread %d: male enters\n", thread_id);
    male_count++;
    sem_post(&mutex);

    use_bathroom(thread_id);

    sem_wait(&mutex);
    male_count--;
    printf("thread %d: male exits\n", thread_id);
    if (male_count == 0 && waiting_females > 0) {
        for (int i = 0; i < waiting_females; i++) {
            sem_post(&wait_for_male);
        }
    }
    sem_post(&mutex);
}

void female_wants_to_enter(int thread_id) {
    printf("thread %d: female wants to enter\n", thread_id);
    sem_wait(&mutex);
    while (male_count > 0) {
        waiting_females++;
        sem_post(&mutex);
        printf("thread %d: female waiting (males present)\n", thread_id);
        sem_wait(&wait_for_male);
        sem_wait(&mutex);
        waiting_females--;
    }
    printf("thread %d: female enters\n", thread_id);
    female_count++;
    sem_post(&mutex);

    use_bathroom(thread_id);

    sem_wait(&mutex);
    female_count--;
    printf("thread %d: female exits\n", thread_id);
    if (female_count == 0 && waiting_males > 0) {
        for (int i = 0; i < waiting_males; i++) {
            sem_post(&wait_for_female);
        }
    }
    sem_post(&mutex);
}

void *male_worker(void *arg) {
    int thread_id = *((int *) arg);
    for (int i = 0; i < MAX_BATHROOM_USES; i++) {
        work(thread_id);
        male_wants_to_enter(thread_id);
    }
    printf("thread %d: CLOCKED OUT\n", thread_id);
    return NULL;
}

void *female_worker(void *arg) {
    int thread_id = *((int *) arg);
    for (int i = 0; i < MAX_BATHROOM_USES; i++) {
        work(thread_id);
        female_wants_to_enter(thread_id);
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

    sem_init(&mutex, 0, 1);
    sem_init(&wait_for_male, 0, 0);    // Initially no waiting female can proceed.
    sem_init(&wait_for_female, 0, 0);  // Initially no waiting male can proceed.

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
    if (wrong)
    {   
        printf("BAD SOLUTION!");
    } else {
        printf("GOOD SOLUTION!");
    }
    
    printf("\n##### PROGRAM FINISHED #####\n");
    return 0;
}