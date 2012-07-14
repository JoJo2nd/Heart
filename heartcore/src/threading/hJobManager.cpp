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
#define HEART_JOB_THREADS (4)

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hJobManager::hJobManager()
        : killSignal_(hFalse)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hJobManager::~hJobManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hJobManager::Initialise()
	{
        jobQueueSemaphore_.Create( 0, HEART_JOB_THREADS+1 );
        jobChainSemaphore_.Create( 0, 64 );
        jobReadIndex_.value_ = 0;
        jobWriteIndex_.value_= 0;
        sleepingJobThreads_.value_ = 0;

        jobCoordinator_.Create( 
            "Job Coordinator", 
            hThread::PRIORITY_HIGH, 
            hThread::ThreadFunc::bind< hJobManager, &hJobManager::JobCoordinator >( this ), 
            NULL );

        jobThreads_.Resize( HEART_JOB_THREADS );
		for ( hUint32 i = 0; i < HEART_JOB_THREADS; ++i )
		{
			jobThreads_[ i ].Create(
				"Job Task Thread",
				hThread::PRIORITY_NORMAL,
				hThread::ThreadFunc::bind< hJobManager, &hJobManager::JobThread >( this ), 
				NULL );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hJobManager::Destory()
	{
        WaitOnJobThreadsToFinish();
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::PushJobChain( hJobChain* jobchain )
    {
        hMutexAutoScope am( &jobChainMatrix_ );

        if ( pendingJobChainCount_ >= pendingJobChains_.GetSize() )
            pendingJobChains_.PushBack( jobchain );
        else
            pendingJobChains_[pendingJobChainCount_] = jobchain;

        ++pendingJobChainCount_;

        jobChainSemaphore_.Post();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::WaitOnJobThreadsToFinish()
    {
        killSignal_ = hTrue;

        jobChainSemaphore_.Post();
        WakeSleepingJobThreads();

        jobCoordinator_.Join();

        for ( hUint32 i = 0; i < HEART_JOB_THREADS; ++i )
        {
            jobThreads_[i].Join();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::PushJobChainToCoordinator( hJobChain* chain )
    {
        if ( processJobChainCount_ >= processJobChains_.GetSize())
            processJobChains_.PushBack(chain);
        else
            processJobChains_[processJobChainCount_] = chain;

        ++processJobChainCount_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::PopJobChainFromCoordinator( hJobChain* chain )
    {
        for ( hUint32 i = 0; i < processJobChainCount_; ++i )
        {
            if ( processJobChains_[i] == chain )
            {
                processJobChains_[i] = processJobChains_[--processJobChainCount_];
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::PopJobChainFromCoordinator( hUint32 idx )
    {
        hcAssert( idx < processJobChainCount_ );
        processJobChains_[idx] = processJobChains_[--processJobChainCount_];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hJobManager::JobCoordinator( void* param )
    {
        while (!killSignal_)
        {
            jobChainSemaphore_.Wait();

            jobChainMatrix_.Lock();

            //push pending job chains to process job chains
            for ( hUint32 i = 0; i < pendingJobChainCount_; ++i )
            {
                PushJobChainToCoordinator( pendingJobChains_[i] );
            }

            pendingJobChainCount_ = 0;

            jobChainMatrix_.Unlock();

            hUint32 chains = processJobChainCount_;
            for ( hUint32 i =0; i < chains; ++i )
            {
                hJobChain* jc = processJobChains_[i];
                if ( jc->IsBatchComplete() || jc->IsFirstBatch() || jc->IsComplete() )
                {
                    if ( jc->IsComplete() )
                    {
                        //The job Chain is complete. remove it and signal anyone waiting on it.
                        PopJobChainFromCoordinator( i );
                        jc->CompleteSignal();
                    }
                    else
                    {
                        hUint32 nextBatchCount = jc->GetJobBatchCount();
                        hAtomicInt writeIdx = jobWriteIndex_;
                        if ( (jobWriteIndex_.value_+nextBatchCount)%JOB_QUEUE_SIZE != jobReadIndex_.value_ )
                        {
                            hJobTagBatch* batch = jc->GetJobBatch();
                            for ( hJob* job = jc->GetJob( batch->firstJob_ ); job; job = jc->GetJob( job->GetNextJobIndex() ) )
                            {
                                jobQueue_[writeIdx.value_] = *job;
                                writeIdx.value_ = (writeIdx.value_+1)%JOB_QUEUE_SIZE;
                            }

                            hAtomic::LWMemoryBarrier();
                            jobWriteIndex_ = writeIdx;

                            jc->BatchUploaded();
                        }

                        WakeSleepingJobThreads();
                    }
                }
            }
        }

        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hJobManager::JobThread( void* param )
    {
        while (!killSignal_)
        {
            hJob* job = GrabJob();

            if ( !job )
            {
                hAtomic::Increment( sleepingJobThreads_ );
                jobQueueSemaphore_.Wait();
                hAtomic::Decrement( sleepingJobThreads_ );
                continue;
            }

            (*job->GetEntryPoint())( job->GetDesc() );
            job->CompleteJob();
        }

		return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hJob* hJobManager::GrabJob()
    {
        hUint32 val;
        hUint32 nextVal;
        do 
        {
            val = jobReadIndex_.value_;
            nextVal = (val+1) % JOB_QUEUE_SIZE;

            if ( val == jobWriteIndex_.value_ )
            {
                return NULL;
            }
        }
        while( hAtomic::CompareAndSwap( jobReadIndex_, val, nextVal ) != val );

        return &jobQueue_[val];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::WakeSleepingJobThreads()
    {
        hUint32 towake = sleepingJobThreads_.value_;
        while ( towake-- )
        {
            jobQueueSemaphore_.Post();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hJobChain::hJobChain( hUint32 maxJob /*= 32 */ )
    {
        chainSemaphore_.Create( 0, 1 );
        jobs_.Resize( maxJob );
        ClearChain();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hJobChain::~hJobChain()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobChain::PushJob( const hJobDesc& desc, hJobFunctionEntry entry, hUint32 syncTag )
    {
        hJob* job = &jobs_[jobCount_];
        job->SetEntryPoint( entry );
        job->SetDesc( desc );
        job->SetTagBarrier( &jobBatches_[syncTag] );
        job->SetNextJobIndex( jobBatches_[syncTag].firstJob_ );
        jobBatches_[syncTag].firstJob_ = jobCount_;
        ++jobBatches_[syncTag].jobsToComplete_;
        ++jobCount_;

        batchCount_ = hMax( syncTag, batchCount_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hJobChain::GetJobBatchCount() const
    {
        return jobBatches_[batch_].jobsToComplete_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hJobTagBatch* hJobChain::GetJobBatch()
    {
        return &jobBatches_[batch_];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hJobChain::IsBatchComplete()
    {
        if ( !jobBatches_[batch_].batchPushed_ )
            return hFalse;
        if ( IsComplete() || jobBatches_[batch_].jobsCompleted_.value_ == jobBatches_[batch_].jobsToComplete_ )
        {
            ++batch_;
            return hTrue;
        }
        return hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hJobChain::IsComplete() const
    {
        return batch_ > batchCount_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobChain::ClearChain()
    {
        batch_ = 0;
        batchCount_ = 0;
        jobCount_ = 0;
        for ( hUint32 i = 0; i < HEART_MAX_TAG_IDS; ++i )
        {
            jobBatches_[i].batchPushed_ = hFalse;
            jobBatches_[i].firstJob_ = ~0U;
            jobBatches_[i].jobsCompleted_.value_ = 0;
            jobBatches_[i].jobsToComplete_ = 0;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobDesc::AppendInputBuffer( void* ptr, hUint32 size )
    {
        hcAssert( inputBufferCount_ < HEART_MAX_INPUT_BUFFERS );
        inputBuffes_[inputBufferCount_].ptr_  = ptr;
        inputBuffes_[inputBufferCount_].size_ = size;
        ++inputBufferCount_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobDesc::AppendOutputBuffer( void* ptr, hUint32 size )
    {
        hcAssert( outputBufferCount_ < HEART_MAX_INPUT_BUFFERS );
        outputBuffers_[outputBufferCount_].ptr_  = ptr;
        outputBuffers_[outputBufferCount_].size_ = size;
        ++outputBufferCount_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJob::CompleteJob()
    {
        hcAssert( tagBarrier_ );
        hAtomic::Increment( tagBarrier_->jobsCompleted_ );
    }

}