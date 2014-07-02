/********************************************************************

    filename:   hDeviceThread.cpp  
    
    Copyright (c) 26:12:2012 James Moran
    
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

#include "pal/hDeviceThread.h"
#include "base/hMemoryUtil.h"
#include <sched.h>

namespace Heart {

    void hThread::create(const hChar* threadName, hInt32 priority, hThreadFunc pFunctor, void* param) {
        hMemCpy( threadName_, threadName, THREAD_NAME_SIZE );
        threadFunc_ = pFunctor;
        threadParam_ = param;
        priority_ = priority;

        hInt prio_min = sched_get_priority_min(SCHED_RR);
        hInt prio_max = sched_get_priority_max(SCHED_RR);
        hInt prio_seg = (prio_max - prio_min) / (PRIORITY_HIGH-PRIORITY_LOWEST);

        if ( priority_ < -2 ) {
            priority_ = -2;
        }
        if ( priority_ > 2 ) {
            priority_ = 2;
        }

        sched_param sp;
        sp.sched_priority = prio_min + (prio_seg * (priority_+2));

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setstacksize(&attr, 1024*1024*2);
        pthread_attr_setschedpolicy(&attr, SCHED_RR);
        pthread_attr_setschedparam(&attr, &sp);
        pthread_create(&thread_, &attr, hThread::staticFunc, this);

        pthread_attr_destroy(&attr);
    }

    void hThread::join() {
        pthread_join(thread_, nullptr);
    }

    void* hThread::staticFunc(void* param) {
        hThread* this_ = (hThread*)param;
        this_->returnCode_ = this_->threadFunc_( this_->threadParam_ );
        return (void*)this_->returnCode_;
    }

}