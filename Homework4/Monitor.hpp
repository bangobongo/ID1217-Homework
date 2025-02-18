#pragma once

#ifndef MYCLASS_H
#define MYCLASS_H

#include <pthread.h>

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

