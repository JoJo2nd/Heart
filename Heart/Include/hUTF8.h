/********************************************************************
	created:	2010/09/05
	created:	5:9:2010   18:28
	filename: 	Locale.h	
	author:		James
	
	purpose:	
*********************************************************************/

#include "hTypes.h"

namespace Heart
{
namespace hUTF8
{
	static const hUint32 MASKBITS   = 0x3F;
	static const hUint32 MASKBYTE   = 0x80;
	static const hUint32 MASK2BYTES = 0xC0;
	static const hUint32 MASK3BYTES = 0xE0;
	static const hUint32 MASK4BYTES = 0xF0;
	static const hUint32 MASK5BYTES = 0xF8;
	static const hUint32 MASK6BYTES = 0xFC;

	typedef hUint16   Unicode;

	inline hUint32 EncodeFromUnicode( Unicode ucIn, hChar* utf8Out )
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

	inline hUint32 DecodeToUnicode( const hChar* uft8In, Unicode& ucOut )
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