/********************************************************************
	created:	2009/08/23
	created:	23:8:2009   22:23
	filename: 	hMemory.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HCMEMORY_H__
#define HCMEMORY_H__


#ifdef HEART_DEBUG

#define ALLOC_BREAK_NUM 0

#define THREAD_CHECK()	{ hcAssert( lastThreadID_ == NULL || lastThreadID_ == Heart::hThreading::GetCurrentThreadID() ); lastThreadID_ = Heart::hThreading::GetCurrentThreadID(); }
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
#include "hMalloc.h"
#include "hMutex.h"
#include "hThread.h"
#include <xmemory>


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

class hMemoryHeap;
extern hMemoryHeap hDebugHeap;
template< class _Ty, hMemoryHeap* pHeap >
class HeapAllocator;
typedef std::map< void*, TrackingInfo, std::less< void* >, HeapAllocator< std::pair< void*, TrackingInfo >, &hDebugHeap > > TackingMapType;

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

	HeapInfo	usage()
	{
		HeapInfo info;
		PRE_ACTION();
		mspace_malloc_stats( localMspace_, &info.peakBytesReserved_, &info.currBytesReserved_, &info.totalBytesAllocated_ );

		POST_ACTION();
		return info;
	}

	mallinfo	info()
	{
		PRE_ACTION();
		mallinfo m = mspace_mallinfo( localMspace_ );
		POST_ACTION();
		return m;
	}

	DWORD		BytesAllocated() const;
	
	hBool		pointerBelongsToMe( void* ptr )
	{
		PRE_ACTION();
		hBool r = mspace_valid_pointer( localMspace_, ptr ) == 1 ? hTrue : hFalse;
		POST_ACTION();
		return r;
	}

	static const hUint32	MAX_HEAPS = 16;
	static hUint32			nHeapsInUse_;
	static hMemoryHeap*	pHeaps_[ MAX_HEAPS ];

private:

	hMemoryHeap( const hMemoryHeap& c );

	mspace									localMspace_;
	DWORD									size_;//ZERO when localMspace is destoryed 
	DWORD									alloced_;
	hBool									useLocks_;
	Heart::hMutex							lock_;
	void*									lastThreadID_;
	hMemoryHeap*							pTrackingHeap_;
	TackingMapType*							trackedAllocs_;
	size_t									allocNum_;
	size_t									debugAlloc_;

};

#undef PRE_ACTION
#undef POST_ACTION

template< class _Ty, hMemoryHeap* pHeap >
class HeapAllocator : public std::allocator< _Ty >
{	// generic allocator for objects of class _Ty
public:
	typedef typename _Ty			value_type;
	typedef value_type				*pointer;
	typedef value_type				& reference;
	typedef const value_type		*const_pointer;
	typedef const value_type		& const_reference;

	typedef size_t		size_type;
	typedef ptrdiff_t	difference_type;

	template< class _Other >
	struct rebind
	{	// convert an allocator<_Ty> to an allocator <_Other>
		typedef HeapAllocator< _Other, pHeap > other;
	};

	pointer address(reference _Val) const
	{	// return address of mutable _Val
		return (&_Val);
	}

	const_pointer address(const_reference _Val) const
	{	// return address of nonmutable _Val
		return (&_Val);
	}

	HeapAllocator() _THROW0()
	{	// construct default allocator (do nothing)
	}

	HeapAllocator(const HeapAllocator<_Ty,pHeap>&) _THROW0()
	{	// construct by copying (do nothing)
	}

	template<class _Other>
	HeapAllocator(const HeapAllocator<_Other,pHeap>&) _THROW0()
	{	// construct from a related allocator (do nothing)
	}

	template<class _Other>
	HeapAllocator<_Ty,pHeap>& operator=(const HeapAllocator<_Other,pHeap>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	void deallocate(pointer _Ptr, size_type)
	{	// deallocate object at _Ptr, ignore size
		pHeap->release( _Ptr );
	}

	pointer allocate(size_type _Count)
	{	// allocate array of _Count elements
		return (pointer)pHeap->alignAlloc( sizeof( value_type ) * _Count, 16 );
	}

	pointer allocate(size_type _Count, const void *)
	{	// allocate array of _Count elements, ignore hint
		return (allocate(_Count));
	}

	void construct(pointer _Ptr, const _Ty& _Val)
	{	// construct object at _Ptr with value _Val
		new (_Ptr) value_type(_Val);
	}

	void destroy(pointer _Ptr)
	{	// destroy object at _Ptr
		_Ptr->~value_type();
	}

	size_t max_size() const _THROW0()
	{	// estimate maximum array size
		size_t _Count = (size_t)(-1) / sizeof (_Ty);
		return (0 < _Count ? _Count : 1);
	}
};

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

/************************************************************************/
/* 
	Engine Memory Heap
*/
/************************************************************************/
extern hMemoryHeap hRendererHeap;
extern hMemoryHeap hResourceHeap;
extern hMemoryHeap hSceneGraphHeap;
extern hMemoryHeap hGeneralHeap;
extern hMemoryHeap hVMHeap;

#ifdef HEART_DEBUG
    #define hAlignMalloc( s, a ) hGeneralHeap.alignAlloc( s, a, __FILE__, __LINE__ )
	#define hMalloc( s ) hAlignMalloc( s, 16 )
	#define hRealloc( p, s ) hGeneralHeap.realloc( p, s, __FILE__, __LINE__ )
    #define hHeapAlignMalloc( h, s, a )  h.alignAlloc( s, a, __FILE__, __LINE__ )
	#define hHeapMalloc( h, s ) h.alignAlloc( s, 16, __FILE__, __LINE__ )
	#define hHeapRealloc( h, p, s ) h.realloc( p, s, __FILE__, __LINE__ )
	#define hFree( p ) hGeneralHeap.release( p )
#else
    #define hAlignMalloc( s, a ) hGeneralHeap.alignAlloc( s, a )
	#define hMalloc( s ) hAlignMalloc( s, 16 )
	#define hRealloc( p, s ) hGeneralHeap.realloc( p, s )
    #define hHeapAlignMalloc( h, s, a )  h.alignAlloc( s, a )
	#define hHeapMalloc( h, s ) h.alignAlloc( s, 16 )
	#define hHeapRealloc( h, p, s ) h.realloc( p, s )
	#define hFree( p ) hGeneralHeap.release( p )
#endif

#ifdef HEART_OVERRIDE_NEW

inline void* operator new ( size_t size )
{
	return hMalloc( size );
}

inline void* operator new[] ( size_t size )
{
	return hMalloc( size );
}

#else

inline void* operator new ( size_t size )
{
    return malloc( size );
}

inline void* operator new[] ( size_t size )
{
    return malloc( size );
}

#endif // HEART_OVERRIDE_NEW

inline void* operator new ( size_t size, hMemoryHeap& heap )
{
	return heap.alignAlloc( size, 16 );
}

inline void* operator new ( size_t size, hMemoryHeap& heap, const char* filename, size_t line )
{
	return heap.alignAlloc( size, 16, filename, line );
}

inline void* operator new[] ( size_t size, hMemoryHeap& heap )
{
	return heap.alignAlloc( size, 16 );
}

inline void* operator new[] ( size_t size, hMemoryHeap& heap, const char* filename, size_t line )
{
	return heap.alignAlloc( size, 16, filename, line );
}

inline void operator delete ( void* mem, hMemoryHeap& heap )
{
	heap.release( mem );
}

inline void operator delete[] ( void* mem, hMemoryHeap& heap )
{
	heap.release( mem );
}

inline void operator delete ( void* mem, hMemoryHeap& heap, const char* /*filename*/, size_t /*line*/ )
{
	heap.release( mem );
}

inline void operator delete[] ( void* mem, hMemoryHeap& heap, const char* /*filename*/, size_t /*line*/ )
{
	heap.release( mem );
}

inline void operator delete ( void* mem )
{
	if ( mem == NULL )
		return;

	for ( hUint32 i = 0; i < hMemoryHeap::nHeapsInUse_; ++i )
	{
		if ( hMemoryHeap::pHeaps_[ i ]->pointerBelongsToMe( mem ) )
		{
			hMemoryHeap::pHeaps_[ i ]->release( mem );
			return;
		}
	}
#ifdef HEART_OVERRIDE_NEW
	hcBreak;//couldn't find the heap
#else
    free( mem );
#endif
}

inline void operator delete[] ( void* mem )
{
	if ( mem == NULL )
		return;

	for ( hUint32 i = 0; i < hMemoryHeap::nHeapsInUse_; ++i )
	{
		if ( hMemoryHeap::pHeaps_[ i ]->pointerBelongsToMe( mem ) )
		{
			hMemoryHeap::pHeaps_[ i ]->release( mem );
			return;
		}
	}
#ifdef HEART_OVERRIDE_NEW
    hcBreak;//couldn't find the heap
#else
    free( mem );
#endif
}

#endif // HCMEMORY_H__
