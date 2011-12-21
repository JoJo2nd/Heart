/********************************************************************
	created:	2010/06/12
	created:	12:6:2010   21:55
	filename: 	DeviceMutex.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef DEVICEMUTEX_H__
#define DEVICEMUTEX_H__

//#include <windows.h>

namespace Heart
{
namespace Device
{
	class Mutex
	{
	public:
		/**
		* Create 
		*
		* @return   hBool
		*/
		hBool Create()
		{
			InitializeCriticalSection( &Mutex_ );
			return hTrue;
		}
		/**
		* Lock 
		*
		* @return   void
		*/
		void Lock()
		{
			EnterCriticalSection( &Mutex_ );
		}
		/**
		* TryLock 
		*
		* @return   hBool
		*/
		hBool TryLock()
		{
			BOOL ret = TryEnterCriticalSection( &Mutex_ );// eh? not compiling?
			return ret == TRUE ? hTrue : hFalse;
		}
		/**
		* Unlock 
		*
		* @return   void
		*/
		void Unlock()
		{
			LeaveCriticalSection( &Mutex_ );
		}
		/**
		* Destroy 
		*
		* @return   void
		*/
		void Destroy()
		{
			DeleteCriticalSection( &Mutex_ );
		}
	private:
		_RTL_CRITICAL_SECTION	Mutex_;
	};
}
}

#endif // DEVICEMUTEX_H__