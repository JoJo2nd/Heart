/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef DEVICESEMAPHORE_H__
#define DEVICESEMAPHORE_H__

namespace Heart
{
    class hSemaphore
    {
    public:
        bool Create( hUint32 initCount, hUint32 maxCount ) {
            hSema_ = CreateSemaphore( NULL, initCount, maxCount, NULL );
            return hSema_ != INVALID_HANDLE_VALUE;
        }
        void Wait() {
            WaitForSingleObject(hSema_, INFINITE);
        }
        hBool poll() {
            DWORD ret=WaitForSingleObject(hSema_, 0);
            return ret == WAIT_OBJECT_0 ? hTrue : hFalse;
        }
        void Post() {
            ReleaseSemaphore( hSema_, 1, NULL );
        }
        void Destroy() {
            CloseHandle( hSema_ );
        }

    private:

        HANDLE hSema_;
    };
}

#endif // DEVICESEMAPHORE_H__