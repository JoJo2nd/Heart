/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef JOBMANAGER_H__
#define JOBMANAGER_H__

#include "base/hTypes.h"
#include "base/hLinkedList.h"
#include "base/hArray.h"
#include "pal/hMutex.h"
#include "pal/hDeviceThread.h"
#include "pal/hConditionVariable.h"
#include "pal/hSemaphore.h"

namespace Heart
{
    class hJobQueue;

    hFUNCTOR_TYPEDEF(void (*)(void* /*inputs*/, void* /*outputs*/), hJobProc);

    class hJob : public hLinkedListElement< hJob >
    {
    public:
        hJob() {
            clear();
        }
        ~hJob() {
            clear();
        }

        void    setJobProc(hJobProc proc) { jobProc_=proc; }
        void    setInput(void* input) { inputPtr_=input; }
        void    setOutput(void* output) { outputPtr_=output; }
        void    clear() {
            jobProc_=hJobProc();
            workerMask_=(hUint16)~0;
            inputPtr_=hNullptr;
            outputPtr_=hNullptr;
            ownerQueue_=hNullptr;
        }
        void    setWorkerMask(hUint16 val) { workerMask_=val; }
        hUint16 getWorkerMask() const { return workerMask_; }

        void* operator new(size_t);
        void* operator new(size_t, const hChar*, hSize_t);
        void operator delete(void*);
        void operator delete(void*, const hChar*, hSize_t);

    private:

        friend class hJobManager;
        friend class hJobQueue;

        //new
        void runJob();

        hUint16             workerMask_;
        hJobQueue*          ownerQueue_;
        hJobProc            jobProc_;
        void*               inputPtr_;
        void*               outputPtr_;
    };

    class hJobQueue // replaces hJobChain
    {
    public:
        hJobQueue()
            : incompleteJobs_(0)
        {
        }
        ~hJobQueue()
        {
            abortJobs();
            waitForIdle();
        }

        hBool queueIdle();
        void  waitForIdle();
        void  pushJob(hJob*);
        void  abortJobs();

    private:
        HEART_PRIVATE_COPY(hJobQueue);

        friend class hJob;
        friend class hJobManager;

        void kickJobs(hLinkedList< hJob >* queue, class hJobManager* jm);
        void notifyJobCompleted();

        hMutex              incompleteJobsAccess_;
        hConditionVariable  incompleteJobsCV_;
        hUint               incompleteJobs_;
        hLinkedList< hJob > pendingJobs_;
    };

    class hJobManager
    {
    public:
        hJobManager() {}
        virtual ~hJobManager() {}

        void initialise();
        void shutdown();
        void kickQueueJobs(hJobQueue* jobqueue);
        void kickWorkers(hUint16 workermask);

        static hJob* allocateJob();
        static void  freeJob(hJob* ptr);

    private:

        struct hWorkerThread
        {
            hThread     thread_;
            hSemaphore  pendingJobsSemaphore_;
            hUint16     workerMask_;
        };

        hUint32                 jobThread( void* param );
        hJob*                   grabJob(hUint16 workermask);

        //
        hSemaphore                killSignal_;
        hMutex                    pendingJobsAccess_;
        hLinkedList< hJob >       pendingJobs_;
        hVector< hWorkerThread >  workerThreads_;
    };
}

#endif // JOBMANAGER_H__