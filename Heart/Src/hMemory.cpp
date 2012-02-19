/********************************************************************
	created:	2010/02/14
	created:	14:2:2010   10:51
	filename: 	hcMemory.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hMemory.h"
#include <functional>
#include "hAtomic.h"

#ifdef HEART_DEBUG

//#define ALLOC_BREAK_NUM 137

#define THREAD_CHECK()	{ hcAssert( lastThreadID_ == NULL || lastThreadID_ == Heart::hThreading::GetCurrentThreadID() ); lastThreadID_ = Heart::hThreading::GetCurrentThreadID(); }
#define PRE_ACTION()	if ( useLocks_ ) lock_.Lock(); else THREAD_CHECK();
#define POST_ACTION()	if ( ALLOC_BREAK_NUM && debugAlloc_ == ALLOC_BREAK_NUM ) hcBreak; if ( useLocks_ ) lock_.Unlock();

#else

#define THREAD_CHECK()	
#define PRE_ACTION()	if ( useLocks_ ) lock_.Lock();
#define POST_ACTION()	if ( useLocks_ ) lock_.Unlock();

#endif // HEART_DEBUG

hUint32 hMemoryHeap::nHeapsInUse_ = 0;
hMemoryHeap* hMemoryHeap::pHeaps_[ MAX_HEAPS ] = {NULL};

extern hMemoryHeap hDebugHeap;

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
	pTrackingHeap_ = NULL;
#ifdef HEART_TRACK_MEMORY_ALLOCS
	pTrackingHeap_ = &hDebugHeap;
	if ( pTrackingHeap_ && pTrackingHeap_ != this )
	{
		trackedAllocs_ = new ( pTrackingHeap_->alloc( sizeof( TackingMapType ) ) ) TackingMapType;
	}
	else 
	{
		pTrackingHeap_ = NULL;
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

DWORD hMemoryHeap::BytesAllocated() const
{
	Heart::hAtomic::LWMemoryBarrier();
	return alloced_;
}

#undef PRE_ACTION
#undef POST_ACTION
