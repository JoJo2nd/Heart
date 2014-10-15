/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/


#pragma once

#include <pthread.h>

namespace Heart {
    class hMutex {
    public:
        hMutex() {
            pthread_mutexattr_t attr;

            pthread_mutexattr_init(&attr);
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
            pthread_mutex_init(&mutex_, &attr);
            pthread_mutexattr_destroy(&attr);
        }
        void Lock() {
            pthread_mutex_lock(&mutex_);
        }
        hBool TryLock() {
            return pthread_mutex_trylock(&mutex_) ? hTrue : hFalse;
        }
        void Unlock() {
            pthread_mutex_unlock(&mutex_);
        }
        ~hMutex() {
            pthread_mutex_destroy(&mutex_);
        }
    
        pthread_mutex_t mutex_;
    };
}