/********************************************************************

	filename: 	hMemoryUtil.h	
	
	Copyright (c) 15:12:2011 James Moran
	
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

#ifndef HMEMORYUTIL_H__
#define HMEMORYUTIL_H__

#include <memory.h>

namespace Heart
{
    hFORCEINLINE void hMemSet(void* dest, hByte val, hSize_t size) {
        memset(dest, val, size);
    }

    hFORCEINLINE void hMemCpy( void* dest, const void* src, hSize_t size ) {
        memcpy( dest, src, size );
    }

    hFORCEINLINE void hZeroMem( void* dest, hSize_t size ) {
        memset( dest, 0, size );
    }

    hFORCEINLINE void hMemMove(void* dest, const void* src, hSize_t size) {
        memmove(dest, src, size);
    }
}

#endif // HMEMORYUTIL_H__