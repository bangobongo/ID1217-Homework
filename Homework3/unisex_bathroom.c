#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "unisex_bathroom.h"

#define MAX_THREAD_COUNT 8
#define UPPER_BOUND 10
#define LOWER_BOUND UPPER_BOUND / 2
#define MAX_BATHROOM_USES 30

// Compile the code by linking with -lpthread -lrt -o0

int male_count = 0;
int female_count = 0;
int thread_count;

pthread_t thread[MAX_THREAD_COUNT];

sem_t wait_for_opposite_gender;
sem_t mutex;
sem_t gender_count;

// take thread count as argument
int main(int argc, char *argv[])
{

    printf("Program stareted\n");

    thread_count = (argc < MAX_THREAD_COUNT) ? argc : MAX_THREAD_COUNT;

    sem_init(&wait_for_opposite_gender, 0, 0);
    sem_init(&mutex, 0, 0);
    sem_init(&gender_count, 0, 0);

    printf("sems initialized \n");

    for (int thread_id = 0; thread_id < thread_count; thread_id++)
    {
        int id = thread_id;
        if (thread_id % 2 == 0)
        {
            pthread_create(&thread[thread_id], NULL, male_worker, &id);
            printf("thread %d created\n", id);
        }
        else
        {
            pthread_create(&thread[thread_id], NULL, female_worker, &id);
            printf("thread %d created\n", id);
        }
        
    }

    printf("threads made\n");

    for (int thread_id = 0; thread_id < thread_count; thread_id)
    {
        pthread_join(thread[thread_id], NULL);
    }

    printf("threads done\n");

    return 0;
}

void use_bathroom(int thread_id)
{
    int random = rand() % ((UPPER_BOUND) - (LOWER_BOUND) + 1) + (LOWER_BOUND);

    for (int i = 0; i < random; i++)
    {
    }
    printf("thread %d used bathroom for %d cycles\n", thread_id, random);
}

void male_wants_to_enter(int thread_id)
{
    printf("thread %d: male wants to enter\n", thread_id);
    sem_wait(&mutex);
    if (male_count == 0)
    {
        sem_wait(&wait_for_opposite_gender);
    }
    male_count++;
    sem_post(&mutex);

    use_bathroom(thread_id);

    sem_wait(&mutex);
    male_count--;

    if (male_count == 0)
    {
        sem_post(&wait_for_opposite_gender);
    }
}

void female_wants_to_enter(int thread_id)
{
    printf("thread %d: female wants to enter\n", thread_id);
    sem_wait(&mutex);
    if (female_count == 0)
    {
        sem_wait(&wait_for_opposite_gender);
    }
    female_count++;
    sem_post(&mutex);

    use_bathroom(thread_id);

    sem_wait(&mutex);
    female_count--;

    if (female_count == 0)
    {
        sem_post(&wait_for_opposite_gender);
    }
}

void work(int thread_id)
{
    int random = rand() % (UPPER_BOUND - LOWER_BOUND + 1) + LOWER_BOUND;

    for (int i = 0; i < random; i++)
    {
    }
    printf("thread %d worked for %d cycles\n", thread_id, random);
}

void *male_worker(void *thread_id_ptr)
{
    int *nice = (int *)thread_id_ptr;
    int thread_id = *nice;
    printf("Thread mem pos: %d", &thread_id);
    for (int bathroom_uses = 0; bathroom_uses < MAX_BATHROOM_USES; bathroom_uses++)
    {
        work(thread_id);
        male_wants_to_enter(thread_id);
    }
}

void *female_worker(void *thread_id_ptr)
{
    int *nice = (int *)thread_id_ptr;
    int thread_id = *nice;
    printf("Thread mem pos: %d", &thread_id);
    for (int bathroom_uses = 0; bathroom_uses < MAX_BATHROOM_USES; bathroom_uses++)
    {
        work(thread_id);
        female_wants_to_enter(thread_id);
    }
}
