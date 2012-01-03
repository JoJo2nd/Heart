/********************************************************************

	filename: 	DeviceDX11RenderDevice.h	
	
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

#ifndef DEVICEDX11RENDERDEVICE_H__
#define DEVICEDX11RENDERDEVICE_H__

namespace Heart
{
    class hdDX11RenderSubmissionCtx;
    class hdDX11Texture;
    class hdDX11ShaderProgram;

    class hdDX11RenderDevice
    {
    public:
        hdDX11RenderDevice();
        ~hdDX11RenderDevice();

        void    Create( Device::Kernel* sysHandle, hUint32 width, hUint32 height, hUint32 bbp, hFloat shaderVersion, hBool fullscreen, hBool vsync );
        void    Destroy();
        void    ActiveContext() {}
        void    BeginRender();
        void    EndRender();
        void	SwapBuffers();
        void    RunSubmissionCtx( hdDX11RenderSubmissionCtx* subCtx );
        hdDX11RenderSubmissionCtx* GetMainSubmissionCtx() { return &mainRenderCtx_; };
        
        hUint32 Width() const { return width_; }
        hUint32 Height() const { return height_; }

        //Resource Create Calls
        hdDX11ShaderProgram*        CompileShader( const hChar* shaderProg, hUint32 len, ShaderType type );
        hdDX11Texture*              CreateTextrue( hUint32 width, hUint32 height, hUint32 levels, TextureFormat format, void* initialData, hUint32 initDataSize );
        void                        DestroyTexture( hdDX11Texture* texture );
        hdDX11BlendState*           CreateBlendState( const hBlendStateDesc& desc );
        void                        DestroyBlendState( hdDX11BlendState* state );
        hdDX11RasterizerState*      CreateRasterizerState( const hRasterizerStateDesc& desc );
        void                        DestoryRasterizerState( hdDX11RasterizerState* state );
        hdDX11DepthStencilState*    CreateDepthStencilState( const hDepthStencilStateDesc& desc );
        void                        DestoryDepthStencilState( hdDX11DepthStencilState* state );
        hdDX11SamplerState*         CreateSamplerState( const hSamplerStateDesc& desc );
        void                        DestroySamplerState( hdDX11SamplerState* state );

    private:

        typedef hMap< hUint32, hdDX11BlendState >         BlendStateMapType;
        typedef hMap< hUint32, hdDX11RasterizerState >    RasterizerStateMapType;
        typedef hMap< hUint32, hdDX11DepthStencilState >  DepthStencilStateMapType;
        typedef hMap< hUint32, hdDX11SamplerState >       SamplerStateMapType;

        Device::Kernel*             kernel_;
        hUint32                     width_;
        hUint32                     height_;
        hdDX11RenderSubmissionCtx   mainRenderCtx_;
        IDXGISwapChain*             mainSwapChain_;
        ID3D11Device*               d3d11Device_;
        ID3D11DeviceContext*        mainDeviceCtx_;
        D3D_FEATURE_LEVEL           featureLevel_;
        ID3D11RenderTargetView*     renderTargetView_;
        ID3D11Texture2D*            depthStencil_;
        ID3D11DepthStencilView*     depthStencilView_;
        hMutex                      resourceMutex_;
        BlendStateMapType           blendStates_;
        RasterizerStateMapType      rasterizerStates_;
        DepthStencilStateMapType    depthStencilStates_;
        SamplerStateMapType         samplerStateMap_;
    };

}

#endif // DEVICEDX11RENDERDEVICE_H__