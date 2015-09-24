/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include <winsock2.h>
#include <windows.h>
#include "base/hTypes.h"
#include "base/hFunctor.h"

namespace Heart
{
namespace Device
{
    hFORCEINLINE
    void* HEART_API GetCurrentThreadID() { return (void*)((hUintptr_t)GetCurrentThreadId()); }
    hFORCEINLINE
    void  HEART_API ThreadSleep( DWORD dwMilliseconds ) { Sleep( dwMilliseconds ); }
    hFORCEINLINE
    void  HEART_API ThreadYield() { SwitchToThread(); }
}

    hFUNCTOR_TYPEDEF(hUint32(*)(void*), hThreadFunc);

    class HEART_CLASS_EXPORT hThread
    {
    public:
        hThread();
        hThread(const hThread& rhs) = delete;
        hThread& operator == (const hThread& rhs) = delete;
        ~hThread();

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
        void            join(){ WaitForSingleObject(ThreadHand_, INFINITE); }

    private:

        static const int THREAD_NAME_SIZE = 32;

        static void SetThreadName( LPCSTR szThreadName );
        static unsigned long WINAPI staticFunc( LPVOID pParam );

        hChar							threadName_[THREAD_NAME_SIZE];
        hThreadFunc*                    threadFunc_;
        void*							pThreadParam_;	
        HANDLE							ThreadHand_;
        hInt32							priority_;
        hUint32							returnCode_;
    };
}