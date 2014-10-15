/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hUTF8.h"

namespace Heart
{
namespace hUTF8
{   

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint bytesRequiredForUTF8(const Unicode& ucin) {
    if( ucin < 0x80 ) {
        return 1;
    } else if( ucin < 0x800 ) { // 110xxxxx 10xxxxxx
        return 2;
    } else if( ucin < 0x10000 ) { // 1110xxxx 10xxxxxx 10xxxxxx
        return 3;
    }

    return 0;
}


hUint32 encodeFromUnicode(Unicode ucIn, hChar* utf8Out)
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

hUint32 DecodeToUnicode( const hChar* hRestrict uft8In, Unicode& ucOut )
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


hUint32 BytesInUTF8Character(const hChar* uft8In)
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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint encodeFromUnicodeString(const Unicode* hRestrict ucin, hUint inlimit, hChar* hRestrict utf8out, hUint outlimit) {
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
}
}