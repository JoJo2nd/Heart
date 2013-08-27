/********************************************************************

	filename: 	hDebugMacros.cpp	
	
	Copyright (c) 1:4:2012 James Moran
	
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

#ifdef WIN32
    #include <windows.h>
#else
    #error ("Platform not supported")
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static hPrintfCallback g_printcallback = NULL;

HEART_DLLEXPORT void HEART_API hcOutputStringVA( const hChar* msg, bool newline, va_list vargs );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
HEART_DLLEXPORT 
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
HEART_DLLEXPORT
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

HEART_DLLEXPORT
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

HEART_DLLEXPORT hUint32 HEART_API hAssertMsgFunc(hBool ignore, const hChar* msg, ...)
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

HEART_DLLEXPORT void HEART_API hcSetOutputStringCallback( hPrintfCallback cb )
{
    g_printcallback = cb;
}
