/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "base/hSysCalls.h"
#include "base/hMemory.h"
#include "memtracker.h"

namespace Heart {
    void* hMalloc(hSize_t size, hSize_t alignment/*=HEART_MIN_ALLOC_ALIGNMENT*/) {
        void* ptr = _aligned_malloc(size, alignment);
        //Heart::hMemTracking::TrackAlloc(""/*const hChar* tag*/, 0/*hSize_t line*/, nullptr/*void* heap*/, ptr, size, ""/*const hChar* heaptag*/);    
        mem_track_alloc(ptr, size, nullptr);
        return ptr;
    }
    void* hRealloc(void* mem, hSize_t size, hSize_t alignment/*=HEART_MIN_ALLOC_ALIGNMENT*/) {
        if (mem) {
            //Heart::hMemTracking::TrackFree(nullptr, mem, "");
            mem_track_free(mem);
        }
        void* ptr = _aligned_realloc(mem, size, alignment);
        if (ptr) {
            //Heart::hMemTracking::TrackAlloc(""/*const hChar* tag*/, 0/*hSize_t line*/, nullptr/*void* heap*/, ptr, size, ""/*const hChar* heaptag*/);
            mem_track_alloc(ptr, size, nullptr);
        }
        return ptr;
    }
    void  hFree(void* mem) {
        if (mem) {
            //Heart::hMemTracking::TrackFree(nullptr, mem, "");
            mem_track_free(mem);
        }
        _aligned_free(mem);
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
