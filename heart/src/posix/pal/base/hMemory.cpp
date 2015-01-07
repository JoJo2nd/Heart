/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "base/hSysCalls.h"
#include "base/hMemory.h"
#include "base/hMemoryUtil.h"
#include <malloc.h>

namespace Heart {
    void* hMalloc(hSize_t size, hSize_t alignment/*=HEART_MIN_ALLOC_ALIGNMENT*/) {
        void* ptr = malloc(size);//memalign(size, alignment);
        return ptr;
    }
    void* hRealloc(void* mem, hSize_t size, hSize_t alignment/*=HEART_MIN_ALLOC_ALIGNMENT*/) {
        hSize_t usable = malloc_usable_size(mem);
        if (usable >= size) {
            return mem;
        }
        void* ptr = hMalloc(size, alignment);
        hMemCpy(ptr, mem, usable > size ? size : usable);
        hFree(mem);
        return ptr;
    }
    void  hFree(void* mem) {
        free(mem);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* operator new (size_t size) {
    return Heart::hMalloc(size);
}

void* operator new[] (size_t size) {
    return Heart::hMalloc(size);
}

void operator delete (void* mem) {
    Heart::hFree(mem);
}

void operator delete[] (void* mem) {
    Heart::hFree(mem);
}
