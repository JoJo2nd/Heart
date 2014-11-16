/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef DEVICESEMAPHORE_H__
#define DEVICESEMAPHORE_H__

#include <semaphore.h>

namespace Heart
{
    class hSemaphore {
    public:

        bool Create( hUint32 initCount, hUint32 maxCount ) {
            auto ret = sem_init(&sema_, 0, initCount);
            hcAssert(ret == 0);
            return ret == 0;
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