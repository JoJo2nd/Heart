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

extern Heart::hMemoryHeap hDebugHeap;
extern Heart::hMemoryHeap hRendererHeap;
extern Heart::hMemoryHeap hResourceHeap;
extern Heart::hMemoryHeap hSceneGraphHeap;
extern Heart::hMemoryHeap hGeneralHeap;
extern Heart::hMemoryHeap hVMHeap;



namespace Heart
{

#define ALLOC_BREAK_NUM 0 //0 = off
#define ALLOC_BREAK_HEAP (hGeneralHeap)

hUint32 hMemoryHeap::nHeapsInUse_ = 0;
hMemoryHeap* hMemoryHeap::pHeaps_[ MAX_HEAPS ] = {0};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hMemoryHeap::hMemoryHeap( DWORD sizeInBytes, hBool threadLocal )
{
	localMspace_ = create_mspace( sizeInBytes, false );
	mspace_track_large_chunks( localMspace_, true );
	size_ = sizeInBytes;
	alloced_ = 0;
	debugAlloc_ = 0;
	allocNum_ = 1;

	hcAssert( nHeapsInUse_ < MAX_HEAPS );

	pHeaps_[ nHeapsInUse_ ] = this;
	++nHeapsInUse_;

	useLocks_ = !threadLocal;
	lastThreadID_ = NULL;
    trackedAllocs_ = NULL;

#ifdef HEART_TRACK_MEMORY_ALLOCS
	if ( this != &hDebugHeap )
	{
		trackedAllocs_ = hNEW(hDebugHeap, hTackingMapType)(&hDebugHeap);
	}
#endif // HEART_TRACK_MEMORY_ALLOCS
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hMemoryHeap::~hMemoryHeap()
{
	for ( hUint32 i = 0; i < nHeapsInUse_; ++i )
	{
		if ( pHeaps_[ i ] == this )
		{
			pHeaps_[ i ] = pHeaps_[ nHeapsInUse_ - 1 ];
			--nHeapsInUse_;
			break;
		}
	}

	destroy();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMemoryHeap::destroy()
{
	if ( size_ != 0 )
	{
		PRE_ACTION();
		DWORD r = usage().currBytesReserved_;
		DWORD f = destroy_mspace( localMspace_ );
		POST_ACTION();
		hcAssert( (r - f) == 0 );
		size_ = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::alloc( DWORD size )
{
	PRE_ACTION();
	void* r = mspace_malloc( localMspace_, size );
	size_t s = mspace_allocate_size( r );
	alloced_ += s;
	TRACK_ALLOC_UNKNOWN( r, s, allocNum_++ );
	POST_ACTION();
	return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::alloc( DWORD size, const char* file, size_t line )
{
	PRE_ACTION();
	void* r = mspace_malloc( localMspace_, size );
	size_t s = mspace_allocate_size( r );
	alloced_ += s;
	TRACK_ALLOC( r, file, line, s, allocNum_++ );
	POST_ACTION();
	return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::realloc( void* ptr, DWORD size )
{
	PRE_ACTION();
	size_t s = mspace_allocate_size(ptr);
	alloced_ -= s;
	RELEASE_TRACK_INFO( ptr, s );
	void* r = mspace_realloc( localMspace_, ptr, size );
	s = mspace_allocate_size( r );
	alloced_ += s;
	TRACK_ALLOC_UNKNOWN( r, s, allocNum_++ );
	POST_ACTION();
	return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::realloc( void* ptr, DWORD size, const char* file, size_t line )
{
	PRE_ACTION();
	size_t s = mspace_allocate_size(ptr);
	alloced_ -= s;
	RELEASE_TRACK_INFO( ptr, s );
	void* r = mspace_realloc( localMspace_, ptr, size );
	s = mspace_allocate_size( r );
	alloced_ += s;
	TRACK_ALLOC( r, file, line, s, allocNum_++ );
	POST_ACTION();
	return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::alignAlloc( DWORD size, DWORD alignment )
{
	PRE_ACTION();
	void* r = mspace_memalign( localMspace_, alignment, size );
	size_t s = mspace_allocate_size( r );
	alloced_ += s;
	TRACK_ALLOC_UNKNOWN( r, s, allocNum_++ );
	POST_ACTION();
	return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* hMemoryHeap::alignAlloc( DWORD size, DWORD alignment, const char* file, size_t line )
{
	PRE_ACTION();
	void* r = mspace_memalign( localMspace_, alignment, size );
	size_t s = mspace_allocate_size( r );
	alloced_ += s;
	TRACK_ALLOC( r, file, line, s, allocNum_++ );
	POST_ACTION();
	return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMemoryHeap::release( void* ptr )
{
	if ( ptr != NULL )
	{	
		PRE_ACTION();
        hcAssert( pointerBelongsToMe(ptr) );
		size_t s = mspace_allocate_size(ptr);
		alloced_ -= s;
		RELEASE_TRACK_INFO( ptr, s );
		mspace_free( localMspace_, ptr );
		POST_ACTION();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

hUint32 hMemoryHeap::bytesAllocated() const
{
	Heart::hAtomic::LWMemoryBarrier();
	return alloced_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hMemoryHeap::HeapInfo hMemoryHeap::usage()
{
    HeapInfo info;
    PRE_ACTION();
    mspace_malloc_stats( localMspace_, &info.peakBytesReserved_, &info.currBytesReserved_, &info.totalBytesAllocated_ );

    POST_ACTION();
    return info;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

mallinfo hMemoryHeap::info()
{
    PRE_ACTION();
    mallinfo m = mspace_mallinfo( localMspace_ );
    POST_ACTION();
    return m;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool hMemoryHeap::pointerBelongsToMe( void* ptr )
{
    PRE_ACTION();
    hBool r = mspace_valid_pointer( localMspace_, ptr ) == 1 ? hTrue : hFalse;
    POST_ACTION();
    return r;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMemoryHeap::printLeaks(const hChar* heapname)
{
    PRE_ACTION();

    if (trackedAllocs_)
    {
        hUint32 leaks = 0;
        hcPrintf("Heap %s Report Start--------------------------", heapname);
        for (hMemTrackingInfo* i = trackedAllocs_->GetHead(); i; i = i->GetNext(), ++leaks)
        {
            hcPrintf("//- Memory Leak Detected -------------------------------------------------------------");
            hcPrintf("%s(%d) : Memory block of %d bytes (Address :: 0x%08X, Alloc # %d) was leaked from here", i->file_, i->line_, i->size_, i->GetKey(), i->allocNum_);
            hcPrintf("------------------------------------------------------------------------------------//");
        }
        hcPrintf("Heap %s Report End (total leaks %d)-----------", heapname, leaks);
    }

    POST_ACTION();
}

#undef PRE_ACTION
#undef POST_ACTION
 
}