/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef HMEMORYDEFINES_H__
#define HMEMORYDEFINES_H__

#include <exception>
#include <new>

#define hPLACEMENT_NEW(ptr)                 ::new ((void*)ptr) 

namespace Heart {
    void* hMalloc(hSize_t size, hSize_t alignment=HEART_MIN_ALLOC_ALIGNMENT);
    void* hRealloc(void* ptr, hSize_t size, hSize_t alignment=HEART_MIN_ALLOC_ALIGNMENT);
    void  hFree(void* ptr);
}    

#endif // HMEMORYDEFINES_H__