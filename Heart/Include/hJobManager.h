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
#include "hSemaphore.h"

namespace Heart
{

#define HEART_MAX_INPUT_BUFFERS  (16)
#define HEART_MAX_OUTPUT_BUFFERS (16)
#define HEART_MAX_TAG_IDS        (32)

    struct hJobBuffer
    {
        void*   ptr_;
        hUint32 size_;
    };

    struct hJobTagBarrier
    {
        hUint32     jobsToComplete_;
        hUint32     jobsCompleted_;
        hSemaphore* semaphore_;
    };

    struct hJobDesc
    {
        void*       scratchBuffer_;
        hUint32     scratchSize_;
        hUint32     inputBufferCount_;
        hJobBuffer  inputBuffes_[HEART_MAX_INPUT_BUFFERS];
        hUint32     outputBufferCount_;
        hJobBuffer  outputBuffers_[HEART_MAX_OUTPUT_BUFFERS];
    };

    typedef void (*hJobFunctionEntry)();

	class hJob
	{
    public:
		hJob() {}
        ~hJob() {}

        void                SetEntryPoint( hJobFunctionEntry entry ) { entry_ = entry; }
        void                AppendInputBuffer( void* ptr, hUint32 size );
        void                AppendOutputBuffer( void* ptr, hUint32 size );
        void                SetScratchBufferSize( hUint32 size ) { desc_.scratchSize_ = size; }
        void                SetTag( hUint32 tag ) { tag_ = tag; hcAssert( tag_ < HEART_MAX_TAG_IDS ); }
        void                SetPushed();
        void                SetTagBarrier( hJobTagBarrier* barrier ) { tagBarrier_ = barrier; }
        hBool               GetPushed() const { return pushed_; }
        hJobFunctionEntry   GetEntryPoint() const { return entry_; }
        hUint32             GetTag() const { return tag_; }
        const hJobDesc&     GetDesc() const { return desc_; }
        void                CompleteJob();

	private:

		friend class hJobManager;

        hJobFunctionEntry   entry_;
        hJobDesc            desc_;
        hUint32             tag_;
        hBool               pushed_;
        hJobTagBarrier*     tagBarrier_;

		hJob( const hJob& ) {}
        hJob& operator = ( const hJob& ) {}
	};

    class hJobChain
    {
    public:
        hJobChain( hUint32 maxJob = 32 );
        ~hJobChain();

        void PushJob( const hJobDesc& desc, hUint32 syncTag );
        void Sync() { chainSemaphore_.Wait(); }

    private:

        hSemaphore          chainSemaphore_;
        hVector< hJob >     jobs_;
        hJobTagBarrier      tagOffsets_[HEART_MAX_TAG_IDS];

        hJobChain( const hJobChain& ) {}
        hJobChain& operator = ( const hJobChain& ) {}
    };

	class hJobManager
	{
	public:
		hJobManager();
		virtual ~hJobManager();

		void		Initialise();
		void		Destory();
        void        PushJobChain( hJobChain* jobchain );
        void        WaitOnFrameJobsToFinish();

	private:

        static const hUint32 JOB_QUEUE_SIZE = 128;

        hUint32                 JobCoordinator( void* param );
        hUint32                 JobThread( void* param );
        void                    PushJobChainToCoordinator( hJobChain* chain );
        void                    PopJobChainFromCoordinator( hJobChain* chain );

        hSemaphore              jobQueueSemaphone_;
        hMutex                  jobChainMatrix_;
        hThread                 jobCoordinator_;
        hVector< hThread >      jobThreads_;
        hUint32                 pendingJobChainCount_;
        hVector< hJobChain* >   pendingJobChains_;
        hUint32                 processJobChainCount_;
        hVector< hJobChain* >   processJobChains_;
        hUint32                 jobReadIndex_;
        hUint32                 jobWriteIndex_;
        hJob                    jobQueue_[JOB_QUEUE_SIZE];
	};
}

#endif // JOBMANAGER_H__