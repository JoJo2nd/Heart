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

    hUint32 hdDX11ShaderProgram::GetShaderParameterCount()
    {
        hUint32 params = 0;
        D3D11_SHADER_DESC desc;
        shaderInfo_->GetDesc( &desc );
        //CONSTANT BUFFERS
        for ( hUint32 buffer = 0; buffer < desc.ConstantBuffers; ++buffer )
        {
            ID3D11ShaderReflectionConstantBuffer* constInfo = shaderInfo_->GetConstantBufferByIndex( buffer );
            D3D11_SHADER_BUFFER_DESC bufInfo;
            constInfo->GetDesc( &bufInfo );

            params += bufInfo.Variables;
        }

        return params;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdDX11ShaderProgram::GetShaderParameter( hUint32 i, hShaderParameter* param )
    {
        hcAssert( param );
        HRESULT hr;
        D3D11_SHADER_DESC desc;
        shaderInfo_->GetDesc( &desc );
        //CONSTANT BUFFERS
        for ( hUint32 buffer = 0; buffer < desc.ConstantBuffers; ++buffer )
        {
            ID3D11ShaderReflectionConstantBuffer* constInfo = shaderInfo_->GetConstantBufferByIndex( buffer );
            D3D11_SHADER_BUFFER_DESC bufInfo;
            constInfo->GetDesc( &bufInfo );
            hUint32 cBufferCRC = hCRC32::StringCRC( bufInfo.Name );

            if ( i < bufInfo.Variables )
            {
                ID3D11ShaderReflectionVariable* var = constInfo->GetVariableByIndex( i );
                D3D11_SHADER_VARIABLE_DESC varDesc;
                var->GetDesc( &varDesc );
                D3D11_SHADER_INPUT_BIND_DESC bindInfo;
                hr = shaderInfo_->GetResourceBindingDescByName( bufInfo.Name, &bindInfo );
                hcAssert(SUCCEEDED(hr));

                hStrCopy(param->name_.GetBuffer(), param->name_.GetMaxSize(), varDesc.Name);
                param->size_    = varDesc.Size;
                param->cBuffer_ = bindInfo.BindPoint;
                param->cReg_    = varDesc.StartOffset;

                return hTrue;
            }
            else
            {
                i -= bufInfo.Variables;
            }
        }

        return hFalse;
    }

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

        ret->reg_ = bindInfo.BindPoint;
        ret->size_ = bufInfo.Size;
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

    hdDX11VertexLayout* hdDX11ShaderProgram::createVertexLayout(hInputLayoutDesc* desc, hUint n) const {
        D3D11_SHADER_DESC shaderdesc;
        shaderInfo_->GetDesc(&shaderdesc);
        if (n < shaderdesc.InputParameters) {
            return NULL;
        }
        return device_->CreateVertexLayout(desc, n, shaderBlob_, blobLen_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11ShaderProgram::destroyVertexLayout(hdDX11VertexLayout* vtxlayout) const {
        if (!vtxlayout) return;
        device_->DestroyVertexLayout(vtxlayout);
    }

}