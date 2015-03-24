/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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
        pthread_setname_np(thread_, threadName_);

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