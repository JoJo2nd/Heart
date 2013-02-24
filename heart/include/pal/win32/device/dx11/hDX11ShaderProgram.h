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


namespace Heart
{

    struct hShaderParameter;
    class hdDX11RenderDevice;

    struct HEART_DLLEXPORT hdDX11ParameterConstantBlock
    {
        ID3D11Buffer*    constBuffer_;
        hUint32          size_;
    };

    class HEART_DLLEXPORT hdDX11ShaderProgram
    {
    public:
        hdDX11ShaderProgram(hdDX11RenderDevice* device) 
            : device_(device)
            , type_(ShaderType_MAX)
            , pixelShader_(NULL)
            , shaderInfo_(NULL)
            , blobLen_(0)
            , shaderBlob_(NULL)
        {

        }
        ~hdDX11ShaderProgram()
        {
        }

        hUint32                         GetConstantBlockCount() const;
        void                            GetConstantBlockDesc(hUint32 idx, hConstantBlockDesc* desc);
        hUint32                         GetConstantBlockRegister(hShaderParameterID id) const;
        hUint32                         GetShaderParameterCount() const;
        hBool                           GetShaderParameter(hUint32 i, hShaderParameter* param) const;
        hUint32                         GetInputRegister(const hChar* name) const;
        hUint32                         GetInputRegister(hShaderParameterID id) const;
        hdDX11VertexLayout*             createVertexLayout(hInputLayoutDesc* desc, hUint n) const;
        void                            destroyVertexLayout(hdDX11VertexLayout* vtxlayout) const;

    private:

        friend class hdDX11RenderDevice;
        friend class hdDX11RenderSubmissionCtx;
        friend class hdDX11ComputeInputObject;
        friend class hdDX11RenderInputObject;

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
}
#endif // DEVICEDX11SHADERPROGRAM_H__