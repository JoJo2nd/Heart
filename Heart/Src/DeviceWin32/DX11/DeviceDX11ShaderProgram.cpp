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

    hBool hdDX11ShaderProgram::GetShaderParameter( hUint32 i, hShaderParameter* param )
    {
        hcAssert( param );

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

                //lets be cheeky and nab the shader var name here!
                param->name_    = (hChar*)varDesc.Name;
                param->size_    = varDesc.Size / sizeof(hFloat);
                param->cBuffer_ = cBufferCRC;
                param->cReg_    = varDesc.StartOffset / sizeof(hFloat);

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

    hUint32 hdDX11ShaderProgram::GetConstantBufferCount() const
    {
        D3D11_SHADER_DESC desc;
        shaderInfo_->GetDesc( &desc );

        return desc.ConstantBuffers;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hdDX11ShaderProgram::GetConstantBufferSize( hUint32 idx ) const
    {
        ID3D11ShaderReflectionConstantBuffer* constInfo = shaderInfo_->GetConstantBufferByIndex( idx );
        D3D11_SHADER_BUFFER_DESC bufInfo;
        constInfo->GetDesc( &bufInfo );

        return bufInfo.Size / sizeof(hFloat);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hChar* hdDX11ShaderProgram::GetConstantBufferName( hUint32 idx ) const
    {
        ID3D11ShaderReflectionConstantBuffer* constInfo = shaderInfo_->GetConstantBufferByIndex( idx );
        D3D11_SHADER_BUFFER_DESC bufInfo;
        constInfo->GetDesc( &bufInfo );

        return bufInfo.Name;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hFloat* hdDX11ShaderProgram::GetShaderParameterDefaultValue( hUint32 idx ) const
    {
        D3D11_SHADER_DESC desc;
        shaderInfo_->GetDesc( &desc );

        for ( hUint32 buffer = 0; buffer < desc.ConstantBuffers; ++buffer )
        {
            ID3D11ShaderReflectionConstantBuffer* constInfo = shaderInfo_->GetConstantBufferByIndex( buffer );
            D3D11_SHADER_BUFFER_DESC bufInfo;
            constInfo->GetDesc( &bufInfo );

            if ( idx < bufInfo.Variables )
            {
                 ID3D11ShaderReflectionVariable* var = constInfo->GetVariableByIndex( idx );
                D3D11_SHADER_VARIABLE_DESC varDesc;
                var->GetDesc( &varDesc );

                return (hFloat*)varDesc.DefaultValue;
            }
            else
            {
                idx -= bufInfo.Variables;
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hdDX11ShaderProgram::GetConstantBufferReg( hUint32 idx ) const
    {
        D3D11_SHADER_DESC desc;
        shaderInfo_->GetDesc( &desc );

        ID3D11ShaderReflectionConstantBuffer* constInfo = shaderInfo_->GetConstantBufferByIndex( idx );
        D3D11_SHADER_BUFFER_DESC bufInfo;
        D3D11_SHADER_INPUT_BIND_DESC bindInfo;
        constInfo->GetDesc( &bufInfo );
        shaderInfo_->GetResourceBindingDescByName( bufInfo.Name, &bindInfo );

        return bindInfo.BindPoint;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hdDX11ShaderProgram::GetSamplerRegister( const hChar* name ) const
    {
        D3D11_SHADER_INPUT_BIND_DESC bindInfo;
        HRESULT hr = shaderInfo_->GetResourceBindingDescByName( name, &bindInfo );

        return hr == S_OK ? bindInfo.BindPoint : ~0U;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11ParameterConstantBlock::Flush( ID3D11DeviceContext* ctx )
    {
        ctx->UpdateSubresource( constBuffer_, 0, NULL, cpuIntermediateData_, 0, 0 );
    }

}