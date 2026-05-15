#ifndef MTHREAD_H
#define MTHREAD_H

#include <pthread.h>


typedef struct {
    pthread_t threadId;
    void (*run)(void*);
    void* arg;
} MThread;

int mt_start(MThread *t);
int mt_wait(MThread *t);

#endif
