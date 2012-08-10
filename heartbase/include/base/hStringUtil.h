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
    HEARTBASE_SLIBEXPORT
    inline hUint32 HEART_API hStrLen( const hChar* s1 )
    {
        return strlen( s1 );
    }

    HEARTBASE_SLIBEXPORT
    inline void HEART_API hStrCopy( hChar* dest, hUint32 destlen, const hChar* src )
    {
        hcAssert( hStrLen( src ) < destlen );
        strcpy( dest, src );
    }

    HEARTBASE_SLIBEXPORT
    inline void HEART_API hStrNCopy( hChar* dest, hUint32 destlen, const hChar* src )
    {
        strncpy( dest, src, destlen-1 );
        dest[destlen-1] = 0;
    }

    HEARTBASE_SLIBEXPORT
    inline void HEART_API hStrCat( hChar* dest, hUint32 destlen, const hChar* src )
    {
        hcAssert( destlen - hStrLen( dest ) < destlen );
        strcat( dest, src );
    }

    HEARTBASE_SLIBEXPORT
    inline hUint32 HEART_API hStrCmp( const hChar* s1, const hChar* s2 )
    {
        return strcmp( s1, s2 );
    }

    HEARTBASE_SLIBEXPORT
    inline hChar* HEART_API hStrChr( hChar* s1, hChar ch)
    {
        return strchr(s1,(hInt)ch);
    }

    HEARTBASE_SLIBEXPORT
    inline hChar* HEART_API hStrRChr( hChar* s1, hChar ch)
    {
        return strrchr(s1,(hInt)ch);
    }

    HEARTBASE_SLIBEXPORT
    inline const hChar* HEART_API hStrChr( const hChar* s1, hChar ch)
    {
        return strchr(s1,(hInt)ch);
    }

    HEARTBASE_SLIBEXPORT
    inline const hChar* HEART_API hStrRChr( const hChar* s1, hChar ch)
    {
        return strrchr(s1,(hInt)ch);
    }

    HEARTBASE_SLIBEXPORT
    inline hUint32 HEART_API hStrICmp( const hChar* s1, const hChar* s2 )
    {
#ifdef WIN32
        return _stricmp( s1, s2 );
#else
        return stricmp( s1, s2 );
#endif
    }

    HEARTBASE_SLIBEXPORT
    inline hInt32 HEART_API hStrPrintf( hChar* dest, hUint32 destlen, const hChar* format, ... )
    {
        va_list marker;
        va_start( marker, format );

        hInt32 r = vsprintf_s( dest, destlen, format, marker );

        va_end( marker );

        return r;
    }

    HEARTBASE_SLIBEXPORT
    inline hInt32 HEART_API hAtoI(const hChar* str)
    {
        return atoi(str);
    }

    HEARTBASE_SLIBEXPORT
    inline hFloat HEART_API hAtoF(const hChar* str)
    {
        return (hFloat)atof(str);
    }

    HEARTBASE_SLIBEXPORT
    int HEART_API hStrWildcardMatch(const hChar *wildcard, const hChar *target);
}

#endif // HSTRINGUTIL_H__