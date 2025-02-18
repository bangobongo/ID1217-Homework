
#include "Monitor.hpp"
#include <pthread.h>

class Monitor {

private:
    // Number of males currently in the bathroom
    int male_count;

    // Number of females currently in the bathroom
    int female_count;

    // Number of males currently waiting to use the bathroom
    int waiting_males;

    // Number of females currently waiting to use the bathroom.
    int waiting_females;

    // General mutex
    pthread_mutex_t mutex;

    pthread_cond_t males_in_bathroom;
    pthread_cond_t females_in_bathroom;

    void use_bathroom(int thread_id) {
        int random = rand() % (UPPER_BOUND - LOWER_BOUND + 1) + LOWER_BOUND;
        delay(random);
        if(male_count > 0 && female_count > 0)
        {
            this.wrong = 1;
        }
        printf("thread %d: used bathroom for %d cycles\n", thread_id, random);
        printf("  Current bathroom state -> Males: %d, Females: %d\n", male_count, female_count);
    }

    void delay(uint32_t iterations) {
        for (uint32_t i = 0; i < iterations; i++) {
            __asm__ volatile ("nop");
        }
    }

public:
    int wrong;

    // Constructor
    Monitor() {
        this.male_count = 0;
        this.female_count = 0;
        this.waiting_males = 0;
        this.waiting_females = 0;
        this.wrong = 0;
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&males_in_bathroom, NULL);
        pthread_cond_init(&females_in_bathroom, NULL);
    }

    void manEnter(int thread_id) {
        pthread_mutex_lock(&mutex);
        while(female_count > 0) 
        {
            waiting_males++;
            pthread_cond_wait(&females_in_bathroom, &mutex);
            waiting_males--;
        }
        male_count++;
        pthread_mutex_unlock(&mutex);
        use_bathroom(thread_id);
    }

    void manExit(int thread_id) {
        pthread_mutex_lock(&mutex);
        male_count--;
        if(male_count == 0) {
            pthread_cond_signal(&males_in_bathroom);
        }
        pthread_mutex_unlock(&mutex);
    }

    void womanEnter(int thread_id) {
        pthread_mutex_lock(&mutex);
        while(male_count > 0) 
        {
            waiting_females++;
            pthread_cond_wait(&males_in_bathroom, &mutex);
            waiting_females--;
        }
        female_count++;
        pthread_mutex_unlock(&mutex);
        use_bathroom(thread_id);
    }

    void womanExit(int thread_id) {
        pthread_mutex_lock(&mutex);
        female_count--;
        if(female_count == 0) {
            pthread_cond_signal(&females_in_bathroom);
        }
        pthread_mutex_unlock(&mutex);
    }
};

















