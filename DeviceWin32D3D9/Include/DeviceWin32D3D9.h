/********************************************************************

	filename: 	DeviceWin32D3D9.h	
	
	Copyright (c) 24:7:2011 James Moran
	
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
#ifndef DEVICEWIN32D3D9_H__
#define DEVICEWIN32D3D9_H__

//////////////////////////////////////////////////////////////////////////
// Common Includes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "hHeartConfig.h"

//////////////////////////////////////////////////////////////////////////
// Platform Includes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "DeviceWin32.h"

//////////////////////////////////////////////////////////////////////////
// Base Heart Includes ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "hTypes.h"
#include "hDebugMacros.h"
#include "hMemory.h"
#include "hEventManager.h"
#include "hAtomic.h"
#include "hRendererConstants.h"

//////////////////////////////////////////////////////////////////////////
// Device Includes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "Direct3D.h"
#include "DeviceD3D9VtxDecl.h"
#include "DeviceD3D9IndexBuffer.h"
#include "DeviceD3D9VtxBuffer.h"
#include "DeviceD3D9Texture.h"
#include "DeviceD3D9Material.h"
#include "DeviceD3D9Renderer.h"

//////////////////////////////////////////////////////////////////////////
// Typedef to common names ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

typedef Heart::hdD3D9Renderer		hdRenderer;
typedef Heart::hdD3D9VtxDecl		hdVtxDecl;
typedef Heart::hdD3D9VtxBuffer		hdVtxBuffer;
typedef Heart::hdD3D9IndexBuffer	hdIndexBuffer;
typedef Heart::hdD3D9Texture		hdTexture;
typedef Heart::hdD3D9Material		hdMaterial;

#endif // DEVICEWIN32D3D9_H__