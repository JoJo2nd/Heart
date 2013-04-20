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

    template< typename _Ty >
    inline void hDestroyObjects(_Ty* ptr, hSizeT elements)
    {
        if (hIs_pod< _Ty >::value || !ptr ) {
            return;
        }
        for ( hSizeT i = 0; i < elements; ++i )
            ptr[i].~_Ty();
    }

    template< typename _Ty >
    inline hSizeT hCalcArrayAllocHeader()
    {
        if (hIs_pod< _Ty >::value) {
            return 0;
        }

        hSizeT align = hAlignOf(_Ty);
        return align > 4 ? align : 4;
    }

    template< typename _Ty >
    inline hSizeT hCalcArrayAllocCount(_Ty* ptr)
    {
        if (!ptr) return 0;
        return *((hUint32*)(((hByte*)ptr)-hCalcArrayAllocHeader<_Ty>()));
    }

    template< typename _Ty >
    inline _Ty* hCalcArrayAllocCorrectPointer(_Ty* ptr)
    {
        if (!ptr) return NULL;
        return (_Ty*)(((hByte*)ptr)-hCalcArrayAllocHeader<_Ty>());
    }

    template< typename _Ty >
    inline _Ty* hConstructArray(_Ty* ptr, hSizeT count)
    {
        _Ty* newptr = (_Ty*)((hByte*)ptr+hCalcArrayAllocHeader< _Ty >());
        if (newptr != ptr) {
            *((hSizeT*)ptr) = count;
        }

        for (hSizeT i = 0; i < count; ++i)
            hPLACEMENT_NEW((void*)(newptr+i)) _Ty;

        return newptr;
    }
}
#ifdef HEART_PLAT_WINDOWS
#   if _MSC_VER >= 1700
#       define hPRIVATE_DESTRUCTOR()\
            template< typename t_ty > friend void Heart::hDestroyObjects(t_ty*, hSizeT);\
            template < typename t_ty > friend struct std::_Get_align; /*MSVC-Secret hidden type that needs exposing*/
#   elif _MSC_VER >= 1500
#       define hPRIVATE_DESTRUCTOR()\
            template< typename t_ty > friend void Heart::hDestroyObjects(t_ty*, hSizeT); \
            template < typename t_ty > friend struct std::tr1::_Get_align; /*MSVC-Secret hidden type that needs exposing*/
#   else
#       define hPRIVATE_DESTRUCTOR()\
            template< typename t_ty > friend void Heart::hDestroyObjects(t_ty*, hSizeT);
#   endif
#else
#   pragma error ("Unknow platform")
#endif

#ifndef HEART_USE_DEFAULT_MEM_ALLOC

#ifdef HEART_DEBUG
    #define hAlignMalloc( s, a ) Heart::GetGlobalHeap()->alloc( s, a, __FILE__, __LINE__ )
    #define hMalloc( s ) hAlignMalloc( s, 16 )
    #define hRealloc( p, s ) Heart::GetGlobalHeap()->realloc( p, 16, s, __FILE__, __LINE__ )
    #define hAlignRealloc( p, a, s ) Heart::GetGlobalHeap()->realloc( p, a, s, __FILE__, __LINE__ )
    #define hFree( p ) Heart::GetGlobalHeap()->release( p )
    #define hFreeSafe( p ) Heart::GetGlobalHeap()->release( p ); p = NULL
    #define hHeapAlignMalloc( h, s, a )  (h)->alloc( s, a, __FILE__, __LINE__ )
    #define hHeapMalloc( h, s ) (h)->alloc( s, 16, __FILE__, __LINE__ )
    #define hHeapRealloc( h, p, s ) (h)->realloc( p, 16, s, __FILE__, __LINE__ )
    #define hHeapAlignRealloc( h, p, a, s ) (h)->realloc( p, a, s, __FILE__, __LINE__ )
    #define hHeapFree( h, p ) (h)->release( p )
    #define hHeapFreeSafe( h, p ) (h)->release( p ); p = NULL
#else
    #define hAlignMalloc( s, a ) Heart::GetGlobalHeap()->alloc( s, a )
    #define hMalloc( s ) hAlignMalloc( s, 16 )
    #define hRealloc( p, s ) Heart::GetGlobalHeap()->realloc( p, 16, s )
    #define hAlignRealloc( p, a, s ) Heart::GetGlobalHeap()->realloc( p, a, s )
    #define hFree( p ) Heart::GetGlobalHeap()->release( p )
    #define hFreeSafe( p ) Heart::GetGlobalHeap()->release( p ); p = NULL
    #define hHeapAlignMalloc( h, s, a )  (h)->alloc( s, a )
    #define hHeapMalloc( h, s ) (h)->alloc( s, 16 )
    #define hHeapRealloc( h, p, s ) (h)->realloc( p, 16, s )
    #define hHeapAlignRealloc( h, p, a, s ) (h)->realloc( p, a, s )
    #define hHeapFree( h, p ) (h)->release( p )
    #define hHeapFreeSafe( h, p ) (h)->release( p ); p = NULL
#endif

    /*
        worth noting that placement new (with vs2008, may be different on other compilers) 
        stores the array size in the first 4 bytes of allocation. So delete array will
        grab that header
    */

    #define hNEW(heap, type)                    hPLACEMENT_NEW(hHeapAlignMalloc(heap,sizeof(type),hAlignOf(type))) type
    #define hNEW_ALIGN(heap, align, type)       hPLACEMENT_NEW(hHeapAlignMalloc(heap,sizeof(type),align)) type
    #define hNEW_ARRAY(heap, type, ele)         Heart::hConstructArray< type >( (type*)hHeapMalloc(heap,(sizeof(type)*ele)+Heart::hCalcArrayAllocHeader<type>()), ele )
    //#define hNEW_ARRAY_ALIGN(heap, align, type) hPLACEMENT_NEW(hHeapAlignMalloc(heap,sizeof(type)*ele,align)) type
    #define hDELETE(heap, ptr)                  Heart::hDestroyObjects(ptr,1); hHeapFree(heap,ptr)
    #define hDELETE_ALIGNED(heap, ptr)          Heart::hDestroyObjects(ptr,1); hHeapFree(heap,ptr)
    #define hDELETE_ARRAY(heap, ptr)            Heart::hDestroyObjects(ptr,Heart::hCalcArrayAllocCount(ptr)); hHeapFree(heap,Heart::hCalcArrayAllocCorrectPointer(ptr))
    #define hDELETE_SAFE(heap, ptr)             Heart::hDestroyObjects(ptr,1); hHeapFree(heap,ptr); ptr = NULL
    #define hDELETE_ARRAY_SAFE(heap, ptr)       Heart::hDestroyObjects(ptr,Heart::hCalcArrayAllocCount(ptr)); hHeapFree(heap,Heart::hCalcArrayAllocCorrectPointer(ptr)); ptr = NULL
#if 0 // may want to enable these later to check only heaps are being used
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
#endif
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

    #define hNEW(heap, type)                    new type
    #define hNEW_ALIGN(heap, align, type)       hPLACEMENT_NEW(hAlignMalloc(sizeof(type),align)) type
    #define hNEW_ARRAY(heap, type, ele)         new type [ele]
    //#define hNEW_ARRAY_ALIGN(heap, align, type) hPLACEMENT_NEW(hHeapAlignMalloc(heap,sizeof(type)*ele,align)) type
    #define hDELETE(heap, ptr)                  delete ptr
    #define hDELETE_ALIGNED(heap, ptr)          hFree(ptr)
    #define hDELETE_ARRAY(heap, ptr)            delete[] ptr
    #define hDELETE_SAFE(heap, ptr)             delete ptr; ptr = NULL
    #define hDELETE_ARRAY_SAFE(heap, ptr)       delete[] ptr; ptr = NULL
/*
    inline void* operator new ( size_t size )
    {
        return hMalloc(size);
    }

    inline void* operator new[] ( size_t size )
    {
        return hMalloc(size);
    }

    inline void operator delete ( void* mem ) 
    {
        hFree(mem);
    }

    inline void operator delete[] ( void* mem )
    {
        hFree(mem);
    }
*/
#endif

#endif // HMEMORYDEFINES_H__