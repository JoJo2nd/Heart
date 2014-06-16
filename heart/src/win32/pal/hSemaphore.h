/********************************************************************

    filename: 	DeviceSemaphore.h	
    
    Copyright (c) 20:3:2011 James Moran
    
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