/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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