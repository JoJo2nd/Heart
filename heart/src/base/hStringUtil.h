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

#include "base/hTypes.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

namespace Heart
{
    inline hUint32 HEART_API hStrLen( const hChar* s1 )
    {
        return (hUint32)strlen( s1 );
    }

    inline void HEART_API hStrCopy( hChar* dest, hUint32 destlen, const hChar* src )
    {
        hcAssert( (hUint32)hStrLen( src ) <= destlen );
        strcpy( dest, src );
    }

    inline void HEART_API hStrNCopy( hChar* dest, hUint32 destlen, const hChar* src )
    {
        strncpy( dest, src, destlen-1 );
        dest[destlen-1] = 0;
    }

    inline void HEART_API hStrCat( hChar* dest, hUint32 destlen, const hChar* src )
    {
        hcAssert( destlen - (hUint32)hStrLen( dest ) <= destlen );
        strcat( dest, src );
    }

    inline hInt HEART_API hStrCmp( const hChar* s1, const hChar* s2 )
    {
        return strcmp( s1, s2 );
    }

    inline hChar* HEART_API hStrChr( hChar* s1, hChar ch)
    {
        return strchr(s1,(hInt)ch);
    }

    inline hChar* HEART_API hStrRChr(hChar* s1, hChar ch)
    {
        return strrchr(s1,(hInt)ch);
    }

    inline const hChar* HEART_API hStrRChr(const hChar* s1, hChar ch)
    {
        return strrchr(s1,(hInt)ch);
    }

    inline const hChar* HEART_API hStrChr( const hChar* s1, hChar ch)
    {
        return strchr(s1,(hInt)ch);
    }

    inline hUint32 HEART_API hStrICmp( const hChar* s1, const hChar* s2 )
    {
#if defined (PLATFORM_WINDOWS)
        return _stricmp( s1, s2 );
#elif defined (PLATFORM_LINUX)
        return strcasecmp( s1, s2 );
#else
#       error ("Unknown platform")
#endif
    }

    inline hInt32 HEART_API hStrPrintf( hChar* dest, hUint32 destlen, const hChar* format, ... )
    {
        va_list marker;
        va_start( marker, format );

#if defined (PLATFORM_LINUX)
        hInt32 r = vsprintf( dest, format, marker );
#elif defined (PLATFORM_WINDOWS)
        hInt32 r = vsprintf_s( dest, destlen, format, marker );
#else
#       error ("Unknown platform")
#endif

        va_end( marker );

        return r;
    }

    inline void HEART_API hStrMove(hChar* src, hChar* dest, hUint count)
    {
        memmove(dest, src, count);
    }

#define hScanf scanf

    inline hInt32 HEART_API hAtoI(const hChar* str)
    {
        return atoi(str);
    }

    inline hFloat HEART_API hAtoF(const hChar* str)
    {
        return (hFloat)atof(str);
    }

    int HEART_API hStrWildcardMatch(const hChar *wildcard, const hChar *target);

    inline hInt HEART_API hIsSpace(hChar c)
    {
        return isspace(c);
    }
}

#endif // HSTRINGUTIL_H__
