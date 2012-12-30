/********************************************************************

	filename: 	hStackMemoryHeap.cpp	
	
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

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hStackMemoryHeap::create( hUint32 sizeInBytes, hBool threadLocal )
    {
        hMH_PRE_ACTION();

        hcAssert(baseHeap_);

        initBaseHeap(!threadLocal);

        size_    = sizeInBytes;
        basePtr_ = baseHeap_->alloc(size_);
        ptr_     = (hByte*)basePtr_;
        alloced_ = 0;

        hMH_POST_ACTION();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hStackMemoryHeap::destroy()
    {
        hMH_PRE_ACTION();

#ifdef HEART_DEBUG
        // Trash it
        hMemSet(basePtr_, 0xF4, size_);
#endif

        baseHeap_->release(basePtr_);

        hMH_POST_ACTION();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* hStackMemoryHeap::alloc( hUint32 size )
    {
        hMH_PRE_ACTION();
        void* ret = ptr_;
        if (alloced_ + size > size_)
        {
            // Assert here?
            return NULL;
        }
        ptr_ += size;
        hMH_POST_ACTION();

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* hStackMemoryHeap::alloc( hUint32 size, const hChar* file, hUint32 line )
    {
        // Don't support tracking on stack heap.
        return alloc(size);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* hStackMemoryHeap::realloc( void* ptr, hUint32 size )
    {
        // Don't support realloc but allow to be used as alloc in case where ptr == NULL or release 
        // via size => zero
        if (ptr == NULL)
        {
            return alloc(size);
        }
        if (size == 0)
            return ptr;

        hcAssertFailMsg("Can't realloc a ptr from Stack heap (calling initially with NULL is ok however)");
        // return NULL to force any usage errors out of caller (that is no NULL pointer handling)
        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* hStackMemoryHeap::realloc( void* ptr, hUint32 size, const hChar* file, hUint32 line )
    {
        // Don't support tracking or realloc on stack heap.
        return realloc(ptr, size);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* hStackMemoryHeap::alignAlloc( hUint32 size, hUint32 alignment )
    {
        hMH_PRE_ACTION();
        hByte* ret = ptr_;
        ret = (hByte*)hAlignPower2(ret, alignment);
        if (alloced_ + size + ((hUint32)ret-(hUint32)ptr_) > size_)
        {
            // Assert here?
            return NULL;
        }
        ptr_ = ret + size;
        hMH_POST_ACTION();

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* hStackMemoryHeap::alignAlloc( hUint32 size, hUint32 alignment, const hChar* file, hUint32 line )
    {
        // Don't support tracking on stack heap.
        return alignAlloc(size, alignment);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hStackMemoryHeap::release( void* /*ptr*/ )
    {
        // Just ignore
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMemoryHeapBase::HeapInfo hStackMemoryHeap::usage()
    {
        hMH_PRE_ACTION();
        hMemoryHeapBase::HeapInfo info;
        info.currBytesReserved_ = size_;
        info.peakBytesReserved_ = size_;
        info.totalBytesAllocated_ = alloced_;
        hZeroMem(&info.exData_, sizeof(info.exData_));

        hMH_POST_ACTION();
        return info;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hStackMemoryHeap::pointerBelongsToMe( void* ptr )
    {
        return (ptr >= basePtr_ && ptr < (((hByte*)basePtr_) + size_));
    }

}