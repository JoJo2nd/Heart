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

#include <semaphore.h>

namespace Heart
{
    class hSemaphore {
    public:

        bool Create( hUint32 initCount, hUint32 maxCount ) {
            return sem_init(&sema_, initCount, maxCount) == 0;
        }
        void Wait() {
            sem_wait(&sema_);
        }
        hBool poll() {
            timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 0;
            return sem_timedwait(&sema_, &ts) == 0;
        }
        void Post() {
            sem_post(&sema_);
        }
        void Destroy() {
            sem_destroy(&sema_);
        }

    private:

        sem_t sema_;
    };
}

#endif // DEVICESEMAPHORE_H__