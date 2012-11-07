/********************************************************************

	filename: 	hUTF8.cpp	
	
	Copyright (c) 11:7:2012 James Moran
	
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

namespace Heart
{
namespace hUTF8
{   

HEART_DLLEXPORT
hUint32 EncodeFromUnicode( Unicode ucIn, hChar* utf8Out )
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

HEART_DLLEXPORT
hUint32 HEART_API DecodeToUnicode( const hChar* uft8In, Unicode& ucOut )
{
    hUint32 ret = 0;
    // 1110xxxx 10xxxxxx 10xxxxxx
    if( (uft8In[ ret ] & MASK3BYTES) == MASK3BYTES )
    {
        ucOut = ( (uft8In[ret++] & 0x0F)     << 12)	|
            ( (uft8In[ret++] & MASKBITS) << 6 )	|
            ( (uft8In[ret++] & MASKBITS) );
    }
    // 110xxxxx 10xxxxxx
    else if((uft8In[ ret ] & MASK2BYTES) == MASK2BYTES)
    {
        ucOut = ( ( uft8In[ ret++ ] & 0x1F ) << 6 ) | ( uft8In[ ret++ ] & MASKBITS );
    }
    // 0xxxxxxx
    else if(uft8In[ ret ] < MASKBYTE)
    {
        ucOut = uft8In[ ret++ ];
    }

    return ret;
}

}
}