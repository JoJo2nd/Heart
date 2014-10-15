/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "pal/hDeviceThread.h"
#include "threading/hThreadLocalStorage.h"

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