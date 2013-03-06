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
    typedef D3D11_MAPPED_SUBRESOURCE hdDX11MappedResourceData;
    typedef ID3D11CommandList* hdDX11CommandBuffer;

    class HEART_DLLEXPORT hdDX11RenderInputObject
    {
    public:
        hdDX11RenderInputObject()
            : vertexShader_(NULL)
            , pixelShader_(NULL)
            , geometryShader_(NULL)
            , hullShader_(NULL)
            , domainShader_(NULL)
        {
            hZeroMem(boundProgs_, sizeof(boundProgs_));
            hZeroMem(inputData_, sizeof(inputData_));
        }

        hBool   BindShaderProgram(hdDX11ShaderProgram* prog);
        hBool   BindSamplerInput(hShaderParameterID paramID, hdDX11SamplerState* srv);
        hBool   BindResourceView(hShaderParameterID paramID, hdDX11Texture* view);
        hBool   BindConstantBuffer(hShaderParameterID paramID, hdDX11ParameterConstantBlock* buffer);

        enum hdDX11ShaderProgTypes
        {
            hdDX11VertexProg,
            hdDX11PixelProg,
            hdDX11GemoProg,
            hdDX11HullProg,
            hdDX11DomainProg,

            hdDX11ProgMax
        };

    private:
        friend class hdDX11RenderSubmissionCtx;

        hdDX11ShaderProgram*            boundProgs_[hdDX11ProgMax];
        ID3D11VertexShader*             vertexShader_;
        ID3D11PixelShader*              pixelShader_;
        ID3D11GeometryShader*           geometryShader_;
        ID3D11HullShader*               hullShader_;
        ID3D11DomainShader*             domainShader_;
        //More shaders to come...
        struct RendererInputs
        {
            hUint32                         resourceViewCount_;
            hUint32                         samplerCount_;
            ID3D11ShaderResourceView*       resourceViews_[HEART_MAX_RESOURCE_INPUTS];
            ID3D11SamplerState*             samplerState_[HEART_MAX_RESOURCE_INPUTS];
            ID3D11Buffer*                   programInputs_[HEART_MAX_CONSTANT_BLOCKS];
        } inputData_[hdDX11ProgMax]; //for inputData_[vertexShader] & inputData_[pixelShader]
    };

    class HEART_DLLEXPORT hdDX11ComputeInputObject 
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
        hBool   bindResourceView(hShaderParameterID paramID, hdDX11Texture* view);
        hBool   bindConstantBuffer(hShaderParameterID paramID, hdDX11ParameterConstantBlock* buffer);
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
        ID3D11UnorderedAccessView* unorderdAccessView_[HEART_MAX_RESOURCE_INPUTS];
    };

    class HEART_DLLEXPORT hdDX11RenderStreamsObject
    {
    public:
        hdDX11RenderStreamsObject()
            : indexFormat_(DXGI_FORMAT_R16_UINT)
            , index_(NULL)
            , layout_(NULL)
            , streamLower_(HEART_MAX_INPUT_STREAMS)
            , streamUpper_(0)
            , topology_(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
        {
            hZeroMem(streams_, sizeof(streams_));
            hZeroMem(strides_, sizeof(strides_));
            hZeroMem(boundStreams_, sizeof(boundStreams_));
        }

        hBool getHasIndexStream() const { return index_ != NULL; }
        void setPrimType(PrimitiveType primType);
        void bindIndexVertex(hdDX11IndexBuffer* index, hIndexBufferType format);
        void bindVertexStream(hUint16 stream, hdDX11VertexBuffer* vertexbuffer, hUint stride);
        void bindVertexFetch(hdDX11ShaderProgram* prog);
        void unbind(hdDX11ShaderProgram* prog);

    private:
        friend class hdDX11RenderSubmissionCtx;

        D3D11_PRIMITIVE_TOPOLOGY topology_;
        DXGI_FORMAT         indexFormat_;
        ID3D11InputLayout*  layout_;
        hUint16             streamLower_;
        hUint16             streamUpper_;
        hdDX11VertexBuffer* boundStreams_[HEART_MAX_INPUT_STREAMS];
        ID3D11Buffer*       index_;
        ID3D11Buffer*       streams_[HEART_MAX_INPUT_STREAMS];
        UINT                strides_[HEART_MAX_INPUT_STREAMS];
    };

    class HEART_DLLEXPORT hdDX11RenderSubmissionCtx
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

        void    SetInputStreams(const hdDX11RenderStreamsObject* streams);
        void    SetRenderInputObject(const hdDX11RenderInputObject* inputobj);
        void    SetRenderStateBlock(hdDX11BlendState* st);
        void    SetRenderStateBlock(hdDX11DepthStencilState* st);
        void    SetRenderStateBlock(hdDX11RasterizerState* st);
        void    setComputeInput(const hdDX11ComputeInputObject* ci);
        void    setTargets(hUint32 idx, hdDX11Texture** target, hdDX11Texture* depth);
        void    SetViewport(const hViewport& viewport);
        void    SetScissorRect(const hScissorRect& scissor);
        void    clearColour(hdDX11Texture* tex, const hColour& colour);
        void    clearDepth(hdDX11Texture* tex, hFloat z);
        void    DrawPrimitive(hUint32 nPrimatives, hUint32 startVertex );
        void    DrawIndexedPrimitive(hUint32 nPrimatives, hUint32 startVertex);
        void    DrawIndexedPrimitiveInstanced(hUint instanceCount, hUint32 nPrims, hUint startVtx);
        void    dispatch(hUint x, hUint y, hUint z);
        void    Map(hdDX11Texture* tex, hUint32 level, hdDX11MappedResourceData* data);
        void    Unmap(hdDX11Texture* tex, hUint32 level, void* ptr);
        void    Map(hdDX11IndexBuffer* ib, hdDX11MappedResourceData* data);
        void    Unmap(hdDX11IndexBuffer* ib, void* ptr);
        void    Map(hdDX11VertexBuffer* vb, hdDX11MappedResourceData* data);
        void    Unmap(hdDX11VertexBuffer* vb, void* ptr);
        void    Map(hdDX11ParameterConstantBlock* vb, hdDX11MappedResourceData* data);
        void    Unmap(hdDX11ParameterConstantBlock* vb, void* ptr);

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
    };
}


#endif // DEVICEDX11RENDERSUBMISSIONCTX_H__