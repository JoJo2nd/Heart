/********************************************************************

	filename: 	hThread.h	
	
	Copyright (c) 7:7:2012 James Moran
	
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
#ifndef THREAD_H__
#define THREAD_H__

namespace Heart
{
namespace Device
{
	hFORCEINLINE HEARTDEV_SLIBEXPORT 
    void* HEART_API GetCurrentThreadID() { return (void*)GetCurrentThreadId(); }
	hFORCEINLINE HEARTDEV_SLIBEXPORT 
    void  HEART_API ThreadSleep( DWORD dwMilliseconds ) { Sleep( dwMilliseconds ); }
	hFORCEINLINE HEARTDEV_SLIBEXPORT 
    void  HEART_API ThreadYield() { SwitchToThread(); }
}
	class HEARTDEV_SLIBEXPORT hThread
	{
	public:
		typedef huFunctor< hUint32(*)(void*) >::type			ThreadFunc;

        enum Priority
        {
            PRIORITY_LOWEST			= -2,
            PRIORITY_BELOWNORMAL	= -1,
            PRIORITY_NORMAL			= 0,
            PRIORITY_ABOVENORMAL	= 1,
            PRIORITY_HIGH			= 2,
        };

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
		hBool			IsComplete()
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
			hThread* pThis_ = (hThread*)pParam;
			SetThreadName( pThis_->threadName_ );
			SetThreadPriority( pThis_->ThreadHand_, pThis_->priority_ );
			pThis_->returnCode_ = pThis_->threadFunc_( pThis_->pThreadParam_ );
			return pThis_->returnCode_;
		}


		hChar							threadName_[THREAD_NAME_SIZE];
		hThread::ThreadFunc		        threadFunc_;
		void*							pThreadParam_;	
		HANDLE							ThreadHand_;
		hInt32							priority_;
		hUint32							returnCode_;
	};
}
#endif // THREAD_H__