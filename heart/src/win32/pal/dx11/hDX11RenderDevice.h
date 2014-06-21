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

#include "base/hTypes.h"    
#include "base/hFunctor.h"

namespace Heart
{
    class hdSystemWindow;
    struct hShaderResourceViewDesc;

    hFUNCTOR_TYPEDEF(void(*)(hUint width, hUint height), hDeviceResizeCallback);

    struct hRCmdSetStates : public hRCmd 
    {
        hRCmdSetStates(ID3D11BlendState* bs, ID3D11RasterizerState* rs, ID3D11DepthStencilState* dss, hUint32 stencilRef) 
            : hRCmd(eRenderCmd_SetRenderStates, sizeof(hRCmdSetStates))
            , blendState_(bs), rasterState_(rs), depthState_(dss), stencilRef_(stencilRef) {}
        ID3D11BlendState*           blendState_;
        ID3D11RasterizerState*      rasterState_;
        ID3D11DepthStencilState*    depthState_;
        hUint32                     stencilRef_;
    };

    struct hRCmdSetVertexShader : public hRCmd
    {
        hRCmdSetVertexShader(ID3D11VertexShader* shader)
            : hRCmd(eRenderCmd_SetVertexShader, sizeof(hRCmdSetVertexShader)), shader_(shader) {}
        ID3D11VertexShader* shader_;
    };

    struct hRCmdSetPixelShader : public hRCmd
    {
        hRCmdSetPixelShader(ID3D11PixelShader* shader)
            : hRCmd(eRenderCmd_SetPixelShader, sizeof(hRCmdSetPixelShader)), shader_(shader) {}
        ID3D11PixelShader* shader_;
    };

    struct hRCmdSetGeometryShader : public hRCmd
    {
        hRCmdSetGeometryShader(ID3D11GeometryShader* shader)
            : hRCmd(eRenderCmd_SetGeometeryShader, sizeof(hRCmdSetGeometryShader)), shader_(shader) {}
        ID3D11GeometryShader* shader_;
    };

    struct hRCmdSetHullShader : public hRCmd
    {
        hRCmdSetHullShader(ID3D11HullShader* shader)
            : hRCmd(eRenderCmd_SetHullShader, sizeof(hRCmdSetHullShader)), shader_(shader) {}
        ID3D11HullShader* shader_;
    };

    struct hRCmdSetDomainShader : public hRCmd
    {
        hRCmdSetDomainShader(ID3D11DomainShader* shader)
            : hRCmd(eRenderCmd_SetDomainShader, sizeof(hRCmdSetDomainShader)), shader_(shader) {}
        ID3D11DomainShader* shader_;
    };

    struct hRCmdSetInputsBase : public hRCmd
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

    struct hRCmdSetVertexInputs     : public hRCmdSetInputsBase
    {
        hRCmdSetVertexInputs(hUint rescount, hUint samcount, hUint bufcount) 
            : hRCmdSetInputsBase(eRenderCmd_SetVertexInputs, rescount, samcount, bufcount) {}
    };
    struct hRCmdSetPixelInputs      : public hRCmdSetInputsBase
    {
        hRCmdSetPixelInputs(hUint rescount, hUint samcount, hUint bufcount) 
            : hRCmdSetInputsBase(eRenderCmd_SetPixelInputs, rescount, samcount, bufcount) {}
    };
    struct hRCmdSetGeometryInputs   : public hRCmdSetInputsBase
    {
        hRCmdSetGeometryInputs(hUint rescount, hUint samcount, hUint bufcount) 
            : hRCmdSetInputsBase(eRenderCmd_SetGeometryInputs, rescount, samcount, bufcount) {}
    };
    struct hRCmdSetHullInputs       : public hRCmdSetInputsBase
    {
        hRCmdSetHullInputs(hUint rescount, hUint samcount, hUint bufcount) 
            : hRCmdSetInputsBase(eRenderCmd_SetHullInputs, rescount, samcount, bufcount) {}
    };
    struct hRCmdSetDomainInputs     : public hRCmdSetInputsBase
    {
        hRCmdSetDomainInputs(hUint rescount, hUint samcount, hUint bufcount) 
            : hRCmdSetInputsBase(eRenderCmd_SetDomainInputs, rescount, samcount, bufcount) {}
    };

    struct hRCmdSetInputStreams : public hRCmd
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

    class hdDX11RenderCommandGenerator
    {
    public:
        hdDX11RenderCommandGenerator() {}
        ~hdDX11RenderCommandGenerator() {}

    protected:

        hUint setRenderStates(hdDX11BlendState* bs, hdDX11RasterizerState* rs, hdDX11DepthStencilState* dss);
        hUint setShader(hdDX11ShaderProgram* shader, hShaderType type);
        hUint setVertexInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hdDX11ShaderResourceView** srv, hUint nsrv,
            hdDX11Buffer** cb, hUint ncb);
        hUint updateVertexInputs(hRCmd* oldcmd, hdDX11SamplerState** samplers, hUint nsamplers,
            hdDX11ShaderResourceView** srv, hUint nsrv,
            hdDX11Buffer** cb, hUint ncb);
        hUint setPixelInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hdDX11ShaderResourceView** srv, hUint nsrv,
            hdDX11Buffer** cb, hUint ncb);
        hUint updatePixelInputs(hRCmd* oldcmd, hdDX11SamplerState** samplers, hUint nsamplers,
            hdDX11ShaderResourceView** srv, hUint nsrv,
            hdDX11Buffer** cb, hUint ncb);
        hUint setGeometryInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hdDX11ShaderResourceView** srv, hUint nsrv,
            hdDX11Buffer** cb, hUint ncb);
        hUint setHullInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hdDX11ShaderResourceView** srv, hUint nsrv,
            hdDX11Buffer** cb, hUint ncb);
        hUint setDomainInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hdDX11ShaderResourceView** srv, hUint nsrv,
            hdDX11Buffer** cb, hUint ncb);
        hUint setStreamInputs(PrimitiveType primType, hdDX11IndexBuffer* index, hIndexBufferType format,
            hdDX11VertexLayout* layout, hdDX11VertexBuffer** vtx, hUint firstStream, hUint streamCount);
        hUint updateShaderInputBuffer(hRCmd* cmd, hUint reg, hdDX11Buffer* cb);
        hUint updateShaderInputSampler(hRCmd* cmd, hUint reg, hdDX11SamplerState* ss);
        hUint updateShaderInputView(hRCmd* cmd, hUint reg, hdDX11ShaderResourceView* srv);
        hUint updateStreamInputs(hRCmd* cmd, PrimitiveType primType, hdDX11IndexBuffer* index, hIndexBufferType format,
            hdDX11VertexLayout* layout, hdDX11VertexBuffer** vtx, hUint firstStream, hUint streamCount);
        
        virtual hRCmd* getCmdBufferStart() = 0;
        virtual hUint  appendCmd(const hRCmd* cmd) = 0;
        virtual hUint  overwriteCmd(const hRCmd* oldcmd, const hRCmd* newcmd) = 0;
        virtual void   reset() = 0;
    };

    struct hRenderDeviceSetup
    {
        hTempRenderMemAlloc     alloc_;
        hTempRenderMemFree      free_;
        hdDX11Texture*          depthBufferTex_;
    };
}

#endif // DEVICEDX11RENDERDEVICE_H__