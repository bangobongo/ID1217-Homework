
#include <pthread.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#define MAX_THREAD_COUNT 12
#define UPPER_BOUND 1000000
#define LOWER_BOUND (UPPER_BOUND / 2)
#define MAX_BATHROOM_USES 30

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
        printf("thread %d: male wants to enter\n", thread_id);
        pthread_mutex_lock(&mutex);
        while(this->female_count > 0) 
        {
            this->waiting_males++;
            printf("thread %d: male waiting (males present)\n", thread_id);
            pthread_cond_wait(&this->females_in_bathroom, &this->mutex);
            this->waiting_males--;
        }
        printf("thread %d: male enters\n", thread_id);
        this->male_count++;
        pthread_mutex_unlock(&this->mutex);
        use_bathroom(thread_id);
    }

    void manExit(int thread_id) {
        pthread_mutex_lock(&this->mutex);
        printf("thread %d: male exits\n", thread_id);
        this->male_count--;
        if(this->male_count == 0) {
            printf("thread %d: signal empty bathroom to females\n", thread_id);
            for(int i = 0; i < 10; i++) {
                pthread_cond_signal(&this->males_in_bathroom);
            }
        }
        pthread_mutex_unlock(&this->mutex);
    }

    void womanEnter(int thread_id) {
        printf("thread %d: female wants to enter\n", thread_id);
        pthread_mutex_lock(&this->mutex);
        while(this->male_count > 0) 
        {
            this->waiting_females++;
            printf("thread %d: female waiting (males present)\n", thread_id);
            pthread_cond_wait(&this->males_in_bathroom, &this->mutex);
            this->waiting_females--;
        }
        printf("thread %d: female enters\n", thread_id);
        this->female_count++;
        pthread_mutex_unlock(&this->mutex);
        use_bathroom(thread_id);
    }

    void womanExit(int thread_id) {
        pthread_mutex_lock(&this->mutex);
        printf("thread %d: female exits\n", thread_id);
        this->female_count--;
        if(this->female_count == 0) {
            printf("thread %d: signal empty bathroom to males\n", thread_id);
            for(int i = 0; i < 10; i++) {
                pthread_cond_signal(&this->females_in_bathroom);
            }
            
        }
        pthread_mutex_unlock(&this->mutex);
    }
};

















