/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef HMEMORYUTIL_H__
#define HMEMORYUTIL_H__

#include <memory.h>

namespace Heart
{
    hFORCEINLINE void hMemSet(void* dest, hByte val, hSize_t size) {
        memset(dest, val, size);
    }

    hFORCEINLINE void hMemCpy( void* dest, const void* src, hSize_t size ) {
        memcpy( dest, src, size );
    }

    hFORCEINLINE void hZeroMem( void* dest, hSize_t size ) {
        memset( dest, 0, size );
    }

    hFORCEINLINE void hMemMove(void* dest, const void* src, hSize_t size) {
        memmove(dest, src, size);
    }

    hFORCEINLINE int hMemCmp(const void* lhs, const void* rhs, hSize_t size) {
        return memcmp(lhs, rhs, size);
    }
}

#endif // HMEMORYUTIL_H__