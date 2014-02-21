/********************************************************************

    filename: 	hMemoryHeap.h	
    
    Copyright (c) 6:7:2012 James Moran
    
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

#ifndef HCMEMORYHEAPBASE_H__
#define HCMEMORYHEAPBASE_H__

/*
    mspace is an opaque type representing an independent
    region of space that supports mspace_malloc, etc.
*/
typedef void* mspace;

#define hMH_ALLOC_BREAK_NUM 0 //0 = off
#define hMH_ALLOC_BREAK_HEAP (hGeneralHeap)

// #ifdef HEART_TRACK_MEMORY_ALLOCS
// #   undef HEART_TRACK_MEMORY_ALLOCS
// #endif

#define HEART_MEMORY_HEAP_CLASS private: hdMutex			lock_;

#if defined (HEART_DEBUG)
#   define hMH_THREAD_CHECK()	{ hcAssert( (lastThreadID_ == NULL || lastThreadID_ == Heart::Device::GetCurrentThreadID()) ); lastThreadID_ = Heart::Device::GetCurrentThreadID(); }
#   define hMH_PRE_ACTION()	    if ( useLocks_ ) lock_.Lock(); else hMH_THREAD_CHECK();
#   define hMH_POST_ACTION()	if ( useLocks_ ) lock_.Unlock(); else hMH_THREAD_CHECK();
#else
#   define hMH_THREAD_CHECK()	
#   define hMH_PRE_ACTION()	    if ( useLocks_ ) lock_.Lock();
#   define hMH_POST_ACTION()	if ( useLocks_ ) lock_.Unlock();
#endif // HEART_DEBUG

#ifdef HEART_TRACK_MEMORY_ALLOCS
#   define hMH_TRACK_ALLOC( ptr, file, line, size, allocnum )   Heart::hMemTracking::TrackAlloc(file, line, this, ptr, size, this->name_)
#   define hMH_TRACK_ALLOC_UNKNOWN( ptr, size, allocnum )       Heart::hMemTracking::TrackAlloc("nofile", 0, this, ptr, size, this->name_)
#   define hMH_RELEASE_TRACK_INFO( ptr, size )                  Heart::hMemTracking::TrackFree(this, ptr, this->name_)
#   define hTRACK_CUSTOM_ADDRESS_ALLOC(type, ptr)               Heart::hMemTracking::TrackAlloc(__FILE__, __LINE__, NULL, ptr, 1, type)
#   define hTRACK_CUSTOM_ADDRESS_FREE(type, ptr)                Heart::hMemTracking::TrackFree(NULL, ptr, type)
#else
#   define hMH_TRACK_ALLOC( ptr, file, line, size, allocnum )
#   define hMH_TRACK_ALLOC_UNKNOWN( ptr, size, allocnum )
#   define hMH_RELEASE_TRACK_INFO( ptr, size ) 
#   define hTRACK_CUSTOM_ADDRESS_ALLOC(type, ptr)
#   define hTRACK_CUSTOM_ADDRESS_FREE(type, ptr)
#endif // HEART_TRACK_MEMORY_ALLOCS

#define UNKNOWN_FILE "UNKNOWN_FILE"
#define UNKNOWN_LINE (0)

#include <malloc.h>
#include <new>
#include <map>
#include <xmemory>

namespace Heart
{

class HEART_DLLEXPORT hMemoryHeapBase
{
public:

    struct HeapInfoEx
    {
        hUint32 arena;    /* non-mmapped space allocated from system */
        hUint32 ordblks;  /* number of free chunks */
        hUint32 smblks;   /* always 0 */
        hUint32 hblks;    /* always 0 */
        hUint32 hblkhd;   /* space in mmapped regions */
        hUint32 usmblks;  /* maximum total allocated space */
        hUint32 fsmblks;  /* always 0 */
        hUint32 uordblks; /* total allocated space */
        hUint32 fordblks; /* total free space */
        hUint32 keepcost; /* releasable (via malloc_trim) space */      
    };

    struct HeapInfo
    {
        size_t          allocs_;
        size_t			peakBytesReserved_;
        size_t			currBytesReserved_;
        size_t			totalBytesAllocated_;
        HeapInfoEx      exData_; // May not always be valid information
    };

    hMemoryHeapBase(const hChar* heapname)
    {
        Heart::hStrCopy(name_, s_nameSize, heapname);
    }
    virtual ~hMemoryHeapBase() {};

    virtual void        create(hSize_t sizeInBytes, hBool threadLocal) = 0;
    virtual void		destroy() = 0;
    virtual void*		alloc( hSize_t size, hSize_t alignment ) = 0;
    virtual void*		alloc( hSize_t size, hSize_t alignment, const hChar* file, hSize_t line ) = 0;
    virtual void*		realloc( void* ptr, hSize_t alignment, hSize_t size ) = 0;
    virtual void*		realloc( void* ptr, hSize_t alignment, hSize_t size, const hChar* file, hSize_t line ) = 0;
    virtual void		release( void* ptr ) = 0;
    virtual HeapInfo	usage() = 0;
    virtual hSize_t      totalAllocationCount() const = 0;
    virtual hBool		pointerBelongsToMe( void* ptr ) = 0;
    const hChar*        getHeapName() const { return name_; }

protected:

    void                initBaseHeap(hBool useLocks)
    {
        useLocks_ = useLocks;
        lastThreadID_ = NULL;
        debugAlloc_ = 0;
        allocNum_ = 1;
    }

    static const hUint32  s_nameSize = 32;

    hChar               name_[32];
    hBool				useLocks_;
    void*				lastThreadID_;
    hSize_t				allocNum_;
    hSize_t				debugAlloc_;

};

// HEART_DLLEXPORT void               HEART_API SetGlobalHeap(hMemoryHeapBase*);
// HEART_DLLEXPORT hMemoryHeapBase*   HEART_API GetGlobalHeap();
// HEART_DLLEXPORT void               HEART_API SetDebugHeap(hMemoryHeapBase*);
// HEART_DLLEXPORT hMemoryHeapBase*   HEART_API GetDebugHeap();
// HEART_DLLEXPORT void               HEART_API SetLuaHeap(hMemoryHeapBase*);
// HEART_DLLEXPORT hMemoryHeapBase*   HEART_API GetLuaHeap();

}


#endif // HCMEMORY_H__
