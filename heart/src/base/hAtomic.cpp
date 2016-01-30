/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#if defined (PLATFORM_WINDOWS)
    #include <winsock2.h>
    #include <windows.h>
    #define hMemoryBarrier() MemoryBarrier()
#elif defined (PLATFORM_LINUX)
    #define hMemoryBarrier() __sync_synchronize()
#else
    #error ("Platform not supported")
#endif
#include "base/hTypes.h"

namespace Heart {
namespace hAtomic {
#if !HEART_USE_ATOMICS_LIB
    HEART_EXPORT hUint32 HEART_API Increment( hAtomicInt& i ) {
        hcAssert( ((hUintptr_t)&i.value_ % 16) == 0 );
        return InterlockedIncrementAcquire( (volatile LONG*)&i.value_ );
    }

    HEART_EXPORT hUint32 HEART_API Decrement( hAtomicInt& i ) {
        hcAssert( ((hUintptr_t)&i.value_ % 16) == 0 );
        return InterlockedDecrementAcquire( (volatile LONG*)&i.value_ );
    }

    HEART_EXPORT hUint32 HEART_API CompareAndSwap( hAtomicInt& val, hUint32 compare, hUint32 newVal ) {
        hcAssert( ((hUintptr_t)&val.value_ % 16) == 0 );
        return InterlockedCompareExchangeAcquire( (volatile LONG*) &val.value_, newVal, compare );
    }

    HEART_EXPORT void HEART_API LWMemoryBarrier() {
        hMemoryBarrier();
    }

    HEART_EXPORT void HEART_API HWMemoryBarrier() {
        hMemoryBarrier();
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

#elif HEART_USE_ATOMICS_LIB
    HEART_EXPORT hUint32 HEART_API Increment( hAtomicInt& i ) {
        return ++i;
    }

    HEART_EXPORT hUint32 HEART_API Decrement( hAtomicInt& i ) {
        return --i;
    }

    HEART_EXPORT hUint32 HEART_API CompareAndSwap( hAtomicInt& val, hUint32 compare, hUint32 newVal ) {
        long l_compare = (long)compare;
        long l_new_val = newVal;
        return val.compare_exchange_strong(l_compare, l_new_val) ? compare : newVal;
    }

    HEART_EXPORT void HEART_API LWMemoryBarrier() {
        hMemoryBarrier();
    }

    HEART_EXPORT void HEART_API HWMemoryBarrier() {
        hMemoryBarrier();
    }

    HEART_EXPORT hUint32 HEART_API AtomicSet( hAtomicInt& i, hUint32 val ) {
        i.store(val);
        return val;
    }

    HEART_EXPORT hUint32 HEART_API AtomicGet(const hAtomicInt& i) {
        return i.load();
    }

    HEART_EXPORT hUint32 HEART_API AtomicAdd(hAtomicInt& i, hUint32 amount) {
        return i.fetch_add(amount)+amount;
    }
#else 
    #error ("Unknown platform")
#endif
}
}