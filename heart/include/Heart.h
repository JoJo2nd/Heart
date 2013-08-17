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
#include "base/hHeartConfig.h"

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
/// lib vorbis/ogg ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

//////////////////////////////////////////////////////////////////////////
// Base //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "HeartBase.h"

#ifdef HEART_PLAT_WINDOWS
    #include "device/hWin32.h"
#endif

#include "utils/hBase64.h"

//////////////////////////////////////////////////////////////////////////
// Threading /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "threading/hLocklessComPipe.h"
#include "threading/hJobManager.h"
#include "threading/hMutexAutoScope.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Events /////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "events/hEventManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Core ///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "core/hMemoryHeap.h"
#include "core/hStackMemoryHeap.h"
#include "core/hTypePoolMemoryHeap.h"
#include "core/hDeferredReturn.h"
#include "utils/hRapidXML.h"
#include "core/hConfigOptions.h"
#include "core/hIFileSystem.h"
#include "core/hIFile.h"
#include "core/hSerialiserFileStream.h"
#include "core/hDriveFileSystem.h"
#include "core/hDriveFile.h"
#include "core/hZipFile.h"
#include "core/hZipFileSystem.h"
#include "core/hResource.h"
#include "core/hResourcePackage.h"
#include "core/hResourceManager.h"
#include "core/hStateBase.h"
#include "core/hSystem.h"


#include "math/hMathUtil.h"
#include "math/hPlane.h"
#include "math/hPoly2.h"
#include "math/hAABB.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Debug //////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Input //////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "input/hController.h"
#include "input/hControllerManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Lua ////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "lua/hLuaStateManager.h"

//////////////////////////////////////////////////////////////////////////
// World objects /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "components/hComponent.h"
#include "components/hEntityDataDefinition.h"
#include "components/hEntity.h"
#include "components/hWorldObjectScript.h"
#include "components/hEntityFactory.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Render /////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//#include "hRenderState.h"
#include "render/hRenderStateBlock.h"
#include "compute/hComputeUAV.h"
#include "render/hViews.h"
#include "render/hVertexBuffer.h"
#include "render/hIndexBuffer.h"
#include "math/hViewFrustum.h"
#include "render/hTexture.h"
#include "render/hParameterConstBlock.h"
#include "render/hRenderShaderProgram.h"
#include "render/hCompute.h"
#include "render/hMaterialTypes.h"
#include "resourcedata/hMaterialResourceData.h"
#include "render/hMaterial.h"
#include "render/hRenderSubmissionContext.h"
#include "render/hRenderMaterialManager.h"
#include "render/hRendererCamera.h"
#include "render/hDebugDraw.h"
#include "render/hRenderer.h"
#include "render/hDrawCallContext.h"
#include "render/hFont.h"
//#include "hMesh.h"
#include "render/hRenderUtility.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "render/hRenderable.h"
#include "render/hGeometryObject.h"
#include "render/hRenderModel.h"

/////////////////////////////////////////////////////////////////////////
// Scene ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#include "scene/hSceneBundle.h"


//////////////////////////////////////////////////////////////////////////
/// Sound ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "audio/hSoundSourceBuffer.h"
#include "audio/hSoundResource.h"
#include "audio/hSoundSource.h"
#include "audio/hSoundManager.h"
#include "audio/hStaticSoundResource.h"

//////////////////////////////////////////////////////////////////////////
// UI ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Debug menus ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "debug/hDebugMenuManager.h"
#include "debug/hDebugInfo.h"
#include "utils/hRealTimeProfilerMenu.h"
#include "utils/hMemoryViewMenu.h"
#include "utils/hTextFormatting.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "core/hSystemConsole.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "components/hLuaScriptComponent.h"
#include "components/hSceneNodeComponent.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "scene/hSceneDefinition.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Resource Structures ////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "resourcedata/hTextureResourceData.h"
#include "resourcedata/hShaderResourceData.h"
#include "resourcedata/hMeshResourceData.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "core/hHeart.h"

#endif // _HEART_H__