/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef CRYPTOCOMMON_H__
#define CRYPTOCOMMON_H__

typedef unsigned int        cyUint;
typedef int                 cyInt;
typedef unsigned long       cyUint32;
typedef unsigned long long  cyUint64;
typedef long                cyInt32;
typedef unsigned char       cyByte;
typedef char                cyChar;

#if defined (PLATFORM_WINDOWS) && defined (BUILDING_DLL)
#	ifdef (crypto_EXPORTS)
#   	define CY_DLLEXPORT __declspec(dllexport)
#	else
#   	define CY_DLLEXPORT __declspec(dllimport)
#	endif
#else
#   define CY_DLLEXPORT 
#endif

#if defined PLATFORM_WINDOWS
#	define CY_API __cdecl
#elif PLATFORM_LINUX
#	if BUILD_64_BIT
#		define CY_API
#	else
#		define CY_API __attribute__((cdecl))
#	endif
#else
#	error
#endif

#define CY_OK           (0)

#define CY_MD5_LEN      (16)

#define cyMin( x, y ) (( x < y ) ? x : y)
#define cyMax( x, y ) (( x > y ) ? x : y)

#endif // CRYPTOCOMMON_H__