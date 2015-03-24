/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef HSTRINGUTIL_H__
#define HSTRINGUTIL_H__

#include "base/hTypes.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>

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

    hFORCEINLINE hInt hStrNCmp(const hChar* str1, const hChar* str2, hSize_t n) {
        return strncmp(str1, str2, n);
    }
}

#endif // HSTRINGUTIL_H__
