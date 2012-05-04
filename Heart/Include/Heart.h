/********************************************************************

	filename: 	Heart.h
	
	Copyright (c) 2011/07/25 James Moran
	
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

#ifndef _HEART_H__
#define _HEART_H__

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hHeartConfig.h"

//////////////////////////////////////////////////////////////////////////
// stdlib ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <new>
#include <string>
#include <deque>
#include <math.h>
#include <float.h>

//////////////////////////////////////////////////////////////////////////
// zlib includes /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "zlib.h"

//////////////////////////////////////////////////////////////////////////
// minizip includes //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "zip.h"
#include "unzip.h"

//////////////////////////////////////////////////////////////////////////
// lua ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
extern "C"
{
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
};

//////////////////////////////////////////////////////////////////////////
// crypto ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "cryptoCRC32.h"
#include "cryptoMD5.h"

//////////////////////////////////////////////////////////////////////////
/// lib vorbis/ogg ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Base ///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hTypes.h"
#include "hDebugMacros.h"
#include "hCRC32.h"
#include "hMemoryDefines.h"
#include "hMemoryUtil.h"
#include "hStringUtil.h" 
#include "huFunctor.h"
#include "hTypeList.h"
#include "hReflection.h"
#include "hReflectionUtils.h"
#include "hSerialiser.h"
#include "hPtrImpl.h"
#include "hArray.h"
#include "hLinkedList.h"
#include "hMap.h"
#include "hStack.h"
#include "hQueue.h"
#include "hAtomic.h"
#include "hLocklessComPipe.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Events /////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hEventManager.h"

#ifdef HEART_PLAT_WINDOWS
    #include "DeviceWin32/DeviceWin32.h"
    #ifdef HEART_PLAT_D3D9
        #include "DeviceWin32/DX9/DeviceWin32D3D9.h"
    #elif HEART_PLAT_DX11
        #include "DeviceWin32/DX11/DeviceWin32DX11.h"
    #elif defined (HEART_PLAT_OPENGL)
        #include "DeviceWin32/GL/DeviceWin32OpenGL.h"
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Core ///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hMutex.h"
#include "hDeferredReturn.h"
#include "hIReferenceCounted.h"
#include "HeartSTL.h"
#include "hUTF8.h"
#include "hString.h"
#include "hThread.h"
#include "hSemaphore.h"
#include "hClock.h"
#include "hRapidXML.h"
#include "hConfigOptions.h"
#include "hIFileSystem.h"
#include "hIFile.h"
#include "hSerialiserFileStream.h"
#include "hDriveFileSystem.h"
#include "hDriveFile.h"
#include "hZipFile.h"
#include "hZipFileSystem.h"
#include "hJobManager.h"
#include "hResource.h"
#include "hResourcePackage.h"
#include "hResourceManager.h"
#include "hSemaphore.h"
#include "hThread.h"
#include "hStateBase.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// OS /////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hSystem.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Math ///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <xnamath.h>

#include "NumericalLimits.h"
#include "hMathUtil.h"
#include "hVector.h"
#include "hVec2.h"
#include "hVec3.h"
#include "hVec4.h"
#include "hQuaternion.h"
#include "hMatrix.h"
#include "hPlane.h"
#include "hPoly2.h"
#include "hAABB.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Debug //////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hDebugRenderer.h"
#include "huMiniDumper.h"
#include "hProfiler.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Input //////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hKeyboard.h"
#include "hController.h"
#include "hControllerManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Lua ////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hLuaStateManager.h"

//////////////////////////////////////////////////////////////////////////
// World objects /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "hComponent.h"
#include "hEntityDataDefinition.h"
#include "hEntity.h"
#include "hWorldObjectScript.h"
#include "hEntityFactory.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Render /////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hRendererConstants.h"
//#include "hRenderState.h"
#include "hVertexDeclarations.h"
#include "hVertexBuffer.h"
#include "hIndexBuffer.h"
#include "hViewFrustum.h"
#include "hTexture.h"
#include "hMaterial.h"
#include "hRenderSubmissionContext.h"
#include "hVertexDeclarations.h"
#include "hRenderMaterialManager.h"
#include "hRenderer.h"
#include "hRendererCamera.h"
#include "hFont.h"
#include "hMesh.h"
#include "hRenderUtility.h"

/////////////////////////////////////////////////////////////////////////
// Scene ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
#include "hSceneNodeBase.h"
#include "hSceneGraphVisitorBase.h"
#include "hDeferredRenderVisitor.h"
#include "hSceneGraphUpdateVisitor.h"
#include "hShadowMapVisitor.h"
#include "hSceneNodeCamera.h"
#include "hSceneNodeLocator.h"
#include "hSceneNodeMesh.h"
#include "hSceneNodeLight.h"
#include "hSceneGraph.h"
#include "hSceneBundle.h"

//////////////////////////////////////////////////////////////////////////
/// Sound ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "hSoundSourceBuffer.h"
#include "hSoundResource.h"
#include "hSoundSource.h"
#include "hSoundManager.h"
#include "hStaticSoundResource.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "hSystemConsole.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "hLuaScriptComponent.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "hHeart.h"

#endif // _HEART_H__