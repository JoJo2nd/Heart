/********************************************************************

	filename: 	GameDatabaseCommon.h	
	
	Copyright (c) 12:10:2011 James Moran
	
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
#ifndef GAMEDATABASECOMMON_H__
#define GAMEDATABASECOMMON_H__

#include <assert.h>
#include <sys/types.h>

typedef unsigned long long  gdUint64;
typedef long long           gdInt64;
typedef unsigned long       gdUint32;
typedef long                gdInt32;
typedef unsigned int        gdUint;
typedef int                 gdInt;
typedef unsigned short      gdUint16;
typedef short               gdInt16;
typedef unsigned char       gdByte;
typedef char                gdChar;
typedef wchar_t             gdWchar;
typedef float               gdFloat;
typedef double              gdDouble;
typedef bool                gdBool;
typedef time_t              gdTime_t;

typedef std::basic_string< gdChar > gdString;
typedef std::basic_string< gdWchar > gdWString;

//TEMP typedef
typedef gdUint32        gdError;

#define gdBreak					__asm { int 3 } 
#define gdAssert( x )			assert( x )

#define gdDLL_EXPORT            __declspec(dllexport)

#define gdERROR_OK              (0)
#define gdERROR_NONE            gdERROR_OK

#define gdERROR_GENERIC                     (0x80000001)
#define gdERROR_PATH_TOO_LONG               (0x80000002)
#define gdERROR_INVALID_CRC                 (0x80000003)
#define gdERROR_INVALID_PARAM               (0x80000004)
#define gdERROR_RESOURCE_NAME_CRC_CONFLICT  (0x80000005)
#define gdERROR_UNHANDLED_SAVE_EXPECTION    (0x80000006)
#define gdERROR_RESOURCE_NOT_FOUND          (0x80000007)
#define gdERROR_DUPLICATE_PLUGIN            (0x80000008)
#define gdERROR_NOT_RESOURCE_TYPE_INFO      (0x80000009)
#define gdERROR_BUILD_CANCELLED             (0x8000000A)
#define gdERROR_PLUGIN_NOT_FOUND            (0x8000000B)

#define gdPREVENT_COPY( klass ) \
    private:\
        klass::klass( const klass& c );\
        klass& operator = ( const klass& c );

#define gdDEFINE_SAVE_VERSION( v ) \
    static const gdUint     saveVersion_ = v;

#define gdDECLARE_SAVE_VERSION( klass ) \
    BOOST_CLASS_VERSION( klass, klass::saveVersion_ )

#ifndef NULL
    #define NULL (0)
#endif

#define gdAPI   gdDLL_EXPORT 

#ifdef GAMEDATABASELIB_EXPORTS //Hide stupid ms warning C4251
    #pragma warning( push )
    #pragma warning( disable: 4251 )
#endif

#ifdef GAMEDATABASELIB_EXPORTS //Stop Hiding warning C4251
    #pragma warning ( pop )
#endif
    

#define gdDEFINE_DLL_FUNCTION_PROTOTYPE( ret, x )\
    extern const gdChar* gdDLL_##x;\
    typedef ret ( __cdecl *gdDLLPT_##x)
#define gdDECLARE_DLL_FUNCTION_PROTOTYPE( ret, x )\
    const gdChar* gdDLL_##x = #x;
#define gdGET_DLL_PROC_ADDRESS( handle, name ) \
    (gdDLLPT_##name)GetProcAddress( handle, gdDLL_##name );

#endif // GAMEDATABASECOMMON_H__
