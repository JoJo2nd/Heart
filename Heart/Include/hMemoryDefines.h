/********************************************************************

	filename: 	hMemoryDefines.h	
	
	Copyright (c) 31:3:2012 James Moran
	
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

#ifndef HMEMORYDEFINES_H__
#define HMEMORYDEFINES_H__

#include <exception>
#include <new>

namespace Heart
{

    template< typename _Ty >
    inline void hDestroyObjects(_Ty* ptr, hUint32 elements)
    {
        if ( !ptr ) return;
        for ( hUint32 i = 0; i < elements; ++i )
            ptr[i].~_Ty();
    }

}

#define hPRIVATE_DESTRUCTOR()\
    template< typename _Ty > friend void Heart::hDestroyObjects(_Ty*, hUint32);

#ifndef HEART_USE_DEFAULT_MEM_ALLOC

#ifdef HEART_DEBUG
    #define hAlignMalloc( s, a ) hGeneralHeap.alignAlloc( s, a, __FILE__, __LINE__ )
    #define hMalloc( s ) hAlignMalloc( s, 16 )
    #define hRealloc( p, s ) hGeneralHeap.realloc( p, s, __FILE__, __LINE__ )
    #define hFree( p ) hGeneralHeap.release( p )
    #define hFreeSafe( p ) hGeneralHeap.release( p ); p = NULL
    #define hHeapAlignMalloc( h, s, a )  h.alignAlloc( s, a, __FILE__, __LINE__ )
    #define hHeapMalloc( h, s ) h.alignAlloc( s, 16, __FILE__, __LINE__ )
    #define hHeapRealloc( h, p, s ) h.realloc( p, s, __FILE__, __LINE__ )
    #define hHeapFree( h, p ) h.release( p )
    #define hHeapFreeSafe( h, p ) h.release( p ); p = NULL
#else
    #define hAlignMalloc( s, a ) hGeneralHeap.alignAlloc( s, a )
    #define hMalloc( s ) hAlignMalloc( s, 16 )
    #define hRealloc( p, s ) hGeneralHeap.realloc( p, s )
    #define hFree( p ) hGeneralHeap.release( p )
    #define hFreeSafe( p ) hGeneralHeap.release( p ); p = NULL
    #define hHeapAlignMalloc( h, s, a )  h.alignAlloc( s, a )
    #define hHeapMalloc( h, s ) h.alignAlloc( s, 16 )
    #define hHeapRealloc( h, p, s ) h.realloc( p, s )
    #define hHeapFree( h, p ) h.release( p )
    #define hHeapFreeSafe( h, p ) h.release( p ); p = NULL
#endif

    #define hPLACEMENT_NEW(ptr)                 new ((void*)ptr) 
    #define hNEW(heap, type)                    hPLACEMENT_NEW(hHeapMalloc(heap,sizeof(type))) type
    #define hNEW_ALIGN(heap, align, type)       hPLACEMENT_NEW(hHeapAlignMalloc(heap,sizeof(type),align)) type
    #define hNEW_ARRAY(heap, type, ele)         hPLACEMENT_NEW(hHeapMalloc(heap,sizeof(type)*ele)) type[ele]
    #define hNEW_ARRAY_ALIGN(heap, align, type) hPLACEMENT_NEW(hHeapAlignMalloc(heap,sizeof(type)*ele,align)) type
    #define hDELETE(heap, ptr )                 hDestroyObjects(ptr,1); hHeapFree(heap,ptr);
    #define hDELETE_ARRAY(heap, ptr, ele)       hDestroyObjects(ptr,ele); hHeapFree(heap,ptr); 
    #define hDELETE_SAFE(heap, ptr)             hDestroyObjects(ptr,1); hHeapFree(heap,ptr); ptr = NULL;
    #define hDELETE_ARRAY_SAFE(heap, ptr, ele)  hDestroyObjects(ptr,ele); hHeapFree(heap,ptr); ptr = NULL;

    inline void* operator new ( size_t size )
    {
        hcBreak;
        return NULL;
    }

    inline void* operator new[] ( size_t size )
    {
        hcBreak;
        return NULL;
    }

    inline void operator delete ( void* mem )
    {
        hcBreak;
    }

    inline void operator delete[] ( void* mem )
    {
        hcBreak;
    }
    
#else//HEART_USE_DEFAULT_MEM

#define hAlignMalloc( s, a ) _aligned_malloc( s, a )
#define hMalloc( s ) hAlignMalloc( s, 16 )
#define hRealloc( p, s ) _aligned_realloc( p, s, 16 )
#define hFree( p ) _aligned_free( p )
#define hFreeSafe( p ) _aligned_free( p ); p = NULL
#define hHeapAlignMalloc( h, s, a )  hAlignMalloc( s, a )
#define hHeapMalloc( h, s ) hMalloc( s )
#define hHeapRealloc( h, p, s ) hRealloc( p, s )
#define hHeapFree( h, p ) hFree( p )
#define hHeapFreeSafe( h, p ) hFreeSafe( p ); p = NULL

    #define hPLACEMENT_NEW(ptr)                 new ((void*)ptr) 
    #define hNEW(heap, type)                    hPLACEMENT_NEW(hHeapMalloc(heap,sizeof(type))) type
    #define hNEW_ALIGN(heap, align, type)       hPLACEMENT_NEW(hHeapAlignMalloc(heap,sizeof(type),align)) type
    #define hNEW_ARRAY(heap, type, ele)         hPLACEMENT_NEW(hHeapMalloc(heap,sizeof(type)*ele)) type[ele]
    #define hNEW_ARRAY_ALIGN(heap, align, type) hPLACEMENT_NEW(hHeapAlignMalloc(heap,sizeof(type)*ele,align)) type
    #define hDELETE(heap, ptr )                 hDestroyObjects(ptr,1); hHeapFree(heap,ptr);
    #define hDELETE_ARRAY(heap, ptr, ele)       hDestroyObjects(ptr,ele); hHeapFree(heap,ptr); 
    #define hDELETE_SAFE(heap, ptr)             hDestroyObjects(ptr,1); hHeapFree(heap,ptr); ptr = NULL;
    #define hDELETE_ARRAY_SAFE(heap, ptr, ele)  hDestroyObjects(ptr,ele); hHeapFree(heap,ptr); ptr = NULL;

//     inline __declspec(nothrow) void* operator new ( size_t size )
//     {
//         return hMalloc(size);
//     }
// 
//     inline __declspec(nothrow) void* operator new[] ( size_t size )
//     {
//         return hMalloc(size);
//     }
// 
//     inline __declspec(nothrow) void operator delete ( void* mem ) 
//     {
//         hFree(mem);
//     }
// 
//     inline __declspec(nothrow) void operator delete[] ( void* mem )
//     {
//         hFree(mem);
//     }
#endif
namespace Heart
{
    class hMemoryHeap;
}
/************************************************************************/
/* 
	Engine Memory Heap
*/
/************************************************************************/
extern Heart::hMemoryHeap hRendererHeap;
extern Heart::hMemoryHeap hResourceHeap;
extern Heart::hMemoryHeap hSceneGraphHeap;
extern Heart::hMemoryHeap hGeneralHeap;
extern Heart::hMemoryHeap hVMHeap;

#endif // HMEMORYDEFINES_H__