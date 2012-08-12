/********************************************************************

	filename: 	hMemoryHeap.h	
	
	Copyright (c) 7:7:2012 James Moran
	
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
#ifndef HMEMORYHEAP_H__
#define HMEMORYHEAP_H__


#ifdef HEART_TRACK_MEMORY_ALLOCS
    #undef HEART_TRACK_MEMORY_ALLOCS
#endif

#if defined (HEART_DEBUG) && defined (HEART_TRACK_MEMORY_ALLOCS)

#define THREAD_CHECK()	{ hcAssert( lastThreadID_ == NULL || lastThreadID_ == Heart::Device::GetCurrentThreadID() ); lastThreadID_ = Heart::Device::GetCurrentThreadID(); }
#define PRE_ACTION()	if ( useLocks_ ) lock_.Lock(); else THREAD_CHECK();
#define POST_ACTION()	if ( ALLOC_BREAK_NUM && debugAlloc_ == ALLOC_BREAK_NUM && this == &ALLOC_BREAK_HEAP ) hcBreak; if ( useLocks_ ) lock_.Unlock()

#else

#define THREAD_CHECK()	
#define PRE_ACTION()	if ( useLocks_ ) lock_.Lock();
#define POST_ACTION()	if ( useLocks_ ) lock_.Unlock();

#endif // HEART_DEBUG

#ifdef HEART_TRACK_MEMORY_ALLOCS

#define TRACK_ALLOC( ptr, file, line, size, allocnum ) ++debugAlloc_; if ( trackedAllocs_ ) trackedAllocs_->Insert(ptr, hNEW(hDebugHeap, hMemTrackingInfo)(file, line, size, allocnum))
#define TRACK_ALLOC_UNKNOWN( ptr, size, allocnum ) ++debugAlloc_; if ( trackedAllocs_ ) trackedAllocs_->Insert(ptr, hNEW(hDebugHeap, hMemTrackingInfo)(size, allocnum))
#define RELEASE_TRACK_INFO( ptr, size ) if ( trackedAllocs_ && ptr ) { hMemTrackingInfo* i = trackedAllocs_->Find( ptr ); hcAssertMsg(i, "Can't Find tracking info for 0x%08X", ptr); if (i) {hcAssert( i->size_ == size ); trackedAllocs_->Remove(i);} }

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
#include <xmemory>

namespace Heart
{

template< typename _Ky, typename _Ty > class hMap;
template < typename _Ky, typename _Ty > class hMapElement;

struct hMemTrackingInfo : public hMapElement< void*, hMemTrackingInfo >
{
	hMemTrackingInfo() :
		file_( NULL ), line_( 0 ), size_( 0 ), allocNum_( 0 )
	{
	}
	hMemTrackingInfo( size_t size, size_t n ) :
		file_( UNKNOWN_FILE ), line_( UNKNOWN_LINE ), size_( size ), allocNum_( n )
	{
	}
	hMemTrackingInfo( const char* file, size_t line, size_t size, size_t n ) :
		file_( file ), line_( line ), size_( size ), allocNum_( n )
	{
	}

	const char* file_;
	size_t		line_;
	size_t		size_;
	size_t		allocNum_;
};

typedef hMap< void*, hMemTrackingInfo > hTackingMapType;

class hMemoryHeap : public hMemoryHeapBase
{
public:

    hMemoryHeap() 
        : localMspace_(0)
    {};

	~hMemoryHeap();

    void        create(hUint32 sizeInBytes, hBool threadLocal);
	void		destroy();
	void*		alloc( hUint32 size );
	void*		alloc( hUint32 size, const hChar* file, hUint32 line );
	void*		realloc( void* ptr, hUint32 size );
	void*		realloc( void* ptr, hUint32 size, const hChar* file, hUint32 line );
	void*		alignAlloc( hUint32 size, hUint32 alignment );
	void*		alignAlloc( hUint32 size, hUint32 alignment, const hChar* file, hUint32 line );
	void		release( void* ptr );
    hMemoryHeapBase::HeapInfo	usage();
	//mallinfo	info();
    hUint32     bytesAllocated() const;
	hBool		pointerBelongsToMe( void* ptr );
    void        printLeaks(const hChar* heapname);

	static const hUint32	MAX_HEAPS = 16;
	static hUint32			nHeapsInUse_;
	static hMemoryHeap*	    pHeaps_[ MAX_HEAPS ];

private:

	hMemoryHeap( const hMemoryHeap& c );

	mspace									localMspace_;
	hUint32									size_;//ZERO when localMspace is destoryed 
	hUint32									alloced_;
	hBool									useLocks_;
	hdW32Mutex							    lock_;
	void*									lastThreadID_;
	hTackingMapType*					    trackedAllocs_;
	hUint32									allocNum_;
	hUint32									debugAlloc_;

};

}
#endif // HMEMORYHEAP_H__