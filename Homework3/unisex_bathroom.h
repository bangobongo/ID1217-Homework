
void *female_worker(void *thread_id);
void *male_worker(void *thread_id);
void work(int thread_id);
void use_bathroom(int thread_id);
void female_wants_to_enter(int thread_id);
void male_wants_to_enter(int thread_id);
void delay(uint32_t iterations);