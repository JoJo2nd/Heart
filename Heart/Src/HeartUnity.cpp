/********************************************************************

	filename: 	HeartUnity.cpp	
	
	Copyright (c) 31:3:2012 James Moran
	
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

#include "Heart.h"

#ifdef HEART_PLAT_WINDOWS
    #include "DeviceWin32/DeviceWin32Unity.cpp"
    #ifdef HEART_PLAT_D3D9
        #include "DeviceWin32/DX9/DeviceWin32D3D9Unity.cpp"
    #elif HEART_PLAT_DX11
        #include "DeviceWin32/DX11/DeviceWin32DX11Unity.cpp"
    #elif defined (HEART_PLAT_OPENGL)
        #include "DeviceWin32/GL/DeviceWin32OpenGLUnity.cpp"
    #endif
#endif

//////////////////////////////////////////////////////////////////////////
// Base //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "hCRC32.cpp"
#include "hDebugMacros.cpp"

//////////////////////////////////////////////////////////////////////////
// Core //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "hClock.cpp"
#include "hConfigOptions.cpp"
#include "hDriveFile.cpp"
#include "hDriveFileSystem.cpp"
#include "hJobManager.cpp"
#include "hResource.cpp"
#include "hResourceManager.cpp"
#include "hResourcePackage.cpp"
#include "hSystemConsole.cpp"
#include "hZipFile.cpp"
#include "hZipFileSystem.cpp"
#include "hCollision.cpp"

//////////////////////////////////////////////////////////////////////////
// Debug /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "hDebugRenderer.cpp"
#include "hProfiler.cpp"

//////////////////////////////////////////////////////////////////////////
// Event Manager /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "hEventManager.cpp"

//////////////////////////////////////////////////////////////////////////
// logic /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "hWorldObjectScript.cpp"
#include "hComponent.cpp"
#include "hEntity.cpp"
#include "hEntityFactory.cpp"
#include "hStateBase.cpp"
#include "hLuaHeartLib.cpp"
#include "hLuaStateManager.cpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "hSystem.cpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Input //////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "hControllerManager.cpp"
#include "hKeyboard.cpp"
#include "hController.cpp"

//////////////////////////////////////////////////////////////////////////
// Renderer //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "hIndexBuffer.cpp"
#include "hRenderer.cpp"
#include "hRendererCamera.cpp"
#include "hRendererConstants.cpp"
#include "hRenderMaterialManager.cpp"
//#include "hRenderState.cpp"
#include "hRenderSubmissionContext.cpp"
#include "hRenderUtility.cpp"
#include "hVertexBuffer.cpp"
#include "hViewFrustum.cpp"

//////////////////////////////////////////////////////////////////////////
// Sound /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "hSoundResource.cpp"
#include "hSoundSource.cpp"
#include "hStaticSoundResource.cpp"
#include "hSoundManager.cpp"

//////////////////////////////////////////////////////////////////////////
// Instances ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "hFont.cpp"
#include "hMaterial.cpp"
#include "hMesh.cpp"
#include "hTexture.cpp"

//////////////////////////////////////////////////////////////////////////
// scene graph: to remove...//////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "hSceneGraph.cpp"
#include "hSceneNodeBase.cpp"
#include "hDeferredRenderVisitor.cpp"
#include "hSceneGraphUpdateVisitor.cpp"
#include "hShadowMapVisitor.cpp"
#include "hSceneNodeCamera.cpp"
#include "hSceneNodeLocator.cpp"
#include "hSceneNodeMesh.cpp"
#include "hSceneNodeLight.cpp"

//////////////////////////////////////////////////////////////////////////
// utils /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "hRapidXML.cpp"
#include "huMiniDumper.cpp"

//////////////////////////////////////////////////////////////////////////
// components ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "hLuaScriptComponent.cpp"

//////////////////////////////////////////////////////////////////////////
// engine object /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "hHeart.cpp"
