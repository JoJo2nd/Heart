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

#define HEART_MAX_INPUT_BUFFERS  (16)
#define HEART_MAX_OUTPUT_BUFFERS (16)
#define HEART_MAX_TAG_IDS        (32)

    struct hJobBuffer
    {
        void*   ptr_;
        hUint32 size_;
    };

    HEART_ALIGNMENT_BEGIN(32)
    struct hJobTagBatch
    {
        hUint32     jobsCompleted_;
        hUint32     jobsToComplete_;
        hUint32     firstJob_;
        hBool       batchPushed_;
    }HEART_ALIGNMENT_END(32);

    struct hJobDesc
    {
        void         AppendInputBuffer( void* ptr, hUint32 size );
        void         AppendOutputBuffer( void* ptr, hUint32 size );

        void*       scratchBuffer_;
        hUint32     scratchSize_;
        hUint32     inputBufferCount_;
        hJobBuffer  inputBuffes_[HEART_MAX_INPUT_BUFFERS];
        hUint32     outputBufferCount_;
        hJobBuffer  outputBuffers_[HEART_MAX_OUTPUT_BUFFERS];
    };

    typedef void (*hJobFunctionEntry)( const hJobDesc& desc );

	class hJob
	{
    public:
		hJob() {}
        ~hJob() {}

        void                SetEntryPoint( hJobFunctionEntry entry ) { entry_ = entry; }
        void                SetTagBarrier( hJobTagBatch* barrier ) { tagBarrier_ = barrier; }
        hJobFunctionEntry   GetEntryPoint() const { return entry_; }
        hUint32             GetTag() const { return tag_; }
        void                SetDesc( const hJobDesc& desc ) { desc_ = desc; }
        const hJobDesc&     GetDesc() const { return desc_; }
        void                CompleteJob();
        void                SetNextJobIndex( hUint32 idx ) { nextJobIndex_ = idx; }
        hUint32             GetNextJobIndex() const { return nextJobIndex_; }

	private:

		friend class hJobManager;

        hJobFunctionEntry   entry_;
        hJobDesc            desc_;
        hUint32             tag_;
        hJobTagBatch*       tagBarrier_;
        hUint32             nextJobIndex_;
	};

    class hJobChain
    {
    public:
        hJobChain( hUint32 maxJob = 32 );
        ~hJobChain();

        void                PushJob( const hJobDesc& desc, hJobFunctionEntry entry, hUint32 syncTag );
        void                Sync() { chainSemaphore_.Wait(); ClearChain(); }
        hJob*               GetJob( hUint32 idx ) { return idx == ~0U ? NULL : &jobs_[idx]; }
        hUint32             GetJobBatchCount() const;
        hJobTagBatch*       GetJobBatch();
        void                CompleteSignal() { chainSemaphore_.Post(); }
        hBool               IsFirstBatch() const { return batch_ == 0 && !jobBatches_->batchPushed_; }
        hBool               IsBatchComplete();
        void                BatchUploaded() { jobBatches_[batch_].batchPushed_ = hTrue; }
        hBool               IsComplete() const;

    private:

        void                ClearChain();

        hUint32             batch_;
        hUint32             batchCount_;
        hSemaphore          chainSemaphore_;
        hUint32             jobCount_;
        hVector< hJob >     jobs_;
        hJobTagBatch        jobBatches_[HEART_MAX_TAG_IDS];

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
        void        WaitOnJobThreadsToFinish();

	private:

        static const hUint32 JOB_QUEUE_SIZE = 128;

        hUint32                 JobCoordinator( void* param );
        hUint32                 JobThread( void* param );
        void                    PushJobChainToCoordinator( hJobChain* chain );
        void                    PopJobChainFromCoordinator( hJobChain* chain );
        void                    PopJobChainFromCoordinator( hUint32 idx );
        hJob*                   GrabJob();
        void                    WakeSleepingJobThreads();

        hSemaphore              jobQueueSemaphore_;
        hSemaphore              jobChainSemaphore_;
        hMutex                  jobChainMatrix_;
        hThread                 jobCoordinator_;
        hVector< hThread >      jobThreads_;
        hUint32                 pendingJobChainCount_;
        hVector< hJobChain* >   pendingJobChains_;
        hUint32                 processJobChainCount_;
        hVector< hJobChain* >   processJobChains_;
        //hUint32                 sleepingJobThreads_;
        HEART_ALIGN_VAR( 32, hUint32, sleepingJobThreads_ );
        //hUint32                 jobReadIndex_;
        HEART_ALIGN_VAR( 32, hUint32, jobReadIndex_ );
        //hUint32                 jobWriteIndex_;
        HEART_ALIGN_VAR( 32, hUint32, jobWriteIndex_ );
        hJob                    jobQueue_[JOB_QUEUE_SIZE];
        volatile hBool          killSignal_;
	};
}

#endif // JOBMANAGER_H__