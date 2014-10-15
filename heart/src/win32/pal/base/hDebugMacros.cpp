/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "pal/hPlatform.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static hPrintfCallback g_printcallback = NULL;

void HEART_API hcOutputStringVA( const hChar* msg, bool newline, va_list vargs );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void HEART_API hcOutputString( const hChar* msg, ... )
{
	va_list marker;
	va_start( marker, msg );

	hcOutputStringVA( msg, true, marker );

	va_end( marker );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void HEART_API hcOutputStringRaw( const hChar* msg, ... )
{
	va_list marker;
	va_start( marker, msg );

	hcOutputStringVA( msg, false, marker );

	va_end( marker );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void HEART_API hcOutputStringVA( const hChar* msg, bool newline, va_list vargs )
{
    const hUint32 ks = 4;
	hChar buffer[ ks*1024 ];
	buffer[ (ks*1024)-1 ] = 0;
	hUint32 len = vsprintf_s( buffer, ks*1024, msg, vargs );

	if ( newline && buffer[ len - 1 ] != '\n' )
	{
		buffer[ len ] = '\n';
		buffer[ len+1 ] =  0;
	}

#ifdef HEART_DEBUG
	OutputDebugString( buffer );
#endif // HEART_DEBUG
#ifdef HEART_STDIO_OUTPUT
	printf( buffer );
#else
    if (g_printcallback)
        g_printcallback(buffer);
#endif
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

    hUint32 len = vsprintf_s( buffer, ks*1024, msg, marker );

    if ( buffer[ len - 1 ] != '\n' )
    {
        buffer[ len ] = '\n';
        buffer[ len+1 ] =  0;
    }

#ifdef HEART_PLAT_WINDOWS
#   ifdef HEART_DEBUG
    OutputDebugString( buffer );
#   endif // HEART_DEBUG
    if (!ignore) {
     ret = MessageBox(NULL, buffer, "ASSERT FAILED!", MB_ABORTRETRYIGNORE);
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
