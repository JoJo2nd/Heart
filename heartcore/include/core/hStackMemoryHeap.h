/********************************************************************

	filename: 	hStackMemoryHeap.h	
	
	Copyright (c) 29:9:2012 James Moran
	
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
#pragma once

#ifndef HSTACKMEMORYHEAP_H__
#define HSTACKMEMORYHEAP_H__

namespace Heart
{
    class hStackMemoryHeap : public hMemoryHeapBase
    {
        HEART_MEMORY_HEAP_CLASS;
    public:

        hStackMemoryHeap(hMemoryHeapBase* base) 
            : hMemoryHeapBase("StackHeap")
            , baseHeap_(base)
            , basePtr_(NULL)
            , ptr_(NULL)
            , size_(0)
            , alloced_(0)
        {

        }

        ~hStackMemoryHeap()
        {

        }

        void                        create(hUint32 sizeInBytes, hBool threadLocal);
        void		                destroy();
        void*		                alloc( hUint32 size );
        void*		                alloc( hUint32 size, const hChar* file, hUint32 line );
        void*		                realloc( void* ptr, hUint32 size );
        void*		                realloc( void* ptr, hUint32 size, const hChar* file, hUint32 line );
        void*		                alignAlloc( hUint32 size, hUint32 alignment );
        void*		                alignAlloc( hUint32 size, hUint32 alignment, const hChar* file, hUint32 line );
        void		                release( void* ptr );
        hMemoryHeapBase::HeapInfo	usage();
        hUint32                     bytesAllocated() const { return alloced_; }
        hBool		                pointerBelongsToMe( void* ptr );

    private:

        hStackMemoryHeap( const hMemoryHeap& c );

        hMemoryHeapBase*    baseHeap_;
        void*               basePtr_;
        hByte*              ptr_;
        hUint32				size_;
        hUint32				alloced_;
    };
}

#endif // HSTACKMEMORYHEAP_H__