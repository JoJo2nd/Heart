/********************************************************************

    filename: 	DeviceDX11ShaderProgram.h	
    
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
#ifndef DEVICEDX11SHADERPROGRAM_H__
#define DEVICEDX11SHADERPROGRAM_H__

#include "base\hTypes.h"
#include "base\hRendererConstants.h"

namespace Heart
{

    struct hShaderParameter;
    class hdDX11RenderDevice;
    struct hConstantBlockDesc;

    struct  hdDX11Buffer
    {
        ID3D11Buffer*    buffer_;
        hUint32          size_;
    };

    class  hdDX11ShaderProgram
    {
    public:
        hdDX11ShaderProgram() 
            : device_(nullptr)
            , type_(ShaderType_MAX)
            , pixelShader_(nullptr)
            , shaderInfo_(nullptr)
            , blobLen_(0)
            , shaderBlob_(nullptr)
        {

        }
        hdDX11ShaderProgram(hdDX11RenderDevice* device) 
            : device_(device)
            , type_(ShaderType_MAX)
            , pixelShader_(hNullptr)
            , shaderInfo_(hNullptr)
            , blobLen_(0)
            , shaderBlob_(hNullptr)
        {

        }
        ~hdDX11ShaderProgram()
        {
        }

        hUint32                         GetConstantBlockCount() const;
        void                            GetConstantBlockDesc(hUint32 idx, hConstantBlockDesc* desc);
        hUint32                         GetConstantBlockRegister(hShaderParameterID id) const;
        void                            getConstantBlockParameter(const hConstantBlockDesc& desc, hUint paramIndex, hShaderParameter* outparam) const;
        hUint32                         GetInputRegister(const hChar* name) const;
        hUint32                         GetInputRegister(hShaderParameterID id) const;
        hUint                           getInputCount() const;
        void                            getInput(hUint idx, hShaderInput* inputdesc) const;
        hdDX11VertexLayout*             createVertexLayout(const hInputLayoutDesc* desc, hUint n) const;
        void                            destroyVertexLayout(hdDX11VertexLayout* vtxlayout) const;

    private:

        friend class hdDX11RenderDevice;
        friend class hdDX11RenderSubmissionCtx;
        friend class hdDX11ComputeInputObject;
        friend class hdDX11RenderInputObject;
        friend class hdDX11RenderCommandGenerator;

        hdDX11RenderDevice*     device_;
        hShaderType             type_;
        union 
        {
            ID3D11PixelShader*      pixelShader_;
            ID3D11VertexShader*     vertexShader_;
            ID3D11GeometryShader*   geomShader_;
            ID3D11HullShader*       hullShader_;
            ID3D11DomainShader*     domainShader_;
            ID3D11ComputeShader*    computeShader_;
        };
        ID3D11ShaderReflection* shaderInfo_;
        hUint                   blobLen_;   // Only valid for vertex shader, stores input layout
        hUint8*                 shaderBlob_;//
    };

    class hShaderReflection;

    class hShaderStage {
    public:
        hShaderStage() {

        }
        ~hShaderStage() {

        }
        
        hShaderType                 type_;
        union 
        {
            ID3D11PixelShader*      pixelShader_;
            ID3D11VertexShader*     vertexShader_;
            ID3D11GeometryShader*   geomShader_;
            ID3D11HullShader*       hullShader_;
            ID3D11DomainShader*     domainShader_;
            ID3D11ComputeShader*    computeShader_;
        };
    };

    class hShaderProgram {
        hShaderProgram() {}
        ~hShaderProgram() {}

        hShaderReflection* getReflectionInfo() const;
        void attachStage(hShaderStage* stage);
        void link();
    };
}
#endif // DEVICEDX11SHADERPROGRAM_H__