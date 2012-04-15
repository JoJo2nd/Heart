/********************************************************************
	created:	2010/06/12
	created:	12:6:2010   21:22
	filename: 	DeviceThread.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef THREAD_H__
#define THREAD_H__

#include "hTypes.h"
#include "huFunctor.h"

namespace Heart
{
namespace Device
{
	inline void* GetCurrentThreadID() { return (void*)GetCurrentThreadId(); }
	inline void  ThreadSleep( DWORD dwMilliseconds ) { Sleep( dwMilliseconds ); }
	inline void	 ThreadYield() { SwitchToThread(); }

	class Thread
	{
	public:
		typedef huFunctor< hUint32(*)(void*) >::type			ThreadFunc;

		/**
		* Create 
		*
		* @param 	ThreadFunc pFunctor
		* @param 	void * param
		* @return   void
		*/
		void			Create( const hChar* threadName, hInt32 priority, ThreadFunc pFunctor, void* param )
		{
			memcpy( threadName_, threadName, THREAD_NAME_SIZE );
			threadFunc_ = pFunctor;
			pThreadParam_ = param;
			priority_ = priority;
			if ( priority_ < -2 )
			{
				priority_ = -2;
			}
			if ( priority_ > 2 )
			{
				priority_ = 2;
			}
			ThreadHand_ = CreateThread( NULL, (1024*1024)*2, staticFunc, this, 0, NULL );
		}
		/**
		* HasFinished 
		*
		* @return   hBool
		*/
		hBool			HasFinished()
		{
			DWORD exitCode;
			GetExitCodeThread( ThreadHand_, &exitCode );
			return exitCode != STILL_ACTIVE;
		}
		/**
		* ReturnCode 
		*
		* @return   hUint32
		*/
		hUint32			ReturnCode()
		{
			return returnCode_;
		}

        void            Join()
        {
            WaitForSingleObject(ThreadHand_, INFINITE);
        }

	private:

		static const int THREAD_NAME_SIZE = 32;

#pragma pack ( push,8 )
		typedef struct tagTHREADNAME_INFO
		{
			DWORD dwType; // must be 0x1000
			LPCSTR szName; // pointer to name (in user addr space)
			DWORD dwThreadID; // thread ID (-1=caller thread)
			DWORD dwFlags; // reserved for future use, must be zero
		} THREADNAME_INFO;
#pragma pack ( pop )

		static void SetThreadName( LPCSTR szThreadName )
		{
			THREADNAME_INFO info;
			info.dwType = 0x1000;
			info.szName = szThreadName;
			info.dwThreadID = -1;//caller thread
			info.dwFlags = 0;

			__try
			{
				RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
			}
			__except(EXCEPTION_CONTINUE_EXECUTION)
			{
			}
		}

		static unsigned long WINAPI staticFunc( LPVOID pParam )
		{
			Thread* pThis_ = (Thread*)pParam;
			SetThreadName( pThis_->threadName_ );
			SetThreadPriority( pThis_->ThreadHand_, pThis_->priority_ );
			pThis_->returnCode_ = pThis_->threadFunc_( pThis_->pThreadParam_ );
			return pThis_->returnCode_;
		}


		hChar							threadName_[ THREAD_NAME_SIZE ];
		Device::Thread::ThreadFunc		threadFunc_;
		void*							pThreadParam_;	
		HANDLE							ThreadHand_;
		hInt32							priority_;
		hUint32							returnCode_;
	};
}
}
#endif // THREAD_H__