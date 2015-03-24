/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#if defined (PLATFORM_WINDOWS)
    #include <winsock2.h>
    #include <windows.h>
#elif defined (PLATFORM_LINUX)
#else
    #error ("Platform not supported")
#endif
#include "base/hTypes.h"

namespace Heart {
namespace hAtomic {
#if defined (PLATFORM_WINDOWS)
    HEART_EXPORT hUint32 HEART_API Increment( hAtomicInt& i ) {
        hcAssert( ((hUint32)&i.value_ % 16) == 0 );
        return InterlockedIncrementAcquire( (volatile LONG*)&i.value_ );
    }

    HEART_EXPORT hUint32 HEART_API Decrement( hAtomicInt& i ) {
        hcAssert( ((hUint32)&i.value_ % 16) == 0 );
        return InterlockedDecrementAcquire( (volatile LONG*)&i.value_ );
    }

    HEART_EXPORT hUint32 HEART_API CompareAndSwap( hAtomicInt& val, hUint32 compare, hUint32 newVal ) {
        hcAssert( ((hUint32)&val.value_ % 16) == 0 );
        return InterlockedCompareExchangeAcquire( (volatile LONG*) &val.value_, newVal, compare );
    }

    HEART_EXPORT void HEART_API LWMemoryBarrier() {
        MemoryBarrier();
    }

    HEART_EXPORT void HEART_API HWMemoryBarrier() {
        MemoryBarrier();
    }

    HEART_EXPORT hUint32 HEART_API AtomicSet( hAtomicInt& i, hUint32 val ) {
        return InterlockedExchangeAcquire((volatile LONG*)&i.value_, val);
    }

    HEART_EXPORT hUint32 HEART_API AtomicGet(const hAtomicInt& i) {
        LWMemoryBarrier();
        return i.value_;
    }

    HEART_EXPORT hUint32 HEART_API AtomicAdd(hAtomicInt& i, hUint32 amount) {
        return InterlockedAddAcquire((volatile LONG*)&i.value_, amount);
    }

#elif defined (PLATFORM_LINUX)
    HEART_EXPORT hUint32 HEART_API Increment( hAtomicInt& i ) {
        return ++i;
    }

    HEART_EXPORT hUint32 HEART_API Decrement( hAtomicInt& i ) {
        return --i;
    }

    HEART_EXPORT hUint32 HEART_API CompareAndSwap( hAtomicInt& val, hUint32 compare, hUint32 newVal ) {
        return val.compare_exchange_strong(compare, newVal) ? compare : newVal;
    }

    HEART_EXPORT void HEART_API LWMemoryBarrier() {
        __sync_synchronize();
    }

    HEART_EXPORT void HEART_API HWMemoryBarrier() {
        __sync_synchronize();
    }

    HEART_EXPORT hUint32 HEART_API AtomicSet( hAtomicInt& i, hUint32 val ) {
        i.store(val);
        return val;
    }

    HEART_EXPORT hUint32 HEART_API AtomicGet(const hAtomicInt& i) {
        return i.load();
    }

    HEART_EXPORT hUint32 HEART_API AtomicAdd(hAtomicInt& i, hUint32 amount) {
        return i.fetch_add(amount);
    }
#else 
    #error ("Unknown platform")
#endif
}
}