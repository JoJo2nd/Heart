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

#define hPLACEMENT_NEW(ptr)                 ::new ((void*)ptr) 

namespace Heart
{
    class hMemoryHeapBase;

    hMemoryHeapBase* hFindMemoryHeapByName(const hChar* heapName);
    hMemoryHeapBase* hFindMemoryHeapByPtr(void* ptr);
    void hGlobalMemoryFree(void* ptr);
}

#ifndef HEART_USE_DEFAULT_MEM_ALLOC

#   ifdef HEART_DEBUG
#      define hAlignMalloc( s, a ) Heart::hFindMemoryHeapByName("general")->alloc( s, a, __FILE__, __LINE__ )
#      define hMalloc( s ) hAlignMalloc( s, 16 )
#      define hRealloc( p, s ) Heart::hFindMemoryHeapByName("general")->realloc( p, 16, s, __FILE__, __LINE__ )
#      define hAlignRealloc( p, a, s ) Heart::hFindMemoryHeapByName("general")->realloc( p, a, s, __FILE__, __LINE__ )
#      define hFree( p ) Heart::hGlobalMemoryFree( p )
#      define hFreeSafe( p ) Heart::hGlobalMemoryFree( p ); p=hNullptr
#      define hHeapAlignMalloc( h, s, a )  Heart::hFindMemoryHeapByName(h)->alloc( s, a, __FILE__, __LINE__ )
#      define hHeapMalloc( h, s ) Heart::hFindMemoryHeapByName(h)->alloc( s, 16, __FILE__, __LINE__ )
#      define hHeapRealloc( h, p, s ) Heart::hFindMemoryHeapByName(h)->realloc( p, 16, s, __FILE__, __LINE__ )
#      define hHeapAlignRealloc( h, p, a, s ) Heart::hFindMemoryHeapByName(h)->realloc( p, a, s, __FILE__, __LINE__ )
#   else
#       define hAlignMalloc( s, a ) Heart::hFindMemoryHeapByName("general")->alloc( s, a )
#       define hMalloc( s ) hAlignMalloc( s, 16 )
#       define hRealloc( p, s ) Heart::hFindMemoryHeapByName("general")->realloc( p, 16, s )
#       define hAlignRealloc( p, a, s ) Heart::hFindMemoryHeapByName("general")->realloc( p, a, s )
#       define hFree( p ) Heart::hGlobalMemoryFree( p )
#       define hFreeSafe( p ) Heart::hGlobalMemoryFree( p ); p = NULL
#       define hHeapAlignMalloc( h, s, a )  Heart::hFindMemoryHeapByName(h)->alloc( s, a )
#       define hHeapMalloc( h, s ) Heart::hFindMemoryHeapByName(h)->alloc( s, 16 )
#       define hHeapRealloc( h, p, s ) Heart::hFindMemoryHeapByName(h)->realloc( p, 16, s )
#       define hHeapAlignRealloc( h, p, a, s ) Heart::hFindMemoryHeapByName(h)->realloc( p, a, s )
#       define hHeapFree( h, p ) Heart::hGlobalMemoryFree( p )
#       define hHeapFreeSafe( h, p ) Heart::hGlobalMemoryFree( p ); p = NULL
#   endif

void* operator new (size_t size, const hChar* file, hSizeT line);
void* operator new[] (size_t size, const hChar* file, hSizeT line);
void operator delete (void* mem, const hChar*, hSizeT);
void operator delete[] (void* mem, const hChar*, hSizeT);

    #define hNEW(type)                    new(__FILE__, __LINE__) type
    #define hNEW_ARRAY(type, ele)         new(__FILE__, __LINE__) type [ele]
    #define hDELETE(ptr)                  delete ptr
    #define hDELETE_ALIGNED(ptr)          delete ptr
    #define hDELETE_SAFE(ptr)             delete ptr; ptr = hNullptr
    #define hDELETE_ARRAY(ptr)            delete [] ptr
    #define hDELETE_ARRAY_SAFE(ptr)       delete [] ptr; ptr = hNullptr

#else//HEART_USE_DEFAULT_MEM

#   define hAlignMalloc( s, a ) _aligned_malloc( s, a )
#   define hAlignRealloc( p, a, s ) _aligned_realloc( p, s, a )
#   define hMalloc( s ) hAlignMalloc( s, 16 )
#   define hRealloc( p, s ) _aligned_realloc( p, s, 16 )
#   define hFree( p ) _aligned_free( p )
#   define hFreeSafe( p ) _aligned_free( p ); p = NULL
#   define hHeapAlignMalloc( h, s, a )  hAlignMalloc( s, a )
#   define hHeapMalloc( h, s ) hMalloc( s )
#   define hHeapRealloc( h, p, s ) hRealloc( p, s )
#   define hHeapFree( h, p ) hFree( p )
#   define hHeapFreeSafe( h, p ) hFreeSafe( p ); p = NULL

#   define hNEW(type)                    new type
#   define hNEW_ARRAY(type, ele)         new type [ele]
#   define hDELETE(ptr)                  delete ptr
#   define hDELETE_ALIGNED(ptr)          hFree(ptr)
#   define hDELETE_ARRAY(ptr)            delete[] ptr
#   define hDELETE_SAFE(ptr)             delete ptr; ptr = hNullptr
#   define hDELETE_ARRAY_SAFE(ptr)       delete[] ptr; ptr = hNullptr

#endif

#endif // HMEMORYDEFINES_H__