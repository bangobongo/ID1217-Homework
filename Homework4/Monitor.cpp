
#include <pthread.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#define MAX_THREAD_COUNT 12
#define UPPER_BOUND 1000000
#define LOWER_BOUND (UPPER_BOUND / 2)
#define MAX_BATHROOM_USES 4

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
        pthread_mutex_lock(&mutex);
        if(this->male_count > 0 && this->female_count > 0)
        {
            this->wrong = 1;
        }
        printf("thread %d: used bathroom for %d cycles\n", thread_id, random);
        printf("  Current bathroom state -> Males: %d, Females: %d\n", male_count, female_count);
        pthread_mutex_unlock(&mutex);
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
        this->male_count = 0;
        this->female_count = 0;
        this->waiting_males = 0;
        this->waiting_females = 0;
        this->wrong = 0;
        pthread_mutex_init(&this->mutex, NULL);
        pthread_cond_init(&this->males_in_bathroom, NULL);
        pthread_cond_init(&this->females_in_bathroom, NULL);
    }

    void manEnter(int thread_id) {
        pthread_mutex_lock(&mutex);
        if(this->female_count > 0) 
        {
            this->waiting_males++;
            pthread_cond_wait(&this->females_in_bathroom, &this->mutex);
            this->waiting_males--;
        }
        this->male_count++;
        pthread_mutex_unlock(&this->mutex);
        use_bathroom(thread_id);
    }

    void manExit(int thread_id) {
        pthread_mutex_lock(&this->mutex);
        this->male_count--;
        if(this->male_count == 0) {
            pthread_cond_signal(&this->males_in_bathroom);
        }
        pthread_mutex_unlock(&this->mutex);
    }

    void womanEnter(int thread_id) {
        pthread_mutex_lock(&this->mutex);
        if(this->male_count > 0) 
        {
            this->waiting_females++;
            pthread_cond_wait(&this->males_in_bathroom, &this->mutex);
            this->waiting_females--;
        }
        this->female_count++;
        pthread_mutex_unlock(&this->mutex);
        use_bathroom(thread_id);
    }

    void womanExit(int thread_id) {
        pthread_mutex_lock(&this->mutex);
        this->female_count--;
        if(this->female_count == 0) {
            pthread_cond_signal(&this->females_in_bathroom);
        }
        pthread_mutex_unlock(&this->mutex);
    }
};

















