/********************************************************************

    filename: 	hJobManager.h	
    
    Copyright (c) 1:4:2012 James Moran
    
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

#ifndef JOBMANAGER_H__
#define JOBMANAGER_H__

namespace Heart
{
    class hJobQueue;

    hFUNCTOR_TYPEDEF(void (*)(void* /*inputs*/, void* /*outputs*/), hJobProc);

    class HEART_DLLEXPORT hJob : public hLinkedListElement< hJob >
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
        void* operator new(size_t, const hChar*, hSizeT);
        void operator delete(void*);
        void operator delete(void*, const hChar*, hSizeT);

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

        void kickJobs(hLinkedList< hJob >* queue, hJobManager* jm);
        void notifyJobCompleted();

        hdMutex             incompleteJobsAccess_;
        hdConditionVariable incompleteJobsCV_;
        hUint               incompleteJobs_;
        hLinkedList< hJob > pendingJobs_;
    };

    class HEART_DLLEXPORT hJobManager
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

        static hTypePoolMemoryHeap<hJob, 1024> s_jobPool;

        struct hWorkerThread
        {
            hdThread    thread_;
            hSemaphore  pendingJobsSemaphore_;
            hUint16     workerMask_;
        };

        hUint32                 jobThread( void* param );
        hJob*                   grabJob(hUint16 workermask);

        //
        hSemaphore                killSignal_;
        hdMutex                   pendingJobsAccess_;
        hLinkedList< hJob >       pendingJobs_;
        hVector< hWorkerThread >  workerThreads_;
    };
}

#endif // JOBMANAGER_H__