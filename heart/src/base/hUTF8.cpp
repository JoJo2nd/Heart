/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hUTF8.h"

namespace Heart {
namespace hUTF8 {

HEART_EXPORT 
hUint HEART_API bytesRequiredForUTF8(const Unicode& ucin) {
    if( ucin < 0x80 ) {
        return 1;
    } else if( ucin < 0x800 ) { // 110xxxxx 10xxxxxx
        return 2;
    } else if( ucin < 0x10000 ) { // 1110xxxx 10xxxxxx 10xxxxxx
        return 3;
    }

    return 0;
}

HEART_EXPORT 
hUint32 HEART_API encodeFromUnicode(Unicode ucIn, hChar* utf8Out)
{
    hUint32 ret = 0;
    // 0xxxxxxx
    if( ucIn < 0x80 )
    {
        utf8Out[ ret++ ] = 0x007F & ucIn;
    }
    // 110xxxxx 10xxxxxx
    else if( ucIn < 0x800 )
    {
        utf8Out[ ret++ ] = 0x00FF & ( MASK2BYTES | ( ucIn >> 6 ) );
        utf8Out[ ret++ ] = 0x00FF & ( MASKBYTE | ( ucIn & MASKBITS ) );
    }
    // 1110xxxx 10xxxxxx 10xxxxxx
    else if( ucIn < 0x10000 )
    {
        utf8Out[ ret++ ] = 0x00FF & ( MASK3BYTES | ( ucIn >> 12 ) );
        utf8Out[ ret++ ] = 0x00FF & ( MASKBYTE | ( ucIn >> 6 & MASKBITS ) );
        utf8Out[ ret++ ] = 0x00FF & ( MASKBYTE | ( ucIn & MASKBITS ) );
    }

    return ret;
}

HEART_EXPORT 
hUint32 HEART_API DecodeToUnicode( const hChar* hRestrict uft8In, Unicode& ucOut )
{
    hUint32 ret = 0;
    // 1110xxxx 10xxxxxx 10xxxxxx
    if( (uft8In[ ret ] & MASK3BYTES) == MASK3BYTES )
    {
        ucOut = ( (uft8In[ret+1] & 0x0F)     << 12)	|
            ( (uft8In[ret+2] & MASKBITS) << 6 )	|
            ( (uft8In[ret+3] & MASKBITS) );
    }
    // 110xxxxx 10xxxxxx
    else if((uft8In[ ret ] & MASK2BYTES) == MASK2BYTES)
    {
        ucOut = ( ( uft8In[ ret+1 ] & 0x1F ) << 6 ) | ( uft8In[ ret+2 ] & MASKBITS );
    }
    // 0xxxxxxx
    else if(uft8In[ ret ] < MASKBYTE)
    {
        ucOut = uft8In[ ret+1 ];
    }

    return ret;
}

HEART_EXPORT
hUint32 HEART_API BytesInUTF8Character(const hChar* uft8In)
{
    hUint32 ret = 1;
    // 1110xxxx 10xxxxxx 10xxxxxx
    if((*uft8In & MASK3BYTES) == MASK3BYTES)
    {
        ret = 3;
    }
    // 110xxxxx 10xxxxxx
    else if((*uft8In & MASK2BYTES) == MASK2BYTES)
    {
        ret = 2;
    }
    // 0xxxxxxx

    return ret;
}

HEART_EXPORT
hUint HEART_API encodeFromUnicodeString(const Unicode* hRestrict ucin, hUint inlimit, hChar* hRestrict utf8out, hUint outlimit) {
    hUint written=0;
    for (hUint i=0; i<inlimit; ++i) {
        hUint32 bytes=bytesRequiredForUTF8(ucin[i]);
        if (written+bytes>=outlimit) {
            break;
        }
        encodeFromUnicode(ucin[i], utf8out+written);
        written+=bytes;
    }
    return written;
}

#define UTF8_MASKBITS   (0x3F)
#define UTF8_MASKBYTE   (0x80)
#define UTF8_MASK2BYTES (0xC0)
#define UTF8_MASK3BYTES (0xE0)
#define UTF8_MASK4BYTES (0xF0)
#define UTF8_MASK5BYTES (0xF8)
#define UTF8_MASK6BYTES (0xFC)

hUint32 utf8_codepoint(const char* uft8In, Unicode* ucOut) {
    hUint32 ret = 0;
    if( (uft8In[ ret ] & UTF8_MASK3BYTES) == UTF8_MASK3BYTES ) { // 1110xxxx 10xxxxxx 10xxxxxx
        *ucOut = ( (uft8In[ret] & 0x0F)     << 12)    |
            ( (uft8In[ret+1] & UTF8_MASKBITS) << 6 )|
            ( (uft8In[ret+3] & UTF8_MASKBITS) );
        ret=3;
    } else if((uft8In[ ret ] & UTF8_MASK2BYTES) == UTF8_MASK2BYTES) { // 110xxxxx 10xxxxxx
        *ucOut = ( ( uft8In[ ret ] & 0x1F ) << 6 ) | ( uft8In[ ret+1 ] & UTF8_MASKBITS );
        ret=2;
    } else if(uft8In[ ret ] < UTF8_MASKBYTE) { // 0xxxxxxx
        *ucOut = uft8In[ ret ];
        ret=1;
    }
    return ret;
}

/*
 * Returns number of bytes written excluding the null terminator
 */
HEART_EXPORT
hSize_t HEART_API utf8_to_uc2(const char* src, Unicode* dst, hSize_t len) {
    hSize_t r = 0;
    len -= sizeof(Unicode);   /* save room for null char. */
    while (len >= sizeof(Unicode))
    {
        hUint32 cp = utf8_codepoint(src, dst);
        if (cp == 0)
            break;

        dst++;// = (minfs_uint16_t)(cp & 0xFFFF);
        src++;
        len -= sizeof(Unicode);
        ++r;
    } /* while */

    *dst = 0;
    return r;
}

/*
 * Returns number of bytes written excluding the null terminator
 */
HEART_EXPORT
hSize_t HEART_API uc2_to_utf8(Unicode* uc_in, char* utf8_out, size_t buf_size) {
    hSize_t ret = 0;
    while (buf_size > 0 && *uc_in) {    // 0xxxxxxx
        if(*uc_in < 0x80 && buf_size > 0) {
            utf8_out[ ret++ ] = 0x007F & *uc_in;
            buf_size -= 1;
        } else if(*uc_in < 0x800 && buf_size > 1) {// 110xxxxx 10xxxxxx
            utf8_out[ ret++ ] = 0x00FF & ( UTF8_MASK2BYTES | ( *uc_in >> 6 ) );
            utf8_out[ ret++ ] = 0x00FF & ( UTF8_MASKBYTE | ( *uc_in & UTF8_MASKBITS ) );
            buf_size -= 2;
        } else if(*uc_in < 0x10000 && buf_size > 2) { // 1110xxxx 10xxxxxx 10xxxxxx
            utf8_out[ ret++ ] = 0x00FF & ( UTF8_MASK3BYTES | ( *uc_in >> 12 ) );
            utf8_out[ ret++ ] = 0x00FF & ( UTF8_MASKBYTE | ( *uc_in >> 6 & UTF8_MASKBITS ) );
            utf8_out[ ret++ ] = 0x00FF & ( UTF8_MASKBYTE | ( *uc_in & UTF8_MASKBITS ) );
            buf_size -= 3;
        }
        ++uc_in;
    }

    if (buf_size) {
        utf8_out[ret] = 0;
    }
    return ret;
}

}
}