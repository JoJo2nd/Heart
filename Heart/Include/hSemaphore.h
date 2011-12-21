/********************************************************************
	created:	2009/09/23
	created:	23:9:2009   21:19
	filename: 	hcSemaphore.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HCSEMAPHORE_H__
#define HCSEMAPHORE_H__

#include "hTypes.h"
#include "DeviceSemaphore.h"

namespace Heart
{
	class hSemaphore : public Device::Semaphore
	{
	public:
		
		hSemaphore() {}
		~hSemaphore() 
		{
			Device::Semaphore::Destroy();
		}
		void Create( hUint32 initCount, hUint32 maxCount )
		{
			Device::Semaphore::Create( initCount, maxCount );
		}
		void Wait()
		{
			Device::Semaphore::Wait();
		}
		void Post()
		{
			Device::Semaphore::Post();
		}
	private:

		hSemaphore( const hSemaphore& c )
		{
			//non copyable 
		}

	};
}

#endif // HCSEMAPHORE_H__