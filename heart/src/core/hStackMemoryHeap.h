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

        void                        create(hSize_t sizeInBytes, hBool threadLocal);
        void		                destroy();
        void*		                alloc( hSize_t size, hSize_t alignment );
        void*		                alloc( hSize_t size, hSize_t alignment, const hChar* file, hSize_t line );
        void*		                realloc( void* ptr, hSize_t alignment, hSize_t size );
        void*		                realloc( void* ptr, hSize_t alignment, hSize_t size, const hChar* file, hSize_t line );
        void		                release( void* ptr );
        hMemoryHeapBase::HeapInfo	usage();
        hSize_t                      totalAllocationCount() const { return alloced_; }
        hBool		                pointerBelongsToMe( void* ptr );

    private:

        hStackMemoryHeap( const hMemoryHeap& c );

        hMemoryHeapBase*    baseHeap_;
        void*               basePtr_;
        hByte*              ptr_;
        hSize_t				size_;
        hSize_t				alloced_;
    };

    class hFixedPoolStackMemoryHeap : public hMemoryHeapBase
    {
        HEART_MEMORY_HEAP_CLASS;
        struct AllocHdr 
        {
            hSize_t size_;
        };
    public:

        hFixedPoolStackMemoryHeap(hSize_t sizeInBytes, void* ptr) 
            : hMemoryHeapBase("FixedPoolStackHeap")
            , alloced_(0)
            , sizeBytes_(sizeInBytes)
            , basePtr_(ptr)
        {

        }

        ~hFixedPoolStackMemoryHeap()
        {

        }

        void create(hSize_t /*sizeInBytes*/, hBool /*threadLocal*/) {}
        void destroy() {}
        void* alloc( hSize_t size, hSize_t alignment ) {
            void* ptr = ((hUint8*)basePtr_)+alloced_;
            alloced_ += size+sizeof(AllocHdr)+alignment;
            if (alloced_ > sizeBytes_) ptr = NULL;
            hcAssertMsg(ptr, "Couldn't allocate from stack");
            ptr = (void*)hAlign(ptr, alignment);
            ptr = (hUint8*)ptr-sizeof(AllocHdr);
            ((AllocHdr*)ptr)->size_ = size;
            return (void*)((hUint8*)ptr+sizeof(AllocHdr));
        }
        void* alloc( hSize_t size, hSize_t alignment, const hChar* file, hSize_t line ) {
            return alloc(size, alignment);
        }
        void*		                realloc(void* ptr, hSize_t alignment, hSize_t size)  { 
            void* newptr = alloc(size, alignment); 
            if (ptr) {
                hSize_t oldSize = ((AllocHdr*)((hUint8*)ptr-sizeof(AllocHdr)))->size_;
                hMemCpy(newptr, ptr, oldSize);
            }
            return newptr;
        }
        void* realloc( void* ptr, hSize_t alignment, hSize_t size, const hChar* /*file*/, hSize_t /*line*/ ) {
            return realloc(ptr, alignment, size); 
        }
        void		                release(void* /*ptr*/) {}
        hMemoryHeapBase::HeapInfo	usage() { 
            hMemoryHeapBase::HeapInfo info = {0};
            info.totalBytesAllocated_ = alloced_;
            return info; 
        }
        hSize_t                     totalAllocationCount() const { return alloced_; }
        hBool                      pointerBelongsToMe(void* ptr) { return (uintptr_t)ptr >= (uintptr_t)basePtr_ && (uintptr_t)ptr < (uintptr_t)basePtr_+sizeBytes_; }

    private:

        hFixedPoolStackMemoryHeap( const hMemoryHeap& c );

        hSize_t              alloced_;
        void*               basePtr_;
        hSize_t              sizeBytes_;
    };
}

#endif // HSTACKMEMORYHEAP_H__