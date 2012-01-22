/********************************************************************
	created:	2010/11/13
	created:	13:11:2010   17:48
	filename: 	hJobManager.h	
	author:		James
	
	purpose:	
*********************************************************************/

#ifndef JOBMANAGER_H__
#define JOBMANAGER_H__

#include "hTypes.h"
#include "hIReferenceCounted.h"
#include "hThread.h"
#include "hMutex.h"

namespace Heart
{
	class hJob : public hIAutoReferenceCounted
	{
	public:

		hJob() 
			: jobComplete_( hFalse )
			, aborted_( hFalse )
		{}

		hBool				IsComplete() const { return jobComplete_; }
		void				Abort() { aborted_ = hTrue; }

	protected:

		void				Finished() { jobComplete_ = hTrue; }

	private:

		friend class hJobManager;

		virtual void		JobTick() = 0;

		hBool				jobComplete_;
		hBool				aborted_;

		hJob( const hJob& /*c*/ ) {}
	};

	class hJobManager
	{
	public:
		hJobManager();
		virtual ~hJobManager();

		void		Initialise();
		void		Destory();

		hJob*		PushJob( hJob* todo );

	private:

#ifdef HEART_REWRITE_ME
		typedef deque< hJob* >	JobQueueType;
		typedef list< hJob* >	JobListType;

		static const hUint32	MAX_JOB_THREADS = 1;

		hUint32					JobWorker( void* data );

		JobQueueType			pendingJob_;
		hMutex					jobQueueMutex_;
		hThread					jobThreads_[ MAX_JOB_THREADS ];

		hBool					workingFlag_;
		volatile hBool			finish_;
#endif
	};
}

#endif // JOBMANAGER_H__