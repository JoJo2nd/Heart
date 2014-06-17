/********************************************************************

	filename: 	hAtomic.cpp	
	
	Copyright (c) 11:7:2012 James Moran
	
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

#ifdef WIN32
    #include <winsock2.h>
    #include <windows.h>
#else
    #error ("Platform not supported")
#endif
#include "base/hTypes.h"

namespace Heart
{
namespace hAtomic
{
#ifdef WIN32

    void HEART_API Increment( hAtomicInt& i )
    {
        hcAssert( ((hUint32)&i.value_ % 16) == 0 );
        InterlockedIncrementAcquire( (volatile LONG*)&i.value_ );
    }

    void HEART_API Decrement( hAtomicInt& i )
    {
        hcAssert( ((hUint32)&i.value_ % 16) == 0 );
        InterlockedDecrementAcquire( (volatile LONG*)&i.value_ );
    }

    hUint32 HEART_API CompareAndSwap( hAtomicInt& val, hUint32 compare, hUint32 newVal )
    {
        hcAssert( ((hUint32)&val.value_ % 16) == 0 );
        return InterlockedCompareExchangeAcquire( (volatile LONG*) &val.value_, newVal, compare );
    }

    void HEART_API LWMemoryBarrier()
    {
        MemoryBarrier();
    }

    void HEART_API HWMemoryBarrier()
    {
        MemoryBarrier();
    }

    hUint32 HEART_API AtomicSet( hAtomicInt& i, hUint32 val )
    {
        /*
         * 'cos I always forget how to do this. 
         **/
        hUint32 old;
        do 
        {
            old = i.value_;
        } while (CompareAndSwap(i, old, val ) != old);

        //return the value we set
        return val;
    }

    hUint32 HEART_API AtomicAdd(hAtomicInt& i, hUint32 amount)
    {
        /*
         * 'cos I always forget how to do this. 
         **/
        hUint32 old, newv;
        do 
        {
            old = i.value_;
            newv = old+amount;
        } while (CompareAndSwap(i, old, newv ) != old);

        //return the value we set
        return newv;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 HEART_API AtomicAddWithPrev( hAtomicInt& i, hUint32 amount, hUint32* prev )
    {
        hUint32 old, newv;
        do 
        {
            old = i.value_;
            newv = old+amount;
        } while (CompareAndSwap(i, old, newv ) != old);

        //return the value we set, and the prev value of i
        *prev = old;
        return newv;
    }

#else 
    #error ("platform not supported")
#endif
}
}