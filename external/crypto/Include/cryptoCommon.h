/********************************************************************

	filename: 	cryptoCommon.h	
	
	Copyright (c) 13:10:2011 James Moran
	
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

#ifndef CRYPTOCOMMON_H__
#define CRYPTOCOMMON_H__

typedef unsigned int        cyUint;
typedef int                 cyInt;
typedef unsigned long       cyUint32;
typedef unsigned long long  cyUint64;
typedef long                cyInt32;
typedef unsigned char       cyByte;
typedef char                cyChar;

#ifdef CRYPTO_COMPILE_DLL
#   define CY_DLLEXPORT __declspec(dllexport)
#elif CRYPTO_IMPORT_DLL
#   define CY_DLLEXPORT __declspec(dllimport)
#else
#   define CY_DLLEXPORT 
#endif

#define CY_API          __cdecl

#define CY_OK           (0)

#define CY_MD5_LEN      (16)

#endif // CRYPTOCOMMON_H__