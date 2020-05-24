#ifndef _LOCK_H
#define _LOCK_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

class sem {
    sem_t id;

public:
    explicit sem(int num = 0) {
        if (sem_init(&id, 0, num) != 0)
            throw std::exception();
    }

    ~sem() {
        sem_destroy(&id);
    }

    bool wait() {
        return sem_wait(&id) == 0;
    }

    bool post() {
        return sem_post(&id) == 0;
    }
};

class locker {
    pthread_mutex_t id;

public:
    locker() {
        if (pthread_mutex_init(&id, nullptr) != 0)
            throw std::exception();
    }

    ~locker() {
        pthread_mutex_destroy(&id);
    }

    bool lock() {
        return pthread_mutex_lock(&id) == 0;
    }

    bool unlock() {
        return pthread_mutex_unlock(&id) == 0;
    }

    pthread_mutex_t *get() {
        return &id;
    }
};

class cond {
    pthread_cond_t id;

public:
    cond() {
        if (pthread_cond_init(&id, nullptr) != 0)
            throw std::exception();
    }

    ~cond() {
        pthread_cond_destroy(&id);
    }

    bool wait(locker &mutex) {
        return pthread_cond_wait(&id, mutex.get()) == 0;
    }

    bool timewait(locker &mutex, timespec &t) {
        return pthread_cond_timedwait(&id, mutex.get(), &t) == 0;
    }

    bool broadcast() {
        return pthread_cond_broadcast(&id) == 0;
    }

    bool signal() {
        return pthread_cond_signal(&id) == 0;
    }
};

#endif