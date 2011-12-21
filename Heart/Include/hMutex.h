/********************************************************************
	created:	2008/06/08
	created:	8:6:2008   15:43
	filename: 	hMutex.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef hcMutex_h__
#define hcMutex_h__

#include "hTypes.h"
#include "DeviceMutex.h"

namespace Heart
{
	class hMutex : public Device::Mutex
	{
	public:
		/**
		* hMutex 
		*
		* @return   
		*/
		hMutex()
		{
			Device::Mutex::Create();
		}
		/**
		* ~hMutex 
		*
		* @return   
		*/
		~hMutex()
		{
			Device::Mutex::Destroy();
		}
		/**
		* Lock 
		*
		* @return   void
		*/
		void Lock()
		{
			Device::Mutex::Lock();
		}
		/**
		* TryLock 
		*
		* @return   void
		*/
		void TryLock()
		{
			Device::Mutex::TryLock();
		}
		/**
		* Unlock 
		*
		* @return   void
		*/
		void Unlock()
		{
			Device::Mutex::Unlock();
		}
	private:
		/**
		* hMutex 
		*
		* @param 	const hMutex & c
		* @return   
		*/
		hMutex( const hMutex& c )
		{
			//non copyable 
		}
	};

}

#endif // hcMutex_h__