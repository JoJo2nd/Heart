/********************************************************************

	filename: 	hSemaphore.h	
	
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

#ifndef HCSEMAPHORE_H__
#define HCSEMAPHORE_H__

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