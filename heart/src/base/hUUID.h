/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"

namespace Heart {
namespace hUUID {
HEART_EXPORT hUuid_t HEART_API generateUUID();
HEART_EXPORT hBool HEART_API compareUUID(const hUuid_t& lhs, const hUuid_t& rhs);
HEART_EXPORT hSize_t HEART_API toStringSize(const hUuid_t& a);
HEART_EXPORT hSize_t HEART_API toString(const hUuid_t& a, hChar* out, hSize_t out_size);
HEART_EXPORT hUuid_t HEART_API fromString(const hChar* out, hSize_t in_size);
}
}
