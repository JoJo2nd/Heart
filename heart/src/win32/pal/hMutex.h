/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef DEVICEMUTEX_H__
#define DEVICEMUTEX_H__

#include <winsock2.h>
#include <windows.h>

namespace Heart
{
    class hMutex
    {
    public:
        hMutex() {
            InitializeCriticalSection( &mutex_ );
        }
        void Lock() {
            EnterCriticalSection( &mutex_ );
        }
        hBool TryLock() {
            BOOL ret = TryEnterCriticalSection( &mutex_ );
            return ret == TRUE ? hTrue : hFalse;
        }
        void Unlock() {
            LeaveCriticalSection( &mutex_ );
        }
        ~hMutex() {
            DeleteCriticalSection( &mutex_ );
        }
    
        _RTL_CRITICAL_SECTION	mutex_;
    };
}

#endif // DEVICEMUTEX_H__