/********************************************************************

    filename:   cryptoBase64.h  
    
    Copyright (c) 14:4:2013 James Moran
    
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

#pragma once

#ifndef CRYPTOBASE64_H__
#define CRYPTOBASE64_H__

#include "cryptoCommon.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif//

    CY_DLLEXPORT cyUint CY_API cyBase64EncodeCalcRequiredSize(cyUint inputCount);
    CY_DLLEXPORT cyUint CY_API cyBase64Encode(const void* inputbuf, cyUint inputCount, void* outputbuf, cyUint outputLimit);
    CY_DLLEXPORT cyUint CY_API cyBase64DecodeCalcRequiredSize(const void* inputbuf, cyUint inputCount);
    CY_DLLEXPORT cyUint CY_API cyBase64Decode(const void* inputbuf, cyUint inputCount, void* outputbuf, cyUint outputLimit);

#ifdef __cplusplus
};
#endif

#endif // CRYPTOBASE64_H__