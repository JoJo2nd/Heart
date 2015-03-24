/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include <atomic>

namespace Heart {
#if defined (PLATFORM_WINDOWS)
    HEART_ALIGNMENT_BEGIN(16)
    struct  hAtomicInt
    {
        hUint32 value_;
    }
    HEART_ALIGNMENT_END(16);
#elif defined (PLATFORM_LINUX)
    typedef std::atomic<hUint32> hAtomicInt;
    typedef std::atomic<hUint64> hAtomicInt64;
#else
#   error ("Unknown Platform")
#endif

namespace hAtomic {

HEART_EXPORT hUint32 HEART_API Increment( hAtomicInt& i );
HEART_EXPORT hUint32 HEART_API Decrement( hAtomicInt& i );
HEART_EXPORT hUint32 HEART_API CompareAndSwap( hAtomicInt& val, hUint32 compare, hUint32 newVal );
HEART_EXPORT hUint32 HEART_API AtomicSet(hAtomicInt& i, hUint32 val);
HEART_EXPORT hUint32 HEART_API AtomicGet(const hAtomicInt& i);
HEART_EXPORT hUint32 HEART_API AtomicAdd(hAtomicInt& i, hUint32 amount);
HEART_EXPORT hUint32 HEART_API AtomicAddWithPrev(hAtomicInt& i, hUint32 amount, hUint32* prev);
HEART_EXPORT void HEART_API LWMemoryBarrier();
HEART_EXPORT void HEART_API HWMemoryBarrier();

}
}
