/********************************************************************

	filename: 	DeviceWin32DX11.h	
	
	Copyright (c) 18:12:2011 James Moran
	
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

#ifndef DEVICEWIN32DX11_H__
#define DEVICEWIN32DX11_H__

//////////////////////////////////////////////////////////////////////////
// Common Includes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "hHeartConfig.h"

//////////////////////////////////////////////////////////////////////////
// Platform Includes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <D3D11.h>
#include <D3DX11.h>
#include <d3dcompiler.h>
#include "DeviceWin32/DeviceWin32.h"

//////////////////////////////////////////////////////////////////////////
// Base Heart Includes ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "hTypes.h"
#include "hDebugMacros.h"
#include "hMemoryUtil.h"
#include "hEventManager.h"
#include "hAtomic.h"
#include "hRendererConstants.h"
#include "hMap.h"
#include "hIReferenceCounted.h"

//////////////////////////////////////////////////////////////////////////
// Device Includes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "DeviceWin32/DX11/DeviceDX11VertexLayout.h"
#include "DeviceWin32/DX11/DeviceDX11ShaderProgram.h"
#include "DeviceWin32/DX11/DeviceDX11IndexBuffer.h"
#include "DeviceWin32/DX11/DeviceDX11VertexBuffer.h"
#include "DeviceWin32/DX11/DeviceDX11Texture.h"
#include "DeviceWin32/DX11/DeviceDX11RenderStateBlock.h"
#include "DeviceWin32/DX11/DeviceDX11RenderSubmissionCtx.h"
#include "DeviceWin32/DX11/DeviceDX11RenderDevice.h"

//////////////////////////////////////////////////////////////////////////
// Typedef to common names ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace Heart 
{
    struct hdDX11PlaceholderType
    {
        void* Map() { return NULL; };
        void Unmap( void* ) {};

        void* Map( hUint32, hUint32* ) { return NULL; };
        void Unmap( hUint32, void* ) {};
    };

    struct hdDX11BlendState;
    struct hdDX11RasterizerState;
    struct hdDX11DepthStencilState;
    struct hdDX11InputState;
    struct hdDX11SamplerState;

    typedef hdDX11RenderDevice		    hdRenderDevice;
    typedef hdDX11RenderSubmissionCtx	hdRenderSubmissionCtx;
    typedef hdDX11VertexLayout   		hdVtxDecl;//make this include input state
    typedef hdDX11VertexBuffer	    	hdVtxBuffer;
    typedef hdDX11IndexBuffer    	    hdIndexBuffer;
    typedef hdDX11Texture		        hdTexture;
    typedef hdDX11ShaderProgram          hdShaderProgram;
    typedef hdDX11ParameterConstantBlock hdParameterConstantBlock;
    typedef hdDX11BlendState             hdBlendState;
    typedef hdDX11RasterizerState        hdRasterizerState;
    typedef hdDX11DepthStencilState      hdDepthStencilState;
    typedef hdDX11InputState             hdInputState;
    typedef hdDX11SamplerState           hdSamplerState;
    typedef hdDX11MappedResourceData     hdMappedData;
    typedef hdDX11CommandBuffer          hdRenderCommandBuffer;
}


#endif // DEVICEWIN32DX11_H__