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
	#ifndef HEART_DEBUG
		#define HEART_DEBUG
	#endif
#else
	#ifndef HEART_RELEASE
		#define HEART_RELEASE
	#endif
#endif

#ifndef USE_DL_PREFIX
	#define USE_DL_PREFIX
#endif

#ifndef ONLY_MSPACES
	#define ONLY_MSPACES 1
#endif

//#define HEART_ALLOW_NEW

#ifndef XM_NO_OPERATOR_OVERLOADS
    #define XM_NO_OPERATOR_OVERLOADS
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef WIN32
	#define HEART_PLAT_WINDOWS
#endif

#ifdef HEART_DLL

    #ifdef HEART_BASE_DLL
        #define HEARTBASE_SLIBEXPORT  __declspec(dllexport)
    #else
        #define HEARTBASE_SLIBEXPORT  __declspec(dllimport)
    #endif

    #ifdef HEART_WIN32_DLL
        #define HEARTDEV_SLIBEXPORT  __declspec(dllexport)
    #else
        #define HEARTDEV_SLIBEXPORT  __declspec(dllimport)
    #endif

    #ifdef HEART_CORE_DLL
        #define HEARTCORE_SLIBEXPORT  __declspec(dllexport)
        #if !defined(GWEN_COMPILE_DLL)
            #define GWEN_COMPILE_DLL
        #endif
    #else
        #define HEARTCORE_SLIBEXPORT  __declspec(dllimport)
    #endif

    #if !defined (GWEN_COMPILE_DLL) && !defined(GWEN_DLL)
        #define GWEN_DLL
    #endif

    #define HEART_SLIB_TEXPORT __declspec(dllexport)

#else
    #define HEARTCORE_SLIBEXPORT 
    #define HEARTDEV_SLIBEXPORT
    #define HEARTBASE_SLIBEXPORT
#endif

#define HEART_BASECLASS(x) typedef x ParentClass;

#ifdef WIN32
    #define HEART_API   __cdecl
#else
    #error "Platform not supported"
#endif

#if defined (HEART_DEBUG)
	#ifdef HEART_PLAT_WINDOWS
		#ifndef D3D_DEBUG_INFO
			#define D3D_DEBUG_INFO
		#endif
	#endif // HEART_PLAT_WINDOWS
	#define HEART_COLLECT_RENDER_STATS
#elif defined (HEART_RELEASE)
#endif

#ifndef HEART_USE_REFLECTION
    #define HEART_USE_REFLECTION
#endif

#define RAPIDXML_NO_EXCEPTIONS

#define HEART_DO_PROFILE

#if defined (HEART_DEBUG) && !defined (HEART_DO_PROFILE)
    #define HEART_DO_PROFILE
#endif

#ifdef HEART_PACKER
    #define HEART_USE_DEFAULT_MEM_ALLOC
#endif

#ifndef HEART_FINAL
#   define HEART_ALLOW_DATA_COMPILE
#endif

#endif // _HHEARTCONFIG_H__
