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
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hJobManager::hJobManager() :
		workingFlag_( hFalse )
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
		for ( hUint32 i = 0; i < MAX_JOB_THREADS; ++i )
		{
			jobThreads_[ i ].Begin(
				"hJob Task hThread",
				hThread::PRIORITY_BELOWNORMAL,
				Heart::Device::Thread::ThreadFunc::bind< hJobManager, &hJobManager::JobWorker >( this ), 
				NULL );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hJobManager::Destory()
	{
		finish_ = hTrue;

		for ( hUint32 i = 0; i < MAX_JOB_THREADS; ++i )
		{
			while ( !jobThreads_[ i ].IsComplete() ) 
			{
				Threading::ThreadYield();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hJob* hJobManager::PushJob( hJob* todo )
	{
		if ( !finish_ )
		{
			jobQueueMutex_.Lock();
			todo->AddRef();
			pendingJob_.push_back( todo );
			jobQueueMutex_.Unlock();
		}

		return todo;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hJobManager::JobWorker( void* data )
	{
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

		return 0;
	}

}