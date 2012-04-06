/********************************************************************

	filename: 	DeviceMutex.h	
	
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

#ifndef DEVICEMUTEX_H__
#define DEVICEMUTEX_H__

//#include <windows.h>

namespace Heart
{
	class hdW32Mutex
	{
	public:
		/**
		* Create 
		*
		* @return   hBool
		*/
		hdW32Mutex()
		{
			InitializeCriticalSection( &mutex_ );
		}
		/**
		* Lock 
		*
		* @return   void
		*/
		void Lock()
		{
			EnterCriticalSection( &mutex_ );
		}
		/**
		* TryLock 
		*
		* @return   hBool
		*/
		hBool TryLock()
		{
			BOOL ret = TryEnterCriticalSection( &mutex_ );// eh? not compiling?
			return ret == TRUE ? hTrue : hFalse;
		}
		/**
		* Unlock 
		*
		* @return   void
		*/
		void Unlock()
		{
			LeaveCriticalSection( &mutex_ );
		}
		/**
		* Destroy 
		*
		* @return   void
		*/
		~hdW32Mutex()
		{
			DeleteCriticalSection( &mutex_ );
		}
	private:
		_RTL_CRITICAL_SECTION	mutex_;
	};
}

#endif // DEVICEMUTEX_H__