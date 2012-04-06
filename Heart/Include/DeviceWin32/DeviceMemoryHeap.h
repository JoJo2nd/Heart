/********************************************************************
	created:	2009/08/23
	created:	23:8:2009   22:23
	filename: 	hMemory.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HCMEMORY_H__
#define HCMEMORY_H__

#ifdef HEART_TRACK_MEMORY_ALLOCS
    #undef HEART_TRACK_MEMORY_ALLOCS
#endif

#ifdef HEART_DEBUG

#define ALLOC_BREAK_NUM 0

#define THREAD_CHECK()	{ hcAssert( lastThreadID_ == NULL || lastThreadID_ == Heart::Device::GetCurrentThreadID() ); lastThreadID_ = Heart::Device::GetCurrentThreadID(); }
#define PRE_ACTION()	if ( useLocks_ ) lock_.Lock(); else THREAD_CHECK();
#define POST_ACTION()	if ( ALLOC_BREAK_NUM && debugAlloc_ == ALLOC_BREAK_NUM ) hcBreak; if ( useLocks_ ) lock_.Unlock()

#else

#define THREAD_CHECK()	
#define PRE_ACTION()	if ( useLocks_ ) lock_.Lock();
#define POST_ACTION()	if ( useLocks_ ) lock_.Unlock();

#endif // HEART_DEBUG

#ifdef HEART_TRACK_MEMORY_ALLOCS

#define TRACK_ALLOC( ptr, file, line, size, allocnum ) ++debugAlloc_; if ( pTrackingHeap_ ) (*trackedAllocs_)[ ptr ] = TrackingInfo( file, line, size, allocnum )
#define TRACK_ALLOC_UNKNOWN( ptr, size, allocnum ) ++debugAlloc_; if ( pTrackingHeap_ ) (*trackedAllocs_)[ ptr ] = TrackingInfo( size, allocnum )
#define RELEASE_TRACK_INFO( ptr, size ) if ( pTrackingHeap_ && ptr ) { TackingMapType::iterator i = (*trackedAllocs_).find( ptr );  hcAssert( i->second.size_ == size ); (*trackedAllocs_).erase( i ); }

#else

#define TRACK_ALLOC( ptr, file, line, size, allocnum )
#define TRACK_ALLOC_UNKNOWN( ptr, size, allocnum )
#define RELEASE_TRACK_INFO( ptr, size ) 

#endif // HEART_TRACK_MEMORY_ALLOCS

#define UNKNOWN_FILE "UNKNOWN_FILE"
#define UNKNOWN_LINE (0)

#include <malloc.h>
#include <new>
#include <map>
#include "hDebugMacros.h"
#include "DeviceMalloc.h"
#include <xmemory>

namespace Heart
{

struct TrackingInfo
{
	TrackingInfo() :
		file_( NULL ), line_( 0 ), size_( 0 ), allocNum_( 0 )
	{
	}
	TrackingInfo( size_t size, size_t n ) :
		file_( UNKNOWN_FILE ), line_( UNKNOWN_LINE ), size_( size ), allocNum_( n )
	{
	}
	TrackingInfo( const char* file, size_t line, size_t size, size_t n ) :
		file_( file ), line_( line ), size_( size ), allocNum_( n )
	{
	}

	const char* file_;
	size_t		line_;
	size_t		size_;
	size_t		allocNum_;
};

// class hMemoryHeap;
// extern hMemoryHeap hDebugHeap;
// template< class _Ty, hMemoryHeap* pHeap >
// class HeapAllocator;
// typedef std::map< void*, TrackingInfo, std::less< void* >, HeapAllocator< std::pair< void*, TrackingInfo >, &hDebugHeap > > TackingMapType;

class hMemoryHeap
{
public:

	struct HeapInfo
	{
		size_t			peakBytesReserved_;
		size_t			currBytesReserved_;
		size_t			totalBytesAllocated_;
	};

	hMemoryHeap( DWORD sizeInBytes, hBool threadLocal );

	~hMemoryHeap();

	void		destroy();
	void*		alloc( DWORD size );
	void*		alloc( DWORD size, const char* file, size_t line );
	void*		realloc( void* ptr, DWORD size );
	void*		realloc( void* ptr, DWORD size, const char* file, size_t line );
	void*		alignAlloc( DWORD size, DWORD alignment );
	void*		alignAlloc( DWORD size, DWORD alignment, const char* file, size_t line );
	void		release( void* ptr );
	HeapInfo	usage();
	mallinfo	info();
    hUint32     bytesAllocated() const;
	hBool		pointerBelongsToMe( void* ptr );

	static const hUint32	MAX_HEAPS = 16;
	static hUint32			nHeapsInUse_;
	static hMemoryHeap*	    pHeaps_[ MAX_HEAPS ];

private:

	hMemoryHeap( const hMemoryHeap& c );

	mspace									localMspace_;
	DWORD									size_;//ZERO when localMspace is destoryed 
	DWORD									alloced_;
	hBool									useLocks_;
	hdW32Mutex							    lock_;
	void*									lastThreadID_;
	hMemoryHeap*							pTrackingHeap_;
	//TackingMapType*							trackedAllocs_;
	size_t									allocNum_;
	size_t									debugAlloc_;

};

}

// allocator TEMPLATE OPERATORS
// template<class _Ty, class _Other> inline
// bool operator==(const hMemoryHeap<_Ty,hMemoryHeap*>&, const hMemoryHeap<_Ty,hMemoryHeap*>&) _THROW0()
// {	// test for allocator equality (always true)
// 	return (true);
// }
//  
// template<class _Ty, class _Other> inline
// bool operator!=(const hMemoryHeap<_Ty,hMemoryHeap*>&, const hMemoryHeap<_Ty,hMemoryHeap*>&) _THROW0()
// {	// test for allocator inequality (always false)
// 	return (false);
// }



#endif // HCMEMORY_H__
