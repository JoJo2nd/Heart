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

/*
    Test these by including them first
    They need to be able to be included singularly
*/
#include "network/hNetDataStructs.h"

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
#include <queue>
#include <vector>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <math.h>
#include <float.h>

#ifdef USING_LIB_PROF
#   include "libprof.h"
#   define heart_thread_prof_begin(filename)        lpf_begin_thread_profile(filename)
#   define heart_thread_prof_end(discard)           lpf_end_thread_profile(discard)
#   define heart_thread_prof_is_running()           lpf_is_thread_profiling()
#else
#   define heart_thread_prof_begin(filename)
#   define heart_thread_prof_end(discard)
#   define heart_thread_prof_is_running()
#endif

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
// enet includes /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "enet/enet.h"

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
/// Proto buffer classes /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*
    Proto buffers spits out a lot of warnings about 64 bit to 32 bit conversions.
    We disable these warnings just around these sections.
*/
#if defined (_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4244)
#   pragma warning(disable:4267)
#else
#   pragma error ("Unknown platform")
#endif

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"

#include "debug_server_common.pb.h"
#include "package.pb.h"
#include "resource_common.pb.h"
#include "resource_shader.pb.h"
#include "resource_texture.pb.h"
#include "resource_material_fx.pb.h"
#include "resource_mesh.pb.h"


#if defined (_MSC_VER)
#   pragma warning(pop)
#endif

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
#include "threading/hJobManager.h"
#include "utils/hRapidXML.h"
#include "core/hConfigOptions.h"
#include "core/hIFileSystem.h"
#include "core/hIFile.h"
#include "core/hDriveFileSystem.h"
#include "core/hDriveFile.h"
#include "core/hResourceStreamHelpers.h"
#include "core/hResource.h"
#include "core/hResourcePackage.h"
#include "core/hResourceManager.h"
#include "core/hSystem.h"


#include "math/hMathUtil.h"
#include "math/hPlane.h"
#include "math/hPoly2.h"
#include "math/hAABB.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Debug //////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// network ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "network/hNetwork.h"
#include "network/hNetHost.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Input //////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "input/hActionManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Lua ////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "lua/hLuaStateManager.h"

//////////////////////////////////////////////////////////////////////////
// World objects /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "components/hEntity.h"
#include "components/hObjectFactory.h"

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
#include "utils/hTextFormatting.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "core/hSystemConsole.h"

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