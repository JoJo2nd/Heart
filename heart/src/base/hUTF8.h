/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"

namespace Heart {
namespace hUTF8 {
    static const hInt32 MASKBITS   = 0x3F;
    static const hInt32 MASKBYTE   = 0x80;
    static const hInt32 MASK2BYTES = 0xC0;
    static const hInt32 MASK3BYTES = 0xE0;
    static const hInt32 MASK4BYTES = 0xF0;
    static const hInt32 MASK5BYTES = 0xF8;
    static const hInt32 MASK6BYTES = 0xFC;

    typedef hUint16   Unicode;

    HEART_EXPORT hUint32 HEART_API encodeFromUnicode(Unicode ucIn, hChar* utf8Out);
    HEART_EXPORT hUint   HEART_API encodeFromUnicodeString(const Unicode* hRestrict ucin, hUint inlimit, hChar* hRestrict utf8out, hUint outlimit);
    HEART_EXPORT hUint   HEART_API bytesRequiredForUTF8(const Unicode& ucin);
    HEART_EXPORT hUint32 HEART_API DecodeToUnicode( const hChar* hRestrict uft8In, Unicode& ucOut );
    HEART_EXPORT hUint32 HEART_API BytesInUTF8Character(const hChar* uft8In);
    HEART_EXPORT hSize_t HEART_API utf8_to_uc2(const char* src, Unicode* dst, hSize_t len);
    HEART_EXPORT hSize_t HEART_API uc2_to_utf8(Unicode* uc_in, char* utf8_out, size_t buf_size);
}
}