/********************************************************************

	filename: 	hMemoryHeapBase.cpp	
	
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

#include "base/hTypes.h"
#include "base/hSysCalls.h"
#include "base/hMemory.h"
#include "base/hMemoryUtil.h"
#include <malloc.h>

namespace Heart {
    void* hMalloc(hSize_t size, hSize_t alignment/*=HEART_MIN_ALLOC_ALIGNMENT*/) {
        void* ptr = memalign(size, alignment);
        Heart::hMemTracking::TrackAlloc(""/*const hChar* tag*/, 0/*hSize_t line*/, nullptr/*void* heap*/, ptr, size, ""/*const hChar* heaptag*/);    
        return ptr;
    }
    void* hRealloc(void* mem, hSize_t size, hSize_t alignment/*=HEART_MIN_ALLOC_ALIGNMENT*/) {
        hSize_t usable = malloc_usable_size(mem);
        if (usable >= size) {
            return mem;
        }
        void* ptr = hMalloc(size, alignment);
        hMemCpy(ptr, mem, usable > size ? size : usable);
        hFree(mem);
        return ptr;
    }
    void  hFree(void* mem) {
        if (mem) {
            Heart::hMemTracking::TrackFree(nullptr, mem, "");
        }
        free(mem);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* operator new (size_t size) {
    return Heart::hMalloc(size);
}

void* operator new[] (size_t size) {
    return Heart::hMalloc(size);
}

void operator delete (void* mem) {
    Heart::hFree(mem);
}

void operator delete[] (void* mem) {
    Heart::hFree(mem);
}
