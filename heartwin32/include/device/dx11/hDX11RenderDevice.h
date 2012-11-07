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
    class hdDX11VertexBuffer;
    class hdDX11Texture;
    class hdDX11ShaderProgram;


    struct HEARTDEV_SLIBEXPORT hRenderDeviceSetup
    {
        hTempRenderMemAlloc     alloc_;
        hTempRenderMemFree      free_;
    };

    class HEARTDEV_SLIBEXPORT hdDX11RenderDevice
    {
    public:
        hdDX11RenderDevice();
        ~hdDX11RenderDevice();

        void                            Create(hdSystemWindow* sysHandle, hUint32 width, hUint32 height, hUint32 bbp, hFloat shaderVersion, hBool fullscreen, hBool vsync, hRenderDeviceSetup setup );
        void                            Destroy();
        void                            ActiveContext() {}
        void                            BeginRender(hFloat* gpuTime);
        void                            EndRender();
        void	                        SwapBuffers();
        void                            InitialiseRenderSubmissionCtx( hdDX11RenderSubmissionCtx* ctx );
        void                            DestroyRenderSubmissionCtx( hdDX11RenderSubmissionCtx* ctx );
        void                            InitialiseMainRenderSubmissionCtx( hdDX11RenderSubmissionCtx* ctx );
        hdDX11RenderSubmissionCtx*      GetMainSubmissionCtx() { return &mainRenderCtx_; };
        
        hUint32                         GetWidth() const { return width_; }
        hUint32                         GetHeight() const { return height_; }

        hUint32                         ComputeVertexLayoutStride(hInputLayoutDesc* desc, hUint32 desccount);
        //Resource Create Calls
        hdDX11ShaderProgram*            CompileShader( const hChar* shaderProg, hUint32 len, hInputLayoutDesc* inputLayout, hUint32 layoutCount, ShaderType type );
        void                            DestroyShader( hdDX11ShaderProgram* shaderProg );
        hdDX11ParameterConstantBlock*   CreateConstantBlocks( const hUint32* sizes, hUint32 count );
        void                            UpdateConstantBlockParameters( hdDX11ParameterConstantBlock* constBlock, hShaderParameter* params, hUint32 parameters );
        void                            DestroyConstantBlocks( hdDX11ParameterConstantBlock* constBlocks, hUint32 count );
        hdDX11Texture*                  CreateTextrue( hUint32 width, hUint32 height, hUint32 levels, hTextureFormat format, hMipDesc* initialData, hUint32 flags );
        void                            DestroyTexture( hdDX11Texture* texture );
        hdDX11IndexBuffer*              CreateIndexBufferDevice( hUint32 sizeInBytes, void* initialData, hUint32 flags );
        void                            DestroyIndexBufferDevice( hdDX11IndexBuffer* indexBuffer );
        hdDX11VertexLayout*             CreateVertexLayout(const hInputLayoutDesc* inputdesc, hUint32 desccount, const void* shaderProg, hUint32 progLen);
        void                            DestroyVertexLayout( hdDX11VertexLayout* layout );
        hdDX11VertexBuffer*             CreateVertexBufferDevice( hUint32 vertexLayout, hUint32 sizeInBytes, void* initialData, hUint32 flags );
        void                            DestroyVertexBufferDevice( hdDX11VertexBuffer* indexBuffer );
        hdDX11BlendState*               CreateBlendState( const hBlendStateDesc& desc );
        void                            DestroyBlendState( hdDX11BlendState* state );
        hdDX11RasterizerState*          CreateRasterizerState( const hRasterizerStateDesc& desc );
        void                            DestoryRasterizerState( hdDX11RasterizerState* state );
        hdDX11DepthStencilState*        CreateDepthStencilState( const hDepthStencilStateDesc& desc );
        void                            DestoryDepthStencilState( hdDX11DepthStencilState* state );
        hdDX11SamplerState*             CreateSamplerState( const hSamplerStateDesc& desc );
        void                            DestroySamplerState( hdDX11SamplerState* state );
        void                            ReleaseCommandBuffer( hdDX11CommandBuffer cmdBuf );

    private:

        typedef hMap< hUint32, hdDX11BlendState >         BlendStateMapType;
        typedef hMap< hUint32, hdDX11RasterizerState >    RasterizerStateMapType;
        typedef hMap< hUint32, hdDX11DepthStencilState >  DepthStencilStateMapType;
        typedef hMap< hUint32, hdDX11SamplerState >       SamplerStateMapType;
        typedef hMap< hUint32, hdDX11VertexLayout >       VertexLayoutMapType;

        hUint32                     BuildVertexFormatArray(const hInputLayoutDesc* desc, hUint32 desccount, hUint32* stride, hUint32* fmtID, D3D11_INPUT_ELEMENT_DESC* elements);

        hdSystemWindow*             sysWindow_;
        hUint64                     frameCounter_;
        hUint32                     width_;
        hUint32                     height_;
        hTempRenderMemAlloc         alloc_;
        hTempRenderMemFree          free_;
        hdDX11RenderSubmissionCtx   mainRenderCtx_;
        IDXGISwapChain*             mainSwapChain_;
        ID3D11Device*               d3d11Device_;
        ID3D11DeviceContext*        mainDeviceCtx_;
        D3D_FEATURE_LEVEL           featureLevel_;
        ID3D11RenderTargetView*     renderTargetView_;
        ID3D11Texture2D*            depthStencil_;
        ID3D11DepthStencilView*     depthStencilView_;
        hdW32Mutex                  resourceMutex_;
        BlendStateMapType           blendStates_;
        RasterizerStateMapType      rasterizerStates_;
        DepthStencilStateMapType    depthStencilStates_;
        SamplerStateMapType         samplerStateMap_;
        VertexLayoutMapType         vertexLayoutMap_;
        ID3D11Query*                timerDisjoint_;
        ID3D11Query*                timerFrameStart_;
        ID3D11Query*                timerFrameEnd_;

    };

}

#endif // DEVICEDX11RENDERDEVICE_H__