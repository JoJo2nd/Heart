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

namespace Heart
{
    HEART_ALIGNMENT_BEGIN(16)
    struct  hAtomicInt
    {
        hUint32 value_;
    }
    HEART_ALIGNMENT_END(16);

namespace hAtomic
{

#ifdef HEART_PLAT_WINDOWS

 void HEART_API Increment( hAtomicInt& i );
 void HEART_API Decrement( hAtomicInt& i );
 hUint32 HEART_API CompareAndSwap( hAtomicInt& val, hUint32 compare, hUint32 newVal );
 hUint32 HEART_API AtomicSet(hAtomicInt& i, hUint32 val);
 hUint32 HEART_API AtomicAdd(hAtomicInt& i, hUint32 amount);
 hUint32 HEART_API AtomicAddWithPrev(hAtomicInt& i, hUint32 amount, hUint32* prev);
 void HEART_API LWMemoryBarrier();
 void HEART_API HWMemoryBarrier();

#elif defined HEART_PLAT_LINUX


#else 
    #error ("platform not supported")
#endif
}
}

#endif // ATOMIC_H__
