/********************************************************************

	filename: 	hBase64.h	
	
	Copyright (c) 8:11:2012 James Moran
	
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
/*
 * Based on code found at http://base64.sourceforge.net/ 
 **/
#pragma once

#ifndef HBASE64_H__
#define HBASE64_H__

#include "base/hTypes.h"

namespace Heart {
namespace hBase64 {
    hUint HEART_API EncodeCalcRequiredSize(hUint inputCount);
    hUint HEART_API Encode(const void* inputbuf, hUint inputCount, void* outputbuf, hUint outputLimit);
    hUint HEART_API DecodeCalcRequiredSize(const void* inputbuf, hUint inputCount);
    hUint HEART_API Decode(const void* inputbuf, hUint inputCount, void* outputbuf, hUint outputLimit);
}
}

#endif // HBASE64_H__