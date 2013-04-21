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
    hFUNCTOR_TYPEDEF(void(*)(hUint width, hUint height), hDeviceResizeCallback);

    struct HEART_DLLEXPORT hRenderDeviceSetup
    {
        hTempRenderMemAlloc     alloc_;
        hTempRenderMemFree      free_;
        hdDX11Texture*          depthBufferTex_;
    };

    class HEART_DLLEXPORT hdDX11RenderDevice
    {
    public:
        hdDX11RenderDevice();
        ~hdDX11RenderDevice();

        void                            Create(hdSystemWindow* sysHandle, hUint32 width, hUint32 height, hBool fullscreen, hBool vsync, hRenderDeviceSetup setup);
        void                            Destroy();
        void                            ActiveContext() {}
        void                            BeginRender(hFloat* gpuTime);
        void                            EndRender();
        void	                        SwapBuffers(hdDX11Texture* buffer);
        void                            InitialiseRenderSubmissionCtx( hdDX11RenderSubmissionCtx* ctx );
        void                            DestroyRenderSubmissionCtx( hdDX11RenderSubmissionCtx* ctx );
        void                            InitialiseMainRenderSubmissionCtx( hdDX11RenderSubmissionCtx* ctx );
        
        hUint32                         GetWidth() const { return width_; }
        hUint32                         GetHeight() const { return height_; }
        void                            setResizeCallback(hDeviceResizeCallback cb) { resizeCallback_ = cb; }
        hUint32                         computeVertexLayoutStride(hInputLayoutDesc* desc, hUint32 desccount);
        static hShaderProfile           getProfileFromString(const hChar* str);
        static const hChar*             getShaderProfileString(hShaderProfile profile) { hcAssert(profile < eShaderProfile_Max); return s_shaderProfileNames[profile]; }
        //Resource Create Calls
        hdDX11VertexLayout*             CreateVertexLayout(const hInputLayoutDesc* inputdesc, hUint32 desccount, const void* shaderProg, hUint32 progLen);
        void                            DestroyVertexLayout( hdDX11VertexLayout* layout );
        void                            ReleaseCommandBuffer( hdDX11CommandBuffer cmdBuf );
        void                            createComputeUAV(hdDX11Texture* res, hTextureFormat viewformat, hUint mip, hdDX11ComputeUAV* outres);
        void                            destroyComputeUAV(hdDX11ComputeUAV* uav);

        static void                     clearDeviceInputs(ID3D11DeviceContext* device);

    protected:

        const hChar*    getDebugShaderSource(hDebugShaderID shaderid);
        hdDX11ShaderProgram* compileShaderFromSourceDevice(hMemoryHeapBase* heap, const hChar* shaderProg, hSizeT len, const hChar* entry, hShaderProfile profile, hdDX11ShaderProgram* out);
        hdDX11ShaderProgram* compileShaderDevice(hMemoryHeapBase* heap, const hChar* shaderProg, hSizeT len, hShaderType type, hdDX11ShaderProgram* out);
        void                 destroyShaderDevice(hMemoryHeapBase* heap, hdDX11ShaderProgram* shaderProg);
        hdDX11Texture*  createTextureDevice(hUint32 levels, hTextureFormat format, hMipDesc* initialData, hUint32 flags, hdDX11Texture* texture);
        void            destroyTextureDevice( hdDX11Texture* texture );
        void            createVertexBufferDevice(hInputLayoutDesc* desc, hUint32 desccount, hUint32 sizeInBytes, void* initialData, hUint32 flags, hdDX11VertexBuffer* vtxBuf);
        void            destroyVertexBufferDevice(hdDX11VertexBuffer* indexBuffer);
        void            createIndexBufferDevice(hUint32 sizeInBytes, void* initialData, hUint32 flags, hdDX11IndexBuffer* idxBuf);
        void            destroyIndexBufferDevice(hdDX11IndexBuffer* indexBuffer);
        void            createShaderResourseViewDevice(hdDX11Texture* texture, const hShaderResourceViewDesc& desc, hdDX11ShaderResourceView* outsrv);
        void            createShaderResourseViewDevice(hdDX11ParameterConstantBlock* cb, const hShaderResourceViewDesc& desc, hdDX11ShaderResourceView* outsrv);
        void            destroyShaderResourceViewDevice(hdDX11ShaderResourceView* srv);
        void            createRenderTargetViewDevice(hdDX11Texture* texture, const hRenderTargetViewDesc& desc, hdDX11RenderTargetView* outrtv);
        void            destroyRenderTargetViewDevice(hdDX11RenderTargetView* outrtv);
        void            createDepthStencilViewDevice(hdDX11Texture* texture, const hDepthStencilViewDesc& desc, hdDX11DepthStencilView* outdsv);
        void            destroyDepthStencilViewDevice(hdDX11DepthStencilView* outdsv);
        void            createBlendStateDevice(const hBlendStateDesc& desc, hdDX11BlendState* out);
        void            destroyBlendStateDevice(hdDX11BlendState* state);
        void            createRasterizerStateDevice(const hRasterizerStateDesc& desc, hdDX11RasterizerState* out);
        void            destroyRasterizerStateDevice(hdDX11RasterizerState* state);
        void            createDepthStencilStateDevice(const hDepthStencilStateDesc& desc, hdDX11DepthStencilState* out);
        void            destroyDepthStencilStateDevice(hdDX11DepthStencilState* state);
        void            createSamplerStateDevice(const hSamplerStateDesc& desc, hdDX11SamplerState* out);
        void            destroySamplerStateDevice(hdDX11SamplerState* state);
        void            createConstantBlockDevice(hUint size, void* initdata, hdDX11ParameterConstantBlock* block);
        void            destroyConstantBlockDevice(hdDX11ParameterConstantBlock* constBlock);

    private:

        hUint32                     BuildVertexFormatArray(const hInputLayoutDesc* desc, hUint32 desccount, hUint32* stride, hUint32* fmtID, D3D11_INPUT_ELEMENT_DESC* elements);
        DXGI_FORMAT                 toDXGIFormat(hTextureFormat format, hBool* compressed);
        void                        createShaderResourseViewDevice(ID3D11Resource* texture, const hShaderResourceViewDesc& desc, hdDX11ShaderResourceView* outsrv);

        static const hChar*         s_shaderProfileNames[];

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
        //ID3D11Texture2D*            depthStencil_;
        //ID3D11DepthStencilView*     depthStencilView_;
        //hdDX11Texture*              depthBufferTex_;
        ID3D11Query*                timerDisjoint_;
        ID3D11Query*                timerFrameStart_;
        ID3D11Query*                timerFrameEnd_;
        hDeviceResizeCallback       resizeCallback_;

        ID3D11Texture2D*            pBackBuffer;
    };

}

#endif // DEVICEDX11RENDERDEVICE_H__