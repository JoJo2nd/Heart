/********************************************************************

    filename: 	hThread.h	
    
    Copyright (c) 7:7:2012 James Moran
    
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