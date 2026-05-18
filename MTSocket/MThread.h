#ifndef MTHREAD_H
#define MTHREAD_H

#include <pthread.h>
#include <stdbool.h>

typedef struct {
    pthread_t threadId;
    void (*run)(void*);
    void* arg;
    bool started;
    bool joined;
} MThread;

void mt_init(MThread *t);
int mt_start(MThread *t);
int mt_wait(MThread *t);

#endif
