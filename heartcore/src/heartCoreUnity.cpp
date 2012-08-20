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

#include "threading/hJobManager.cpp"

//////////////////////////////////////////////////////////////////////////
// Core //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "core/hMemoryHeap.cpp"
#include "core/hConfigOptions.cpp"
#include "core/hDriveFile.cpp"
#include "core/hDriveFileSystem.cpp"
#include "core/hResource.cpp"
#include "core/hResourceManager.cpp"
#include "core/hResourcePackage.cpp"
#include "core/hSystemConsole.cpp"
#include "core/hZipFile.cpp"
#include "core/hZipFileSystem.cpp"

#include "math/hCollision.cpp"

//////////////////////////////////////////////////////////////////////////
// Event Manager /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "events/hEventManager.cpp"

//////////////////////////////////////////////////////////////////////////
// logic /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "components/hWorldObjectScript.cpp"
#include "components/hComponent.cpp"
#include "components/hEntity.cpp"
#include "components/hEntityFactory.cpp"
#include "core/hStateBase.cpp"
#include "lua/hLuaHeartLib.cpp"
#include "lua/hLuaStateManager.cpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "core/hSystem.cpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Input //////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "input/hControllerManager.cpp"
#include "input/hController.cpp"

//////////////////////////////////////////////////////////////////////////
// Renderer //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "render/hIndexBuffer.cpp"
#include "render/hRenderer.cpp"
#include "render/hRendererCamera.cpp"
#include "render/hRendererConstants.cpp"
#include "render/hRenderMaterialManager.cpp"
//#include "hRenderState.cpp"
#include "render/hRenderSubmissionContext.cpp"
#include "render/hRenderUtility.cpp"
#include "render/hVertexBuffer.cpp"
#include "render/hDrawCallContext.cpp"

#include "math/hViewFrustum.cpp"

//////////////////////////////////////////////////////////////////////////
// Sound /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "audio/hSoundResource.cpp"
#include "audio/hSoundSource.cpp"
#include "audio/hStaticSoundResource.cpp"
#include "audio/hSoundManager.cpp"

//////////////////////////////////////////////////////////////////////////
// Instances ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "render/hFont.cpp"
#include "render/hMaterial.cpp"
//#include "hMesh.cpp"
#include "render/hTexture.cpp"

//////////////////////////////////////////////////////////////////////////
// UI ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "ui/hGwenRenderer.cpp"
#include "ui/hGwenInputBinder.cpp"

//////////////////////////////////////////////////////////////////////////
// utils /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "utils/hRapidXML.cpp"
#include "utils/hBuiltDataCache.cpp"

//////////////////////////////////////////////////////////////////////////
// components ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "components/hLuaScriptComponent.cpp"
#include "components/hSceneNodeComponent.cpp"

//////////////////////////////////////////////////////////////////////////
// Debug menus ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "debug/hDebugMenuManager.cpp"
#include "utils/hRealTimeProfilerMenu.cpp"

//////////////////////////////////////////////////////////////////////////
// engine object /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "core/hHeart.cpp"
