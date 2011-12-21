/********************************************************************
	created:	2008/06/08
	created:	8:6:2008   15:22
	filename: 	hThread.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef hcThread_h__
#define hcThread_h__

#include "hTypes.h"
#include "DeviceThread.h"

namespace Heart
{
namespace Threading
{
	inline void*	GetCurrentThreadID() { return Device::GetCurrentThreadID(); };
	inline void		ThreadSleep( DWORD dwMilliseconds ) { Device::ThreadSleep( dwMilliseconds ); }
	inline void		ThreadYield() { Device::ThreadYield(); }
}
	class hThread : public Device::Thread
	{
	public:

		enum Priority
		{
			PRIORITY_LOWEST			= -2,
			PRIORITY_BELOWNORMAL	= -1,
			PRIORITY_NORMAL			= 0,
			PRIORITY_ABOVENORMAL	= 1,
			PRIORITY_HIGH			= 2,
		};

		/**
		* Begin 
		*
		* @param 	Device::Thread::ThreadFunc pFunctor
		* @param 	void * param
		* @return   void
		*/
		void			Begin( const hChar* threadName, Priority prio, Device::Thread::ThreadFunc pFunctor, void* param )
		{
			Device::Thread::Create( threadName, (hUint32)prio, pFunctor, param );
		}
		/**
		* IsComplete 
		*
		* @return   hBool
		*/
		hBool			IsComplete()
		{
			return Device::Thread::HasFinished();
		}
		/**
		* ReturnCode 
		*
		* @return   hUint32
		*/
		hUint32			ReturnCode()
		{
			return Device::Thread::ReturnCode();
		}

	private:
	};
}

#endif // hcThread_h__