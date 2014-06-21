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

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hThread::create(const hChar* threadName, hInt32 priority, hThreadFunc pFunctor, void* param)
    {
        memcpy( threadName_, threadName, THREAD_NAME_SIZE );
        threadFunc_ = pFunctor;
        pThreadParam_ = param;
        priority_ = priority;
        if ( priority_ < -2 )
        {
            priority_ = -2;
        }
        if ( priority_ > 2 )
        {
            priority_ = 2;
        }
        ThreadHand_ = CreateThread( NULL, (1024*1024)*2, staticFunc, this, 0, NULL );
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hThread::isComplete()
    {
        DWORD exitCode;
        GetExitCodeThread( ThreadHand_, &exitCode );
        return exitCode != STILL_ACTIVE;
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hThread::SetThreadName(LPCSTR szThreadName)
    {
#pragma pack ( push,8 )
        typedef struct tagTHREADNAME_INFO
        {
            DWORD dwType; // must be 0x1000
            LPCSTR szName; // pointer to name (in user addr space)
            DWORD dwThreadID; // thread ID (-1=caller thread)
            DWORD dwFlags; // reserved for future use, must be zero
        } THREADNAME_INFO;
#pragma pack ( pop )
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = szThreadName;
        info.dwThreadID = -1;//caller thread
        info.dwFlags = 0;

        __try
        {
            RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (ULONG_PTR*)&info );
        }
        __except(EXCEPTION_CONTINUE_EXECUTION)
        {
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    unsigned long WINAPI hThread::staticFunc(LPVOID pParam)
    {
        hThread* pThis_ = (hThread*)pParam;
        SetThreadName( pThis_->threadName_ );
        SetThreadPriority( pThis_->ThreadHand_, pThis_->priority_ );
        pThis_->returnCode_ = pThis_->threadFunc_( pThis_->pThreadParam_ );
        TLS::threadExit();
        return pThis_->returnCode_;
    }

}