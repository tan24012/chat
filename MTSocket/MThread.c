#include "MThread.h"

static void* thread_entry(void* t) {
    MThread* mt = (MThread*)t;

    if (mt->run != NULL) {
        mt->run(mt->arg);
    }
    
    return NULL;
}

int mt_start(MThread *t) { 
    return pthread_create(&t->threadId, NULL, thread_entry, t);
}

int mt_wait(MThread *t) {
    return pthread_join(t->threadId, NULL);
}
