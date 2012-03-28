// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// shut the fucking compiler up [6/7/2008 James]
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP(SP1) or later.
#define WINVER 0x0502		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP(SP1) or later.                   
#define _WIN32_WINNT 0x0502	// Change this to the appropriate value to target other versions of Windows.
#endif						

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

//Heart stuff
#include "hHeartConfig.h"
#include "hTypes.h"
#include "hMemory.h"
//#include "HeartSTL.h"

#ifdef HEART_PLAT_WINDOWS
    #ifdef HEART_PLAT_D3D9
        #include "DeviceWin32D3D9.h"
    #elif HEART_PLAT_DX11
        #include "DeviceWin32DX11.h"
    #elif defined (HEART_PLAT_OPENGL)
        #include "DeviceWin32OpenGL.h"
    #endif
#endif

// C RunTime Header Files
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#ifndef __COMMON_H__
#define __COMMON_H__

#define BEGIN_FLAGS() union{ hUint32 allFlags_; struct	{

#define HEART_FLAG( x )		hBool ##x :  1

#define END_FLAGS() }; };

#endif //__COMMON_H__

// TODO: reference additional headers your program requires here
