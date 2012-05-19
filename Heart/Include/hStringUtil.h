/********************************************************************

	filename: 	hStringUtil.h	
	
	Copyright (c) 10:12:2011 James Moran
	
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

#ifndef HSTRINGUTIL_H__
#define HSTRINGUTIL_H__

namespace Heart
{
    inline hUint32  hStrLen( const hChar* s1 )
    {
        return strlen( s1 );
    }

    inline void     hStrCopy( hChar* dest, hUint32 destlen, const hChar* src )
    {
        hcAssert( hStrLen( src ) < destlen );
        strcpy( dest, src );
    }

    inline void     hStrNCopy( hChar* dest, hUint32 destlen, const hChar* src )
    {
        strncpy( dest, src, destlen-1 );
        dest[destlen-1] = 0;
    }

    inline void     hStrCat( hChar* dest, hUint32 destlen, const hChar* src )
    {
        hcAssert( destlen - hStrLen( dest ) < destlen );
        strcat( dest, src );
    }

    inline hUint32  hStrCmp( const hChar* s1, const hChar* s2 )
    {
        return strcmp( s1, s2 );
    }

    inline hUint32  hStrICmp( const hChar* s1, const hChar* s2 )
    {
#ifdef WIN32
        return _stricmp( s1, s2 );
#else
        return stricmp( s1, s2 );
#endif
    }

    inline hInt32 hStrPrintf( hChar* dest, hUint32 destlen, const hChar* format, ... )
    {
        va_list marker;
        va_start( marker, format );

        hInt32 r = vsprintf_s( dest, destlen, format, marker );

        va_end( marker );

        return r;
    }

    inline hInt32 hAtoI(const hChar* str)
    {
        return atoi(str);
    }

    inline hFloat hAtoF(const hChar* str)
    {
        return (hFloat)atof(str);
    }
}

#endif // HSTRINGUTIL_H__