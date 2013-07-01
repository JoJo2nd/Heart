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

    struct HEART_DLLEXPORT hRCmdSetStates : public hRCmd 
    {
        hRCmdSetStates(ID3D11BlendState* bs, ID3D11RasterizerState* rs, ID3D11DepthStencilState* dss, hUint32 stencilRef) 
            : hRCmd(eRenderCmd_SetRenderStates, sizeof(hRCmdSetStates))
            , blendState_(bs), rasterState_(rs), depthState_(dss), stencilRef_(stencilRef) {}
        ID3D11BlendState*           blendState_;
        ID3D11RasterizerState*      rasterState_;
        ID3D11DepthStencilState*    depthState_;
        hUint32                     stencilRef_;
    };

    struct HEART_DLLEXPORT hRCmdSetVertexShader : public hRCmd
    {
        hRCmdSetVertexShader(ID3D11VertexShader* shader)
            : hRCmd(eRenderCmd_SetVertexShader, sizeof(hRCmdSetVertexShader)), shader_(shader) {}
        ID3D11VertexShader* shader_;
    };

    struct HEART_DLLEXPORT hRCmdSetPixelShader : public hRCmd
    {
        hRCmdSetPixelShader(ID3D11PixelShader* shader)
            : hRCmd(eRenderCmd_SetPixelShader, sizeof(hRCmdSetPixelShader)), shader_(shader) {}
        ID3D11PixelShader* shader_;
    };

    struct HEART_DLLEXPORT hRCmdSetGeometryShader : public hRCmd
    {
        hRCmdSetGeometryShader(ID3D11GeometryShader* shader)
            : hRCmd(eRenderCmd_SetGeometeryShader, sizeof(hRCmdSetGeometryShader)), shader_(shader) {}
        ID3D11GeometryShader* shader_;
    };

    struct HEART_DLLEXPORT hRCmdSetHullShader : public hRCmd
    {
        hRCmdSetHullShader(ID3D11HullShader* shader)
            : hRCmd(eRenderCmd_SetHullShader, sizeof(hRCmdSetHullShader)), shader_(shader) {}
        ID3D11HullShader* shader_;
    };

    struct HEART_DLLEXPORT hRCmdSetDomainShader : public hRCmd
    {
        hRCmdSetDomainShader(ID3D11DomainShader* shader)
            : hRCmd(eRenderCmd_SetDomainShader, sizeof(hRCmdSetDomainShader)), shader_(shader) {}
        ID3D11DomainShader* shader_;
    };

    struct HEART_DLLEXPORT hRCmdSetInputsBase : public hRCmd
    {
        hRCmdSetInputsBase(hRenderCmdOpCode oc, hUint rescount, hUint samcount, hUint bufcount)
            : hRCmd(oc, sizeof(hRCmdSetInputsBase))
            , resourceViewCount_(rescount), samplerCount_(samcount), bufferCount_(bufcount) {}
        hUint resourceViewCount_;
        hUint samplerCount_;
        hUint bufferCount_;
        // Followed by an array of the following
        //ID3D11ShaderResourceView* resourceViews_[resourceViewCount_];
        //ID3D11SamplerState*       samplerState_[samplerCount_];
        //ID3D11Buffer*             programInputs_[bufferCount_];
    };

    struct HEART_DLLEXPORT hRCmdSetVertexInputs     : public hRCmdSetInputsBase
    {
        hRCmdSetVertexInputs(hUint rescount, hUint samcount, hUint bufcount) 
            : hRCmdSetInputsBase(eRenderCmd_SetVertexInputs, rescount, samcount, bufcount) {}
    };
    struct HEART_DLLEXPORT hRCmdSetPixelInputs      : public hRCmdSetInputsBase
    {
        hRCmdSetPixelInputs(hUint rescount, hUint samcount, hUint bufcount) 
            : hRCmdSetInputsBase(eRenderCmd_SetPixelInputs, rescount, samcount, bufcount) {}
    };
    struct HEART_DLLEXPORT hRCmdSetGeometryInputs   : public hRCmdSetInputsBase
    {
        hRCmdSetGeometryInputs(hUint rescount, hUint samcount, hUint bufcount) 
            : hRCmdSetInputsBase(eRenderCmd_SetGeometryInputs, rescount, samcount, bufcount) {}
    };
    struct HEART_DLLEXPORT hRCmdSetHullInputs       : public hRCmdSetInputsBase
    {
        hRCmdSetHullInputs(hUint rescount, hUint samcount, hUint bufcount) 
            : hRCmdSetInputsBase(eRenderCmd_SetHullInputs, rescount, samcount, bufcount) {}
    };
    struct HEART_DLLEXPORT hRCmdSetDomainInputs     : public hRCmdSetInputsBase
    {
        hRCmdSetDomainInputs(hUint rescount, hUint samcount, hUint bufcount) 
            : hRCmdSetInputsBase(eRenderCmd_SetDomainInputs, rescount, samcount, bufcount) {}
    };

    struct HEART_DLLEXPORT hRCmdSetInputStreams : public hRCmd
    {
        hRCmdSetInputStreams(D3D11_PRIMITIVE_TOPOLOGY top, DXGI_FORMAT format, ID3D11InputLayout* lo, hUint16 fstream, hUint16 estream, ID3D11Buffer* index)
            : hRCmd(eRenderCmd_SetInputStreams, sizeof(hRCmdSetInputStreams))
            , topology_(top), indexFormat_(format), layout_(lo), firstStream_(fstream), lastStream_(estream), index_(index) {}
        D3D11_PRIMITIVE_TOPOLOGY topology_;
        DXGI_FORMAT         indexFormat_;
        ID3D11InputLayout*  layout_;
        hUint16             firstStream_;
        hUint16             lastStream_;
        ID3D11Buffer*       index_;
        // Followed by an array of 
        //ID3D11Buffer*       streams[lastStream-firstStream];
    };

    class HEART_DLLEXPORT hdDX11RenderCommandGenerator
    {
    public:
        hdDX11RenderCommandGenerator() {}
        ~hdDX11RenderCommandGenerator() {}

    protected:

        hUint setRenderStates(hdDX11BlendState* bs, hdDX11RasterizerState* rs, hdDX11DepthStencilState* dss);
        hUint setShader(hdDX11ShaderProgram* shader);
        hUint setVertexInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hdDX11ShaderResourceView** srv, hUint nsrv,
            hdDX11ParameterConstantBlock** cb, hUint ncb);
        hUint setPixelInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hdDX11ShaderResourceView** srv, hUint nsrv,
            hdDX11ParameterConstantBlock** cb, hUint ncb);
        hUint setGeometryInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hdDX11ShaderResourceView** srv, hUint nsrv,
            hdDX11ParameterConstantBlock** cb, hUint ncb);
        hUint setHullInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hdDX11ShaderResourceView** srv, hUint nsrv,
            hdDX11ParameterConstantBlock** cb, hUint ncb);
        hUint setDomainInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hdDX11ShaderResourceView** srv, hUint nsrv,
            hdDX11ParameterConstantBlock** cb, hUint ncb);
        hUint setStreamInputs(PrimitiveType primType, hdDX11IndexBuffer* index, hIndexBufferType format,
            hdDX11VertexLayout* layout, hdDX11VertexBuffer** vtx, hUint firstStream, hUint streamCount);
        
        virtual hRCmd* getCmdBufferStart() = 0;
        virtual hUint  appendCmd(const hRCmd* cmd) = 0;
        virtual void   reset() = 0;
    };

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