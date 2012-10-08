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


namespace Heart
{

    class hMemoryHeap : public hMemoryHeapBase
    {
        HEART_MEMORY_HEAP_CLASS
    public:

        hMemoryHeap(const hChar* heapname) 
            : hMemoryHeapBase(heapname)
            , localMspace_(0)
        {}

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
        hUint32     bytesAllocated() const;
	    hBool		pointerBelongsToMe( void* ptr );
        void        printLeaks(const hChar* heapname);

    private:

	    hMemoryHeap( const hMemoryHeap& c );
        
	    mspace									localMspace_;
	    hUint32									size_;//ZERO when localMspace is destoryed 
	    hUint32									alloced_;
    };

}
#endif // HMEMORYHEAP_H__