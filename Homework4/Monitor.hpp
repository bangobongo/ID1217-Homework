
#ifndef MYCLASS_H
#define MYCLASS_H

#include <pthreads.h>

#define MAX_THREAD_COUNT 12
#define UPPER_BOUND 1000
#define LOWER_BOUND (UPPER_BOUND / 2)
#define MAX_BATHROOM_USES 50

class Monitor {
public:
    int wrong;
    Monitor();
    void manEnter(int thread_id);
    void manExit(int thread_id);
    void womanEnter(int thread_id);
    void womanExit(int thread_id);
};

#endif // MYCLASS_H

