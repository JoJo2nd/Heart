/********************************************************************

	filename: 	Types.h	
	
	Copyright (c) 23:10:2011 James Moran
	
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

#ifndef TYPES_H__
#define TYPES_H__

#include "assert.h"

typedef	char				pChar;
typedef unsigned char		pByte;
typedef unsigned short		pUint16;
typedef short				pInt16;
typedef unsigned long		pUint32;
typedef long				pInt32;
typedef unsigned long long  pUint64;
typedef long long			pInt64;
typedef int					pInt;
typedef float				pFloat;
typedef double				pDouble;
typedef bool				pBool;

#ifndef pAssert
    #define pAssert(x)      assert(x)
#endif 

#endif // TYPES_H__