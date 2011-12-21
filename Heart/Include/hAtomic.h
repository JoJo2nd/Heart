/********************************************************************

	filename: 	hAtomic.h
	
	Copyright (c) 2011/07/12 James Moran
	
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

#ifndef ATOMIC_H__
#define ATOMIC_H__

#ifdef WIN32

#include <windows.h>

#else

#error "Platform not defined"

#endif

namespace Heart
{
namespace hAtomic
{

#ifdef WIN32

inline void Increment( hUint32* i )
{
	InterlockedIncrementAcquire( (volatile LONG*)i );
}

inline void Decrement(hUint32* i )
{
	InterlockedDecrementAcquire( (volatile LONG*)i );
}

inline hUint32 CompareAndSwap( hUint32* val, hUint32 compare, hUint32 newVal )
{
	return InterlockedCompareExchangeAcquire( (volatile LONG*) val, newVal, compare );
}

inline void LWMemoryBarrier()
{
	MemoryBarrier();
}

inline void HWMemoryBarrier()
{
	MemoryBarrier();
}

#endif // WIN32
}
}

#endif // ATOMIC_H__