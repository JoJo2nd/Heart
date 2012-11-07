/********************************************************************

	filename: 	shaderloader.h	
	
	Copyright (c) 31:7:2012 James Moran
	
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
#ifndef SHADERLOADER_H__
#define SHADERLOADER_H__

#include <stdio.h>
#include <vector>
#include "Heart.h"

#if defined (SHADER_LOADER_COMPILE_DLL)
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

extern "C"
{
    DLL_EXPORT
    Heart::hResourceClassBase* HEART_API HeartBinLoader(Heart::hISerialiseStream*, Heart::hIDataParameterSet*, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine*);
    DLL_EXPORT
    hBool HEART_API HeartDataCompiler(Heart::hIDataCacheFile* inFile, Heart::hIBuiltDataCache* fileCache, Heart::hIDataParameterSet* params, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine, Heart::hISerialiseStream* binoutput);
    DLL_EXPORT
    hBool HEART_API HeartPackageLink(Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine);
    DLL_EXPORT
    void HEART_API HeartPackageUnlink(Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine);
    DLL_EXPORT
    void HEART_API HeartPackageUnload(Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine);
};

#endif // SHADERLOADER_H__