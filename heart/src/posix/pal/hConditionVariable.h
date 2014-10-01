/********************************************************************

    filename:   hConditionVariable.h  
    
    Copyright (c) 19:10:2013 James Moran
    
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    
    3. This notice may not be removed or altered from any source
    distribution.

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