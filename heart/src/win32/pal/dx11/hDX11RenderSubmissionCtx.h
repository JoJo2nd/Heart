/********************************************************************

    filename: 	DeviceDX11RenderSubmissionCtx.h	
    
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

#ifndef DEVICEDX11RENDERSUBMISSIONCTX_H__
#define DEVICEDX11RENDERSUBMISSIONCTX_H__

namespace Heart
{
    struct hRCmdSetStates;
    struct hRCmdSetVertexShader;
    struct hRCmdSetPixelShader;
    struct hRCmdSetGeometryShader;
    struct hRCmdSetHullShader;
    struct hRCmdSetDomainShader;
    struct hRCmdSetVertexInputs;
    struct hRCmdSetPixelInputs;
    struct hRCmdSetGeometryInputs;
    struct hRCmdSetHullInputs;
    struct hRCmdSetDomainInputs;
    struct hRCmdSetInputStreams;

    typedef D3D11_MAPPED_SUBRESOURCE hdDX11MappedResourceData;
    typedef ID3D11CommandList* hdDX11CommandBuffer;

#pragma message ("TODO: Is hdComputeInputObject still needed or would hRenderCommands replace it?")
    class  hdDX11ComputeInputObject 
    {
    public:
        hdDX11ComputeInputObject() 
            : boundComputeProg_(NULL)
            , computeShader_(NULL)
            , resourceViewCount_(0)
            , samplerCount_(0)
            , constCount_(0)
            , uavCount_(0)
        {
            hZeroMem(resourceViews_, sizeof(resourceViews_));
            hZeroMem(samplerState_, sizeof(samplerState_));
            hZeroMem(programInputs_, sizeof(programInputs_));
            hZeroMem(unorderdAccessView_, sizeof(unorderdAccessView_));
        }
        hBool   bindShaderProgram(hdDX11ShaderProgram* prog);
        hBool   bindSamplerInput(hShaderParameterID paramID, hdDX11SamplerState* srv);
        hBool   bindResourceView(hShaderParameterID paramID, hdDX11ShaderResourceView* view);
        hBool   bindConstantBuffer(hShaderParameterID paramID, hdDX11Buffer* buffer);
        hBool   bindUAV(hShaderParameterID paramID, hdDX11ComputeUAV* uav);
    private:
        friend class hdDX11RenderSubmissionCtx;

        hdDX11ShaderProgram*       boundComputeProg_;
        ID3D11ComputeShader*       computeShader_;
        hUint16                    resourceViewCount_;
        hUint16                    samplerCount_;
        hUint16                    constCount_;
        hUint16                    uavCount_;
        ID3D11ShaderResourceView*  resourceViews_[HEART_MAX_RESOURCE_INPUTS];
        ID3D11SamplerState*        samplerState_[HEART_MAX_RESOURCE_INPUTS];
        ID3D11Buffer*              programInputs_[HEART_MAX_CONSTANT_BLOCKS];
        ID3D11UnorderedAccessView* unorderdAccessView_[HEART_MAX_UAV_INPUTS];
    };

    class  hdDX11RenderSubmissionCtx
    {
    public:
        hdDX11RenderSubmissionCtx() 
            : depthStencilView_(NULL)
        {
            for ( hUint32 i = 0; i < MAX_RENDERTARGE_VIEWS; ++i )
            {
                renderTargetViews_[i] = NULL;
            }
        }
        ~hdDX11RenderSubmissionCtx() 
        {}

        hdDX11CommandBuffer     SaveToCommandBuffer();
        void                    RunSubmissionBuffer(hdDX11CommandBuffer cmdBuf);

        void    setRenderStateBlock(hdDX11BlendState* st);
        void    setRenderStateBlock(hdDX11DepthStencilState* st);
        void    setRenderStateBlock(hdDX11RasterizerState* st);
        void    setRenderStates(hRCmdSetStates* cmd);
        void    setVertexShader(hRCmdSetVertexShader* prog);
        void    setVertexInputs(hRCmdSetVertexInputs* cmd);
        void    setPixelShader(hRCmdSetPixelShader* prog);
        void    setPixelInputs(hRCmdSetPixelInputs* cmd);
        void    setGeometryShader(hRCmdSetGeometryShader* prog);
        void    setGeometryInputs(hRCmdSetGeometryInputs* cmd);
        void    setHullShader(hRCmdSetHullShader* prog);
        void    setHullInputs(hRCmdSetHullInputs* cmd);
        void    setDomainShader(hRCmdSetDomainShader* prog);
        void    setDomainInputs(hRCmdSetDomainInputs* cmd);
        void    setInputStreams(hRCmdSetInputStreams* cmd);
        //void    setComputeShader(hRCmdSetComputeShader* prog);
        //void    setComputeInputs(hRCmdSetC)
        void    setComputeInput(const hdDX11ComputeInputObject* ci);
        void    setTargets(hUint32 idx, hdDX11RenderTargetView** target, hdDX11DepthStencilView* depth);
        void    SetViewport(const hViewport& viewport);
        void    SetScissorRect(const hScissorRect& scissor);
        void    clearColour(hdDX11RenderTargetView* view, const hColour& colour);
        void    clearDepth(hdDX11DepthStencilView* view, hFloat z);
        void    setViewPixel(hUint slot, hdDX11ShaderResourceView* buffer);
        void    DrawPrimitive(hUint32 nPrimatives, hUint32 startVertex );
        void    DrawPrimitiveInstanced(hUint instanceCount, hUint32 nPrimatives, hUint32 startVertex );
        void    DrawIndexedPrimitive(hUint32 nPrimatives, hUint32 startVertex);
        void    DrawIndexedPrimitiveInstanced(hUint instanceCount, hUint32 nPrims, hUint startVtx);
        void    dispatch(hUint x, hUint y, hUint z);
        void    Map(hdDX11Texture* tex, hUint32 level, hdDX11MappedResourceData* data);
        void    Unmap(hdDX11Texture* tex, hUint32 level, void* ptr);
        void    Map(hdDX11IndexBuffer* ib, hdDX11MappedResourceData* data);
        void    Unmap(hdDX11IndexBuffer* ib, void* ptr);
        void    Map(hdDX11VertexBuffer* vb, hdDX11MappedResourceData* data);
        void    Unmap(hdDX11VertexBuffer* vb, void* ptr);
        void    Map(hdDX11Buffer* vb, hdDX11MappedResourceData* data);
        void    Unmap(hdDX11Buffer* vb, void* ptr);
        void    appendRenderStats(hRenderFrameStats* outstats);
        void    resetStats();
        const hRenderFrameStats* getRenderStats() const { return &stats_; }

        void                    SetDeviceCtx( ID3D11DeviceContext* device, hTempRenderMemAlloc alloc, hTempRenderMemFree free ) { device_ = device; alloc_ = alloc; free_ = free; }
        ID3D11DeviceContext*    GetDeviceCtx() const { return device_; };

    private:

        static const hUint32    MAX_RENDERTARGE_VIEWS = 4;

        D3D11_PRIMITIVE_TOPOLOGY primType_;
        hTempRenderMemAlloc      alloc_;
        hTempRenderMemFree       free_;
        ID3D11RenderTargetView*  renderTargetViews_[MAX_RENDERTARGE_VIEWS];
        ID3D11DepthStencilView*  depthStencilView_;
        ID3D11DeviceContext*     device_;

        hRenderFrameStats        stats_;
    };
}


#endif // DEVICEDX11RENDERSUBMISSIONCTX_H__