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

namespace Heart
{

class HEARTBASE_SLIBEXPORT hMemoryHeapBase
{
public:

    struct HeapInfo
    {
        size_t			peakBytesReserved_;
        size_t			currBytesReserved_;
        size_t			totalBytesAllocated_;
    };

    hMemoryHeapBase() {};
    virtual ~hMemoryHeapBase() {};

    virtual void        create(hUint32 sizeInBytes, hBool threadLocal) = 0;
    virtual void		destroy() = 0;
    virtual void*		alloc( hUint32 size ) = 0;
    virtual void*		alloc( hUint32 size, const hChar* file, hUint32 line ) = 0;
    virtual void*		realloc( void* ptr, hUint32 size ) = 0;
    virtual void*		realloc( void* ptr, hUint32 size, const hChar* file, hUint32 line ) = 0;
    virtual void*		alignAlloc( hUint32 size, hUint32 alignment ) = 0;
    virtual void*		alignAlloc( hUint32 size, hUint32 alignment, const hChar* file, hUint32 line ) = 0;
    virtual void		release( void* ptr ) = 0;
    virtual HeapInfo	usage() = 0;
    virtual hUint32     bytesAllocated() const = 0;
    virtual hBool		pointerBelongsToMe( void* ptr ) = 0;
    virtual void        printLeaks(const hChar* heapname) = 0;
};

HEARTBASE_SLIBEXPORT void               HEART_API SetGlobalHeap(hMemoryHeapBase*);
HEARTBASE_SLIBEXPORT hMemoryHeapBase*   HEART_API GetGlobalHeap();
HEARTBASE_SLIBEXPORT void               HEART_API SetDebugHeap(hMemoryHeapBase*);
HEARTBASE_SLIBEXPORT hMemoryHeapBase*   HEART_API GetDebugHeap();

}


#endif // HCMEMORY_H__
