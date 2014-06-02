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
// Platform Includes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include <winsock2.h>
#include <windows.h>
#include <D3D11.h>
#include <d3dcompiler.h>
#if defined (HEART_USE_DXSDK)
#   include <D3DX11.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Device Includes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "pal/dx11/hDX11View.h"
#include "pal/dx11/hDX11VertexLayout.h"
#include "pal/dx11/hDX11ComputeTypes.h"
#include "pal/dx11/hDX11ShaderProgram.h"
#include "pal/dx11/hDX11IndexBuffer.h"
#include "pal/dx11/hDX11VertexBuffer.h"
#include "pal/dx11/hDX11Texture.h"
#include "pal/dx11/hDX11RenderStateBlock.h"
#include "pal/dx11/hDX11RenderSubmissionCtx.h"
#include "pal/dx11/hDX11RenderDevice.h"

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

    typedef hdDX11RenderDevice           hdRenderDevice;
    typedef hdDX11RenderSubmissionCtx    hdRenderSubmissionCtx;
    typedef hdDX11RenderCommandGenerator hdRenderCommandGenerator;
    typedef hdDX11VertexLayout           hdInputLayout;
    typedef hdDX11VertexBuffer           hdVtxBuffer;
    typedef hdDX11IndexBuffer            hdIndexBuffer;
    typedef hdDX11Texture                hdTexture;
    typedef hdDX11ShaderProgram          hdShaderProgram;
    typedef hdDX11Buffer                 hdRenderBuffer;
    typedef hdDX11BlendState             hdBlendState;
    typedef hdDX11RasterizerState        hdRasterizerState;
    typedef hdDX11DepthStencilState      hdDepthStencilState;
    typedef hdDX11InputState             hdInputState;
    typedef hdDX11SamplerState           hdSamplerState;
    typedef hdDX11MappedResourceData     hdMappedData;
    typedef hdDX11CommandBuffer          hdRenderCommandBuffer;
    typedef hdDX11ComputeUAV             hdComputeUAV;
    typedef hdDX11ShaderResourceView     hdShaderResourceView;
    typedef hdDX11DepthStencilView       hdDepthStencilView;
    typedef hdDX11RenderTargetView       hdRenderTargetView;
}


#endif // DEVICEWIN32DX11_H__