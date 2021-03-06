/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "pal/hPlatform.h"
#include "base/hTypes.h"
#include <stdio.h>
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static hPrintfCallback g_printcallback = NULL;

static void HEART_API hcOutputStringFlush() {
#if defined (PLATFORM_LINUX)
    fflush(stdout);
#endif
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void HEART_API hcOutputStringVA( const hChar* msg, bool newline, va_list vargs )
{
#ifdef HEART_DEBUG
    #ifdef HEART_PLAT_WINDOWS
        const hUint32 ks = 4;
        hChar buffer[ ks*1024 ];
        buffer[ (ks*1024)-1 ] = 0;
        hUint32 len = vsnprintf(buffer, ks*1024, msg, vargs);

        if ( newline && buffer[ len - 1 ] != '\n' )
        {
            buffer[ len ] = '\n';
            buffer[ len+1 ] =  0;
        }
        OutputDebugString( buffer );
    #endif

    #if defined (PLATFORM_LINUX)
        hUint32 len = vprintf(msg, vargs);
        if (msg[len-1] != '\n' ) {
            putchar('\n');
        }
        hcOutputStringFlush();     
    #endif
    // if (g_printcallback)
    //     g_printcallback(buffer);
#endif
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void HEART_API hcOutputString( const hChar* msg, ... )
{
	va_list marker;
	va_start(marker, msg);
    hcOutputStringVA(msg, true, marker);
	va_end(marker);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 HEART_API hAssertMsgFunc(hBool ignore, const hChar* msg, ...)
{
    va_list marker;
    va_start( marker, msg );
    const hUint32 ks = 4;
    hChar buffer[ ks*1024 ];
    hUint32 ret = 0;

    buffer[ (ks*1024)-1 ] = 0;

    hUint32 len = vsnprintf(buffer, ks*1024, msg, marker);

    if ( buffer[ len - 1 ] != '\n' )
    {
        buffer[ len ] = '\n';
        buffer[ len+1 ] =  0;
    }

    hcOutputString( buffer );
#if 0
    if (!ignore) {
     ret = 1;//MessageBox(NULL, buffer, "ASSERT FAILED!", MB_ABORTRETRYIGNORE);
     if (ret == IDABORT) {
         ret = 1; // break to debugger
         //ret = 0; // abort
     }
     else if (ret == IDRETRY) {
         ret = 1; // break to debugger
     }
     else {
         ret = 2; // ignore
     }
    }
    else {
        ret = 2; // ignore
    }
#else
    ret = 1;
#endif
    va_end( marker );

    return ret;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void HEART_API hcSetOutputStringCallback( hPrintfCallback cb )
{
    g_printcallback = cb;
}

void debug_break_now() {
    raise(SIGTRAP);
}
