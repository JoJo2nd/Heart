/********************************************************************

	filename: 	hThread.h	
	
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

#ifndef hcThread_h__
#define hcThread_h__

namespace Heart
{
namespace hThreading
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