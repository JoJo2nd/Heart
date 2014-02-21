/********************************************************************

    filename: 	hJobManager.cpp	
    
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

namespace Heart
{
#define HEART_JOB_THREADS (8)

    hTypePoolMemoryHeap<hJob, 1024> hJobManager::s_jobPool;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::initialise()
    {
        killSignal_.Create(0, 128);
        workerThreads_.Resize(HEART_JOB_THREADS); // need a hGetCPUCoreCount() here?
        for (hUint32 i = 0; i < workerThreads_.GetSize(); ++i)
        {
            hChar threadname[128];
            hStrPrintf(threadname, (hUint)hArraySize(threadname), "Worker %u", i);
            workerThreads_[ i ].pendingJobsSemaphore_.Create(0, 4096);
            workerThreads_[ i ].workerMask_ = 1 << i;
            workerThreads_[ i ].thread_.create(
                threadname,
                hdThread::PRIORITY_NORMAL,
                hFUNCTOR_BINDMEMBER(hThreadFunc, hJobManager, jobThread, this),
                &workerThreads_[i] );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::shutdown()
    {
        for ( hUint32 i = 0; i < workerThreads_.GetSize(); ++i ) {
            for ( hUint32 i2 = 0; i2 < workerThreads_.GetSize(); ++i2 ) {
                killSignal_.Post();
            }
            workerThreads_[ i ].pendingJobsSemaphore_.Post();
            workerThreads_[ i ].thread_.join();
        }

        pendingJobs_.clear();
        killSignal_.Destroy();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hJobManager::jobThread( void* param )
    {
        hWorkerThread* worker=(hWorkerThread*)param;
        while (!killSignal_.poll()) {
            worker->pendingJobsSemaphore_.Wait();
            hJob* job = grabJob(worker->workerMask_);
            if ( job ) {
                job->runJob();
                hDELETE(job);
            }
        }
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hJob* hJobManager::grabJob(hUint16 workermask) {
        hMutexAutoScope mas(&pendingJobsAccess_);

        for(hJob* i=pendingJobs_.begin(), *n=pendingJobs_.end(); i!=n; i=i->GetNext()) {
            if (i->getWorkerMask()&workermask) {
                pendingJobs_.remove(i);
                return i;
            }
        }

        //Did someone beat us to it?
        return hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::kickQueueJobs(hJobQueue* jobqueue) {
        hMutexAutoScope mas(&pendingJobsAccess_);

        jobqueue->kickJobs(&pendingJobs_, this);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hJob* hJobManager::allocateJob() {
        return (hJob*)hMalloc(sizeof(hJob));
        //return (hJob*)s_jobPool.alloc(sizeof(hJob), hAlignOf(hJob));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::freeJob(hJob* ptr) {
        //s_jobPool.release(ptr);
        hFree(ptr);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::kickWorkers(hUint16 workermask) {
        for (hUint32 i = 0; i < workerThreads_.GetSize(); ++i)
        {
            if (1<<i & workermask) {
                workerThreads_[ i ].pendingJobsSemaphore_.Post();
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* hJob::operator new(size_t) {
        return hJobManager::allocateJob();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* hJob::operator new(size_t, const hChar*, hSize_t) {
        return hJobManager::allocateJob();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJob::operator delete(void* ptr) {
        hJobManager::freeJob((hJob*)ptr);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJob::operator delete(void* ptr, const hChar*, hSize_t) {
        hJobManager::freeJob((hJob*)ptr);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJob::runJob() {
        if (jobProc_.isValid()) {
            jobProc_(inputPtr_, outputPtr_);
            if (ownerQueue_) {
                ownerQueue_->notifyJobCompleted();
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hJobQueue::queueIdle() {
        hBool idle;
        incompleteJobsAccess_.Lock();
        idle = incompleteJobs_==0;
        incompleteJobsAccess_.Unlock();
        return idle;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobQueue::waitForIdle() {
        incompleteJobsAccess_.Lock();

        if (incompleteJobs_!=0) {
            incompleteJobsCV_.wait(&incompleteJobsAccess_);
        }

        incompleteJobsAccess_.Unlock();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobQueue::pushJob(hJob* job) {
        pendingJobs_.addTail(job);
        job->ownerQueue_=this;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobQueue::abortJobs() {
        for (hJob* i=pendingJobs_.begin(), *n=pendingJobs_.end(); i!=n; ++i) {
            i->clear();
        }
        for (hJob* i=pendingJobs_.begin(), *n=pendingJobs_.end(); i!=n; ) {
            hJob* del=i;
            i=pendingJobs_.remove(del);
            hDELETE(del);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobQueue::kickJobs(hLinkedList< hJob >* queue, hJobManager* jm) {
        hMutexAutoScope mas(&incompleteJobsAccess_);

        for (hJob* i=pendingJobs_.begin(), *n=pendingJobs_.end(), *next; i!=n; i=next) {
            hUint16 workermask=i->getWorkerMask();
            next=pendingJobs_.remove(i);
            queue->addTail(i);
            jm->kickWorkers(workermask);
            ++incompleteJobs_;
        }

        pendingJobs_.clear();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobQueue::notifyJobCompleted() {
        hMutexAutoScope mas(&incompleteJobsAccess_);

        --incompleteJobs_;
        if (incompleteJobs_==0) {
            // wake up anyone waiting on this queue to complete
            incompleteJobsCV_.broadcast();
        }
    }

}