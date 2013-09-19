/********************************************************************

	filename: 	DeviceDX11ShaderProgram.cpp	
	
	Copyright (c) 20:12:2011 James Moran
	
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

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hdDX11ShaderProgram::GetConstantBlockCount() const
    {
        D3D11_SHADER_DESC desc;
        shaderInfo_->GetDesc( &desc );

        return desc.ConstantBuffers;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hdDX11ShaderProgram::GetConstantBlockRegister(hShaderParameterID id) const
    {
        D3D11_SHADER_DESC desc;
        D3D11_SHADER_INPUT_BIND_DESC bindInfo;
        D3D11_SHADER_BUFFER_DESC bufInfo;

        shaderInfo_->GetDesc( &desc );

        // Possibly worth caching this information but this hopefully isn't called too often
        for (hUint32 cb = 0; cb < desc.ConstantBuffers; ++cb) {
            ID3D11ShaderReflectionConstantBuffer* constInfo = shaderInfo_->GetConstantBufferByIndex(cb);
            constInfo->GetDesc(&bufInfo);
            shaderInfo_->GetResourceBindingDescByName(bufInfo.Name, &bindInfo);
            if (id == hCRC32::StringCRC(bufInfo.Name)) return bindInfo.BindPoint;
        }

        return hErrorCode;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11ShaderProgram::GetConstantBlockDesc( hUint32 idx, hConstantBlockDesc* ret )
    {
        D3D11_SHADER_DESC desc;
        shaderInfo_->GetDesc( &desc );

        ID3D11ShaderReflectionConstantBuffer* constInfo = shaderInfo_->GetConstantBufferByIndex( idx );
        D3D11_SHADER_BUFFER_DESC bufInfo;
        D3D11_SHADER_INPUT_BIND_DESC bindInfo;
        constInfo->GetDesc( &bufInfo );
        shaderInfo_->GetResourceBindingDescByName( bufInfo.Name, &bindInfo );

        ret->bindPoint_=bindInfo.BindPoint;
        ret->size_=bufInfo.Size;
        ret->parameterCount_=bufInfo.Variables;
        ret->index_=idx;
        hStrCopy(ret->name_, ret->name_.GetMaxSize(), bufInfo.Name);

        //build a hash of all the parameters in the constant buffer
        hCRC32::StartCRC32(&ret->hash_, bufInfo.Name, hStrLen(bufInfo.Name));
        for (hUint32 i = 0; i < bufInfo.Variables; ++i)
        {
            ID3D11ShaderReflectionVariable* var = constInfo->GetVariableByIndex( i );
            D3D11_SHADER_VARIABLE_DESC varDesc;
            var->GetDesc( &varDesc );

            hCRC32::ContinueCRC32(&ret->hash_, varDesc.Name, hStrLen(varDesc.Name));
            hCRC32::ContinueCRC32(&ret->hash_, (hChar*)&varDesc.StartOffset, sizeof(varDesc.StartOffset));
            hCRC32::ContinueCRC32(&ret->hash_, (hChar*)&varDesc.Size, sizeof(varDesc.Size));
        }
        hCRC32::FinishCRC32(&ret->hash_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11ShaderProgram::getConstantBlockParameter(const hConstantBlockDesc& desc, hUint paramIndex, hShaderParameter* outparam) const {
        ID3D11ShaderReflectionConstantBuffer* constinfo=shaderInfo_->GetConstantBufferByIndex(desc.index_);
        ID3D11ShaderReflectionVariable* var=constinfo->GetVariableByIndex(paramIndex);
        D3D11_SHADER_VARIABLE_DESC vardesc;
        var->GetDesc(&vardesc);
        outparam->cBufferBindPoint_=desc.bindPoint_;
        outparam->cReg_=vardesc.StartOffset;
        outparam->size_=vardesc.Size;
        hStrCopy(outparam->name_, outparam->name_.GetMaxSize(), vardesc.Name);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hdDX11ShaderProgram::GetInputRegister( const hChar* name ) const
    {
        D3D11_SHADER_INPUT_BIND_DESC bindInfo;
        HRESULT hr = shaderInfo_->GetResourceBindingDescByName( name, &bindInfo );

        return hr == S_OK ? bindInfo.BindPoint : ~0U;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hdDX11ShaderProgram::GetInputRegister(hShaderParameterID id) const
    {
        D3D11_SHADER_DESC desc;
        D3D11_SHADER_INPUT_BIND_DESC bindInfo;

        shaderInfo_->GetDesc(&desc);

        for(hUint32 i = 0, c = desc.BoundResources; i < c; ++i) {
            shaderInfo_->GetResourceBindingDesc(i, &bindInfo);
            if (id == hCRC32::StringCRC(bindInfo.Name)) return bindInfo.BindPoint;
        }

        return hErrorCode;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11VertexLayout* hdDX11ShaderProgram::createVertexLayout(const hInputLayoutDesc* desc, hUint n) const {
        D3D11_SHADER_DESC shaderdesc;
        shaderInfo_->GetDesc(&shaderdesc);
        //if (n < shaderdesc.InputParameters) {
        //    return NULL;
        //}
        return device_->CreateVertexLayout(desc, n, shaderBlob_, blobLen_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11ShaderProgram::destroyVertexLayout(hdDX11VertexLayout* vtxlayout) const {
        if (!vtxlayout) return;
        device_->DestroyVertexLayout(vtxlayout);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11ShaderProgram::getInputCount() const {
        D3D11_SHADER_DESC shaderdesc;
        shaderInfo_->GetDesc(&shaderdesc);
        return shaderdesc.BoundResources;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11ShaderProgram::getInput(hUint idx, hShaderInput* inputdesc) const {
        D3D11_SHADER_INPUT_BIND_DESC bindInfo;
        shaderInfo_->GetResourceBindingDesc(idx, &bindInfo);

        hStrCopy(inputdesc->name_, inputdesc->name_.GetMaxSize(), bindInfo.Name);
        inputdesc->bindPoint_=bindInfo.BindPoint;
        inputdesc->arraySize_=bindInfo.BindCount;
        switch(bindInfo.Type) {
        case D3D_SIT_CBUFFER                        :
            inputdesc->type_=eShaderInputType_Buffer; break;
        case D3D_SIT_SAMPLER                        :
            inputdesc->type_=eShaderInputType_Sampler; break;
        case D3D_SIT_TBUFFER                        :
        case D3D_SIT_TEXTURE                        :
        case D3D_SIT_STRUCTURED                     :
        case D3D_SIT_BYTEADDRESS                    :
            inputdesc->type_=eShaderInputType_Resource; break;
        case D3D_SIT_UAV_RWTYPED                    :
        case D3D_SIT_UAV_RWSTRUCTURED               :
        case D3D_SIT_UAV_RWBYTEADDRESS              :
        case D3D_SIT_UAV_APPEND_STRUCTURED          :
        case D3D_SIT_UAV_CONSUME_STRUCTURED         :
        case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER  :
            inputdesc->type_=eShaderInputType_UAV; break;
        }
    }

}