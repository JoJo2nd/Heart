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

void hMemoryHeap::create( hUint32 sizeInBytes, hBool threadLocal )
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
    hMemoryViewMenu::RegisterMemoryHeap(this);
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
        hMemoryViewMenu::UnregisterMemoryHeap(this);

		hMH_PRE_ACTION();
        mallinfo mi = mspace_mallinfo(localMspace_);
        hcAssertMsg(alloced_ == 0, "%u allocation(s) have not been released from heap %s", alloced_, getHeapName());
		DWORD r = usage().currBytesReserved_;
		DWORD f = destroy_mspace( localMspace_ );
        size_ = 0;
        localMspace_ = 0;
		hMH_POST_ACTION();
        hcAssert( (r - f) == 0 );
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::alloc( hUint32 size )
{
	hMH_PRE_ACTION();
	void* r = mspace_malloc( localMspace_, size );
	size_t s = mspace_allocate_size( r );
	++alloced_;
	hMH_TRACK_ALLOC_UNKNOWN( r, s, allocNum_++ );
	hMH_POST_ACTION();
	return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::alloc( hUint32 size, const hChar* file, hUint32 line )
{
	hMH_PRE_ACTION();
	void* r = mspace_malloc( localMspace_, size );
	size_t s = mspace_allocate_size( r );
	++alloced_;
	hMH_TRACK_ALLOC( r, file, line, s, allocNum_++ );
	hMH_POST_ACTION();
	return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::realloc( void* ptr, hUint32 size )
{
	hMH_PRE_ACTION();
	size_t s = mspace_allocate_size(ptr);
    if (ptr != NULL)
    {
        hMH_RELEASE_TRACK_INFO( ptr, s );
    }
	void* r = mspace_realloc( localMspace_, ptr, size );
	s = mspace_allocate_size( r );
    if (ptr == 0)
    {
        ++alloced_;
    }
	hMH_TRACK_ALLOC_UNKNOWN( r, s, allocNum_++ );
	hMH_POST_ACTION();
	return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::realloc( void* ptr, hUint32 size, const hChar* file, hUint32 line )
{
	hMH_PRE_ACTION();
	size_t s = mspace_allocate_size(ptr);
    if (ptr != NULL)
    {
        hMH_RELEASE_TRACK_INFO( ptr, s );
    }
	void* r = mspace_realloc( localMspace_, ptr, size );
	s = mspace_allocate_size( r );
    if (ptr == 0)
    {
        ++alloced_;
    }
	hMH_TRACK_ALLOC( r, file, line, s, allocNum_++ );
	hMH_POST_ACTION();
	return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::alignAlloc( hUint32 size, hUint32 alignment )
{
	hMH_PRE_ACTION();
	void* r = mspace_memalign( localMspace_, alignment, size );
	size_t s = mspace_allocate_size( r );
	++alloced_;
	hMH_TRACK_ALLOC_UNKNOWN( r, s, allocNum_++ );
	hMH_POST_ACTION();
	return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::alignAlloc( hUint32 size, hUint32 alignment, const hChar* file, hUint32 line )
{
	hMH_PRE_ACTION();
	void* r = mspace_memalign( localMspace_, alignment, size );
	size_t s = mspace_allocate_size( r );
	++alloced_;
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
        hcAssert( pointerBelongsToMe(ptr) );
		size_t s = mspace_allocate_size(ptr);
		--alloced_;
		hMH_RELEASE_TRACK_INFO( ptr, s );
		mspace_free( localMspace_, ptr );
		hMH_POST_ACTION();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

hUint32 hMemoryHeap::totalAllocationCount() const
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
#ifdef HEART_TRACK_MEMORY_ALLOCS
    hMH_PRE_ACTION();
    info.allocs_ = alloced_;
    mspace_malloc_stats( localMspace_, &info.peakBytesReserved_, &info.currBytesReserved_, &info.totalBytesAllocated_ );
    hMemCpy(&info.exData_, &mspace_mallinfo(localMspace_), sizeof(mallinfo));

    hMH_POST_ACTION();
#endif
    return info;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool hMemoryHeap::pointerBelongsToMe( void* ptr )
{
    hMH_PRE_ACTION();
    hBool r = mspace_valid_pointer( localMspace_, ptr ) == 1 ? hTrue : hFalse;
    hMH_POST_ACTION();
    return r;
}
 
}