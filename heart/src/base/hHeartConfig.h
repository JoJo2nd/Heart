/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#if defined (_DEBUG) || defined (DEBUG)
#   ifndef HEART_DEBUG
#       define HEART_DEBUG (1)
#   endif
#   ifndef HEART_RELEASE
#       define HEART_RELEASE (0)
#   endif
#else
#   ifndef HEART_DEBUG
#       define HEART_DEBUG (0)
#   endif
#   ifndef HEART_RELEASE
#       define HEART_RELEASE (1)
#   endif
#endif

#if HEART_DEBUG
#   define HEART_DO_ASSERTS (1)
#elif HEART_RELEASE
#   define HEART_DO_ASSERTS (0)
#endif

#define HEART_MIN_ALLOC_ALIGNMENT (16)

#ifndef USE_DL_PREFIX
#   define USE_DL_PREFIX
#endif

#ifndef ONLY_MSPACES
#   define ONLY_MSPACES 0
#endif

// To enable compile at load time of shaders.
#if HEART_DEBUG // or OpenGL when supported
#   define HEART_ALLOW_SHADER_SOURCE_COMPILE (1)
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

#if HEART_DEBUG
#   if !defined (HEART_TRACK_MEMORY_ALLOCS) && !defined (HEART_FORCE_DISABLE_TRACK_MEMORY_ALLOCS) && defined (HEART_PLAT_WINDOWS)
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
#   define hFORCEINLINE
#else
#   ifdef HEART_PLAT_WINDOWS
#   	define hFORCEINLINE   __forceinline
#	elif defined (HEART_PLAT_LINUX)
#		define hFORCEINLINE   __inline__
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


#ifdef HEART_PLAT_WINDOWS
#   define hSymPrefix "?"
#   if HEART_SHARED_LIB_EXPORTS
#       define HEART_EXPORT __declspec(dllexport)
#       define HEART_C_EXPORT extern "C" __declspec(dllexport)
#       define HEART_CLASS_EXPORT __declspec(dllexport)
#   else
#       define HEART_EXPORT __declspec(dllimport)
#       define HEART_C_EXPORT extern "C" __declspec(dllimport)
#       define HEART_CLASS_EXPORT __declspec(dllimport)
#   endif
#else
#   define hSymPrefix "_"
#   define HEART_EXPORT
#   define HEART_C_EXPORT extern "C" __declspec(dllexport)
#   define HEART_CLASS_EXPORT
#endif
