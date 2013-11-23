/********************************************************************

    filename: 	DeviceMemoryHeap.cpp	
    
    Copyright (c) 31:3:2012 James Moran
    
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

namespace Heart
{

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMemoryHeap::create( hSizeT sizeInBytes, hBool threadLocal )
{
    lock_.Lock();
    if (!localMspace_)
    {
        localMspace_ = create_mspace( sizeInBytes, false );
        mspace_track_large_chunks( localMspace_, true );
        size_ = sizeInBytes;
        alloced_ = 0;

#ifdef HEART_TRACK_MEMORY_ALLOCS
        threadLocal = false;
#endif
        initBaseHeap(!threadLocal);
    }
#pragma message ("TODO- Register memory heaps with debug menu")
    //hMemoryViewMenu::RegisterMemoryHeap(this);
    lock_.Unlock();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hMemoryHeap::~hMemoryHeap()
{
    destroy();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMemoryHeap::destroy()
{
    if ( localMspace_ != 0 )
    {
#pragma message ("TODO- unregister memory heap with debug memory view menu")
        //hMemoryViewMenu::UnregisterMemoryHeap(this);

        hMH_PRE_ACTION();
        mallinfo mi = mspace_mallinfo(localMspace_);
        hcAssertMsg(alloced_ == 0, "%u allocation(s) have not been released from heap %s", alloced_, getHeapName());
        hSizeT r = usage().currBytesReserved_;
        hSizeT f = destroy_mspace( localMspace_ );
        size_ = 0;
        localMspace_ = 0;
        hMH_POST_ACTION();
        hcAssert( (r - f) == 0 );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::alloc( hSizeT size, hSizeT alignment )
{
    hMH_PRE_ACTION();
    void* r = mspace_memalign( localMspace_, alignment, size );
    hSizeT s = mspace_allocate_size( r );
    ++alloced_;
    hMH_TRACK_ALLOC_UNKNOWN( r, s, allocNum_++ );
    hMH_POST_ACTION();
    return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::alloc( hSizeT size, hSizeT alignment, const hChar* file, hSizeT line )
{
    hMH_PRE_ACTION();
    void* r = mspace_memalign( localMspace_, alignment, size );
    hSizeT s = mspace_allocate_size( r );
    ++alloced_;
    hMH_TRACK_ALLOC( r, file, line, s, allocNum_++ );
    hMH_POST_ACTION();
    return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::realloc( void* ptr, hSizeT alignment, hSizeT size )
{
    hMH_PRE_ACTION();
    size_t s = 0;
    if (ptr != NULL) {
        s=mspace_allocate_size(ptr);
        hMH_RELEASE_TRACK_INFO( ptr, s );
    }
    void* r = mspace_memalign(localMspace_, alignment, size);
    if (ptr){
        hMemCpy(r, ptr, hMin(s, size));
    }
    mspace_free(localMspace_, ptr);
    s = mspace_allocate_size( r );
    if (ptr == 0) {
        ++alloced_;
    }
    hMH_TRACK_ALLOC_UNKNOWN( r, s, allocNum_++ );
    hMH_POST_ACTION();
    return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::realloc( void* ptr, hSizeT alignment, hSizeT size, const hChar* file, hSizeT line )
{
    hMH_PRE_ACTION();
    size_t s = 0;
    if (ptr != NULL) {
        s=mspace_allocate_size(ptr);
        hMH_RELEASE_TRACK_INFO( ptr, s );
    }
    void* r = mspace_memalign(localMspace_, alignment, size);
    if (ptr){
        hMemCpy(r, ptr, hMin(s, size));
    }
    mspace_free(localMspace_, ptr);
    s = mspace_allocate_size( r );
    if (ptr == 0) {
        ++alloced_;
    }
    hMH_TRACK_ALLOC( r, file, line, s, allocNum_++ );
    hMH_POST_ACTION();
    return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMemoryHeap::release( void* ptr )
{
    if ( ptr != NULL )
    {	
        hMH_PRE_ACTION();
        //hcAssert( pointerBelongsToMe(ptr) );
        size_t s = mspace_allocate_size(ptr);
        --alloced_;
        hMH_RELEASE_TRACK_INFO( ptr, s );
        //mspace_free( localMspace_, ptr );
        dlfree( ptr );
        hMH_POST_ACTION();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

hSizeT hMemoryHeap::totalAllocationCount() const
{
    hAtomic::LWMemoryBarrier();
    return alloced_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hMemoryHeapBase::HeapInfo hMemoryHeap::usage()
{
    hMemoryHeapBase::HeapInfo info;
    hMH_PRE_ACTION();
    info.allocs_ = alloced_;
    mspace_malloc_stats( localMspace_, &info.peakBytesReserved_, &info.currBytesReserved_, &info.totalBytesAllocated_ );
    hMH_POST_ACTION();
    return info;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool hMemoryHeap::pointerBelongsToMe( void* ptr )
{
    if (!ptr) return hFalse;
    hMH_PRE_ACTION();
    hBool r = mspace_valid_pointer( localMspace_, ptr ) == 1 ? hTrue : hFalse;
    hMH_POST_ACTION();
    return r;
}
 
}