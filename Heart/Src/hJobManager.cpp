/********************************************************************
	created:	2010/11/13
	created:	13:11:2010   18:00
	filename: 	hJobManager.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hJobManager.h"

namespace Heart
{
#define HEART_JOB_THREADS (4)

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hJobManager::hJobManager()
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
        jobQueueSemaphone_.Create( 0, HEART_JOB_THREADS+1 );
        jobReadIndex_ = 0;
        jobWriteIndex_= 0;

        jobCoordinator_.Begin( 
            "Job Coordinator", 
            hThread::PRIORITY_HIGH, 
            Heart::Device::Thread::ThreadFunc::bind< hJobManager, &hJobManager::JobCoordinator >( this ), 
            NULL );

        jobThreads_.Resize( HEART_JOB_THREADS );
		for ( hUint32 i = 0; i < HEART_JOB_THREADS; ++i )
		{
			jobThreads_[ i ].Begin(
				"Job Task Thread",
				hThread::PRIORITY_NORMAL,
				Heart::Device::Thread::ThreadFunc::bind< hJobManager, &hJobManager::JobThread >( this ), 
				NULL );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hJobManager::Destory()
	{
        WaitOnFrameJobsToFinish();
#ifdef HEART_REWRITE_ME
		for ( hUint32 i = 0; i < MAX_JOB_THREADS; ++i )
		{
			while ( !jobThreads_[ i ].IsComplete() ) 
			{
				Threading::ThreadYield();
			}
		}
#endif
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::PushJobChain( hJobChain* jobchain )
    {
        hMutexAutoScope am( &jobChainMatrix_ );

        pendingJobChains_.PushBack( jobchain );

        jobQueueSemaphone_.Post();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::WaitOnFrameJobsToFinish()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hJobManager::PushJobChainToCoordinator( hJobChain* chain )
    {
        processJobChains_[processJobChainCount_++] = chain;
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

    hUint32 hJobManager::JobCoordinator( void* param )
    {
        while (1)
        {
            jobQueueSemaphone_.Wait();

            jobChainMatrix_.Lock();

            //push pending job chains to process job chains
            hUint32 size = pendingJobChains_.GetSize();
            for ( hUint32 i = 0; i < size; ++i )
            {
                PushJobChainToCoordinator( pendingJobChains_[i] );
            }

            pendingJobChainCount_ = 0;

            jobChainMatrix_.Unlock();


        }

        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hJobManager::JobThread( void* param )
    {
        jobQueueSemaphone_.Wait();
/*
		list< hJob* >			runningJobs_;
		hBool					complete = hFalse;

		while ( !complete )
		{
			//size is const so should be able to get away with this without mutex Lock
			if ( pendingJob_.size() > 0 )
			{
				jobQueueMutex_.Lock();

				JobQueueType::iterator i = pendingJob_.begin(), iend = pendingJob_.end();
				for ( ; i != iend; ++i )
				{
					runningJobs_.push_back( *i );
				}

				pendingJob_.clear();
				jobQueueMutex_.Unlock();
			}

			if ( runningJobs_.size() )
			{
				JobListType::iterator i = runningJobs_.begin(), iend = runningJobs_.end();
				while ( i != iend && !finish_ )
				{
					(*i)->JobTick();
					if ( (*i)->IsComplete() || (*i)->aborted_ )
					{
						hJob* ptr = *i;
						ptr->Finished();
						i = runningJobs_.erase( i );
						ptr->DecRef();
					}
				}
			}
			else
			{
				Threading::ThreadYield();
			}

			if ( finish_ )
			{
				complete = hTrue;
			}
		}

		jobQueueMutex_.Lock();

		for ( JobQueueType::iterator i = pendingJob_.begin(), iend = pendingJob_.end(); 
			i != iend; ++i )
		{
			runningJobs_.push_back( *i );
		}

		pendingJob_.clear();
		jobQueueMutex_.Unlock();

		for( JobListType::iterator i = runningJobs_.begin(), iend = runningJobs_.end();
			i != iend; ++i )
		{
			(*i)->DecRef();
		}
*/
		return 0;
    }

}