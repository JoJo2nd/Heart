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

#include <winsock2.h>
#include <windows.h>
#include "base/hTypes.h"
#include "base/hFunctor.h"

namespace Heart
{
namespace Device
{
    hFORCEINLINE
    void* HEART_API GetCurrentThreadID() { return (void*)GetCurrentThreadId(); }
    hFORCEINLINE
    void  HEART_API ThreadSleep( DWORD dwMilliseconds ) { Sleep( dwMilliseconds ); }
    hFORCEINLINE
    void  HEART_API ThreadYield() { SwitchToThread(); }
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
        hBool			isComplete();
        hUint32			returnCode() { return returnCode_; }
        void            join(){ WaitForSingleObject(ThreadHand_, INFINITE); }

    private:

        static const int THREAD_NAME_SIZE = 32;

        static void SetThreadName( LPCSTR szThreadName );
        static unsigned long WINAPI staticFunc( LPVOID pParam );

        hChar							threadName_[THREAD_NAME_SIZE];
        hThreadFunc		                threadFunc_;
        void*							pThreadParam_;	
        HANDLE							ThreadHand_;
        hInt32							priority_;
        hUint32							returnCode_;
    };
}
#endif // THREAD_H__