/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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