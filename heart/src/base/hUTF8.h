/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"

namespace Heart
{
namespace hUTF8
{
    static const hInt32 MASKBITS   = 0x3F;
    static const hInt32 MASKBYTE   = 0x80;
    static const hInt32 MASK2BYTES = 0xC0;
    static const hInt32 MASK3BYTES = 0xE0;
    static const hInt32 MASK4BYTES = 0xF0;
    static const hInt32 MASK5BYTES = 0xF8;
    static const hInt32 MASK6BYTES = 0xFC;

    typedef hUint16   Unicode;

    hUint32 encodeFromUnicode(Unicode ucIn, hChar* utf8Out);
    hUint   encodeFromUnicodeString(const Unicode* hRestrict ucin, hUint inlimit, hChar* hRestrict utf8out, hUint outlimit);
    hUint   bytesRequiredForUTF8(const Unicode& ucin);
    hUint32 DecodeToUnicode( const hChar* hRestrict uft8In, Unicode& ucOut );
    hUint32 BytesInUTF8Character(const hChar* uft8In);

}
}