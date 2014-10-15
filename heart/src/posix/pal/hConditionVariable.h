/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#ifndef HCONDITIONVARIABLE_H__
#define HCONDITIONVARIABLE_H__

namespace Heart {
    class hConditionVariable {
    public:
        hConditionVariable() {
            pthread_condattr_t attr;
            pthread_condattr_init(&attr);
            pthread_cond_init(&var_, &attr);
            pthread_condattr_destroy(&attr);
        }
        ~hConditionVariable() {
            pthread_cond_destroy(&var_);
        }
        hBool tryWait(hMutex* mtx) {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 0;
            return pthread_cond_timedwait(&var_, &mtx->mutex_, &ts) == 0;
        }
        void wait(hMutex* mtx) {
            pthread_cond_wait(&var_, &mtx->mutex_);
        }
        /* Wake a single thread waiting on CV */
        void signal() {
            pthread_cond_signal(&var_);
        }
        /* Wake ALL threads waiting on CV */
        void broadcast() {
            pthread_cond_broadcast(&var_);
        }
    private:
        hConditionVariable(const hConditionVariable&) {}
        hConditionVariable& operator = (const hConditionVariable&) { return *this; }

        pthread_cond_t var_;
    };
}

#endif // HCONDITIONVARIABLE_H__