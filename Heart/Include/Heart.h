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

#ifdef HEART_PLAT_WINDOWS
	#ifdef HEART_PLAT_D3D9
		#include "DeviceWin32D3D9.h"
    #elif HEART_PLAT_DX11
        #include "DeviceWin32DX11.h"
	#elif defined (HEART_PLAT_OPENGL)
		#include "DeviceWin32OpenGL.h"
	#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Core ///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hTypes.h"
#include "hMemoryUtil.h"
#include "hStringUtil.h"
#include "hUTF8.h"
#include "hClock.h"
#include "hConfigOptions.h"
#include "hDebugMacros.h"
#include "hReflectionUtils.h"
#include "hIFileSystem.h"
#include "hIFile.h"
#include "hDriveFileSystem.h"
#include "hDriveFile.h"
#include "hJobManager.h"
#include "hMalloc.h"
#include "hMemory.h"
#include "hMutex.h"
#include "hResource.h"
#include "hResourceManager.h"
#include "hResourcePackage.h"
#include "hSemaphore.h"
#include "hSystemConsole.h"
#include "hThread.h"
#include "hStateBase.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// OS /////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hAtomic.h"
#include "hSystem.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hCRC32.h"
#include "HeartSTL.h"
#include "hPtrImpl.h"
#include "hMap.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Math ///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hAABB.h"
#include "hMath.h"
#include "hMathUtil.h"
#include "hMatrix.h"
#include "hPlane.h"
#include "hPoly2.h"
#include "hQuaternion.h"
#include "hVector.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Debug //////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hDebugRenderer.h"
#include "hProfiler.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Events /////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hEventManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Input //////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hControllerManager.h"
#include "hKeyboard.h"
#include "hController.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Lua ////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hLuaStateManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Render /////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hRendererConstants.h"
#include "hRenderer.h"
#include "hRenderState.h"
#include "hVertexDeclarations.h"
#include "hVertexBuffer.h"
#include "hIndexBuffer.h"
#include "hRendererCamera.h"
#include "hViewFrustum.h"
#include "hTexture.h"
#include "hMesh.h"
#include "hMaterial.h"
#include "hFont.h"
#include "hRenderUtility.h"

/////////////////////////////////////////////////////////////////////////
// Scene ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
#include "hSceneGraph.h"
#include "hSceneNodeBase.h"
#include "hSceneGraphVisitorBase.h"
#include "hSceneBundle.h"
#include "hDeferredRenderVisitor.h"
#include "hSceneGraphUpdateVisitor.h"
#include "hShadowMapVisitor.h"
#include "hSceneNodeCamera.h"
#include "hSceneNodeLocator.h"
#include "hSceneNodeMesh.h"
#include "hSceneNodeLight.h"

//////////////////////////////////////////////////////////////////////////
/// Sound ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "hSoundResource.h"
#include "hSoundSource.h"
#include "hSoundManager.h"
#include "hStaticSoundResource.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "hHeart.h"

#endif // _HEART_H__