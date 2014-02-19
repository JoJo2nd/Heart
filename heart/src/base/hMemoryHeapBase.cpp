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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static hByte g_globalMemoryPoolSpace[sizeof(Heart::hMemoryHeap)];
static hByte g_debugMemoryPoolSpace[sizeof(Heart::hMemoryHeap)];
static hByte g_luaMemoryPoolSpace[sizeof(Heart::hMemoryHeap)];

void initialiseBaseMemoryHeaps()
{
#ifdef HEART_TRACK_MEMORY_ALLOCS
    Heart::hMemTracking::InitMemTracking();
#endif

    Heart::hMemoryHeapBase* global=hPLACEMENT_NEW(g_globalMemoryPoolSpace) Heart::hMemoryHeap("GlobalHeap");
    Heart::hMemoryHeapBase* debug=hPLACEMENT_NEW(g_debugMemoryPoolSpace) Heart::hMemoryHeap("DebugHeap");
    Heart::hMemoryHeapBase* lua=hPLACEMENT_NEW(g_luaMemoryPoolSpace) Heart::hMemoryHeap("LuaHeap");

    // It important that the debug heap is created first
    global->create(1024*1024,hFalse);
    debug->create(1024*1024,hFalse);
    lua->create(1024*1024,hTrue);
}

namespace Heart
{
namespace HeapPrivate 
{
    struct HeapLookup
    {
        hUint32             stringHash_;
        hMemoryHeapBase*    heapPtr_;

        bool operator < (const HeapLookup& rhs) {
            return stringHash_ < rhs.stringHash_;
        }
        bool operator < (const hUint32& rhs) {
            return stringHash_ < rhs;
        }
    };

    static hBool g_heapInit = false;
    static HeapLookup g_heapArray[128];
    static hUint g_heapCount = 0;

    static hUint readMemoryMap() {
        static hdMutex g_initMutex;
        hMutexAutoScope mas(&g_initMutex);
        if (g_heapInit) {
            return g_heapCount;
        }

        initialiseBaseMemoryHeaps();

        g_heapArray[g_heapCount].stringHash_=hCRC32::StringCRC("general");
        g_heapArray[g_heapCount++].heapPtr_=(hMemoryHeapBase*)g_globalMemoryPoolSpace;
        g_heapArray[g_heapCount].stringHash_=hCRC32::StringCRC("lua");
        g_heapArray[g_heapCount++].heapPtr_=(hMemoryHeapBase*)g_luaMemoryPoolSpace;
        g_heapArray[g_heapCount].stringHash_=hCRC32::StringCRC("debug");
        g_heapArray[g_heapCount++].heapPtr_=(hMemoryHeapBase*)g_debugMemoryPoolSpace;

        std::sort(g_heapArray, g_heapArray+3);

        g_heapInit=true;

        return g_heapCount;
    }

    static hMemoryHeapBase* findHeapByName(const hChar* name) {
        static hThreadLocal hBool doneinit=false;
        static hThreadLocal HeapLookup heapArray_[128];
        static hThreadLocal hUint heapCount=0;
        if (!doneinit) {
            heapCount=readMemoryMap();
            hMemCpy(heapArray_, g_heapArray, sizeof(g_heapArray));
            doneinit=true;
        }

        hUint32 hash=hCRC32::StringCRC(name);
        HeapLookup* found=std::lower_bound(heapArray_, heapArray_+heapCount, hash);
        hcAssert(found->stringHash_==hash);

        return found->heapPtr_;
    }

    static hMemoryHeapBase* findHeapByPtr(void* ptr) {
        for (hUint i=0; g_heapArray[i].heapPtr_; ++i) {
            if (g_heapArray[i].heapPtr_->pointerBelongsToMe(ptr)) {
                return g_heapArray[i].heapPtr_;
            }
        }
        // This might be ok for alloc only heaps? Null ptrs follow this path
        return hNullptr;
    }
}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMemoryHeapBase* hFindMemoryHeapByName(const hChar* heapName) {
        return HeapPrivate::findHeapByName(heapName);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMemoryHeapBase* hFindMemoryHeapByPtr(void* ptr) {
        return HeapPrivate::findHeapByPtr(ptr);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGlobalMemoryFree(void* ptr) {
        if (!ptr) {
            return;
        }
        hMemoryHeapBase* h=hFindMemoryHeapByPtr(ptr);
        if (h) {
            h->release(ptr);
        }
    }
}

void* operator new (size_t size) {
    Heart::hMemoryHeapBase* heap=Heart::HeapPrivate::findHeapByName("general");
    return heap->alloc(size, HEART_MIN_ALLOC_ALIGNMENT);
}

void* operator new[] (size_t size) {
    Heart::hMemoryHeapBase* heap=Heart::HeapPrivate::findHeapByName("general");
    return heap->alloc(size, HEART_MIN_ALLOC_ALIGNMENT);
}

void* operator new (size_t size, const hChar* file, hSizeT line) {
    Heart::hMemoryHeapBase* heap=Heart::HeapPrivate::findHeapByName("general");
    return heap->alloc(size, HEART_MIN_ALLOC_ALIGNMENT, file, line);
}

void* operator new[] (size_t size, const hChar* file, hSizeT line) {
    Heart::hMemoryHeapBase* heap=Heart::HeapPrivate::findHeapByName("general");
    return heap->alloc(size, HEART_MIN_ALLOC_ALIGNMENT, file, line);
}

void operator delete (void* mem) {
    Heart::hGlobalMemoryFree(mem);
}

void operator delete[] (void* mem) {
    Heart::hGlobalMemoryFree(mem);
}

void operator delete (void* mem, const hChar*, hSizeT) {
    Heart::hGlobalMemoryFree(mem);
}

void operator delete[] (void* mem, const hChar*, hSizeT) {
    Heart::hGlobalMemoryFree(mem);
}

