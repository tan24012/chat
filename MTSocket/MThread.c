#include "MThread.h"

static void* thread_entry(void* t) {
    MThread* mt = (MThread*)t;

    if (mt->run != NULL) {
        mt->run(mt->arg);
    }
    
    return NULL;
}

void mt_init(MThread *t) {
    t->run = NULL;
    t->arg = NULL;
    t->started = false;
    t->joined = false;
}

int mt_start(MThread *t) { 
    int ret = pthread_create(&t->threadId, NULL, thread_entry, t);
    if (ret == 0) {
        t->started = true;
        t->joined = false;
    }
    return ret;
}

int mt_wait(MThread *t) {
    if (t == NULL || !t->started || t->joined) {
        return 0;
    }

    int ret = pthread_join(t->threadId, NULL);
    if (ret == 0) {
        t->joined = true;
    }
    return ret;
}

