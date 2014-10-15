/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef THREAD_H__
#define THREAD_H__

#include "base/hTypes.h"
#include "base/hFunctor.h"
#include <pthread.h>

namespace Heart
{
namespace Device
{
    hFORCEINLINE void* HEART_API GetCurrentThreadID() { 
        return (void*)pthread_self();
    }
    hFORCEINLINE void  HEART_API ThreadSleep(hUint milliseconds) {
#if _POSIX_C_SOURCE >= 199309L
        hUint secs = milliseconds/1000;
        timespec t;
        t.tv_sec = (time_t)(secs+.5f);
        t.tv_nsec = (long)(secs - t.tv_sec)*1000000000;
        nanosleep(&t, nullptr);
#else        
        sleep((uint_t)(secs+.5f));
#endif
    }
    hFORCEINLINE void  HEART_API ThreadYield() {
        sched_yield();
    }
}

    hFUNCTOR_TYPEDEF(hUint32(*)(void*), hThreadFunc);

    class hThread
    {
    public:

        enum Priority
        {
            PRIORITY_LOWEST			= -2,
            PRIORITY_BELOWNORMAL	= -1,
            PRIORITY_NORMAL			= 0,
            PRIORITY_ABOVENORMAL	= 1,
            PRIORITY_HIGH			= 2,
        };

        void			create( const hChar* threadName, hInt32 priority, hThreadFunc pFunctor, void* param );
        hUint32			returnCode() { return returnCode_; }
        void            join();

    private:

        static const int THREAD_NAME_SIZE = 32;
        static void* staticFunc(void* param);

        hChar							threadName_[THREAD_NAME_SIZE];
        hThreadFunc		                threadFunc_;
        void*							threadParam_;	
        pthread_t                       thread_;
        hInt32							priority_;
        hUint32							returnCode_;
    };
}
#endif // THREAD_H__