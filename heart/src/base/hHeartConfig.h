/********************************************************************

	filename: 	hHeartConfig.h
	
	Copyright (c) 2011/07/11 James Moran
	
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

#ifndef _HHEARTCONFIG_H__
#define _HHEARTCONFIG_H__

#if defined (_DEBUG) || defined (DEBUG)
#   ifndef HEART_DEBUG
#       define HEART_DEBUG
#   endif
#else
#   ifndef HEART_RELEASE
#       define HEART_RELEASE
#   endif
#endif

#define HEART_MIN_ALLOC_ALIGNMENT (16)

#ifndef USE_DL_PREFIX
#   define USE_DL_PREFIX
#endif

#ifndef ONLY_MSPACES
#   define ONLY_MSPACES 0
#endif

// To enable compile at load time of shaders.
#ifdef HEART_DEBUG // or OpenGL when supported
#   define HEART_ALLOW_SHADER_SOURCE_COMPILE
#endif

//#define HEART_ALLOW_NEW

#ifndef XM_NO_OPERATOR_OVERLOADS
#   define XM_NO_OPERATOR_OVERLOADS
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#   define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef PLATFORM_WINDOWS
#   define HEART_PLAT_WINDOWS
#   define HEART_SHARED_LIB_EXT ".dll"
#elif defined (PLATFORM_LINUX)
#	define HEART_PLAT_LINUX
#	define HEART_SHARED_LIB_EXT ".so"
#else
#	error ("Platform not defined")
#endif

#if defined (HEART_FORCE_TRACK_MEMORY_ALLOCS) && !defined (HEART_FORCE_DISABLE_TRACK_MEMORY_ALLOCS)
#   define HEART_TRACK_MEMORY_ALLOCS
#endif

#ifdef HEART_DEBUG
#   if !defined (HEART_TRACK_MEMORY_ALLOCS) && !defined (HEART_FORCE_DISABLE_TRACK_MEMORY_ALLOCS)
#       define HEART_TRACK_MEMORY_ALLOCS
#   endif
#endif

#define HEART_BASECLASS(x) typedef x ParentClass;

#if defined BUILD_64_BIT
#   define HEART_64BIT
#else
#   define HEART_32BIT
#endif

#ifdef PLATFORM_WINDOWS
#   define HEART_API   __cdecl
#elif PLATFORM_LINUX
#	if BUILD_64_BIT
#		define HEART_API
#	else
#		define HEART_API   __attribute__((__cdecl__))
#	endif
#else
#   error "Platform not supported"
#endif

#if defined (HEART_DEBUG)
#   ifdef HEART_PLAT_WINDOWS
#       ifndef D3D_DEBUG_INFO
#           define D3D_DEBUG_INFO
#       endif
#   endif // HEART_PLAT_WINDOWS
#   define HEART_COLLECT_RENDER_STATS
#elif defined (HEART_RELEASE)
#endif

#ifndef HEART_USE_REFLECTION
#   define HEART_USE_REFLECTION
#endif

#define RAPIDXML_NO_EXCEPTIONS

#ifdef HEART_NO_FORCE_INLINE
#   define hForceInline
#else
#   ifdef HEART_PLAT_WINDOWS
#   	define hForceInline   __forceinline
#	elif defined (HEART_PLAT_LINUX)
#		define hForceInline   __inline__
#   else
#      error "Platform not supported"
#   endif
#endif

#define hGetMurmurHashSeed() (12345)

#ifdef HEART_PLAT_WINDOWS
#   define hputenv _putenv
#   define hgetenv getenv
#else
#   define hputenv putenv
#   define hgetenv getenv
#endif

#endif // _HHEARTCONFIG_H__
