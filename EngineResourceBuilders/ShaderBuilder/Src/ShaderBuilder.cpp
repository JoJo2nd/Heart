/********************************************************************

	filename: 	ShaderBuilder.cpp	
	
	Copyright (c) 31:3:2012 James Moran
	
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
#include "ShaderBuilder.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const hChar* ShaderProgramBuilder::ParameterName_Entry = "Entry Point";
const hChar* ShaderProgramBuilder::ParameterName_Profile = "Profile";
const hChar* ShaderProgramBuilder::ParameterName_Type = "Program Type";
const hChar* ShaderProgramBuilder::ParameterName_DebugInfo = "Include Debug Info";

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct FXIncludeHandler : public ID3DInclude 
{
    STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) 
    //HRESULT Open( D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
    {
        GameData::gdFileHandle* f = progameBuilder_->OpenFile( pFileName );

        if ( !f->IsValid() )
            return E_FAIL;

        gdByte* buffer = new gdByte[(hUint32)f->GetFileSize()];
        f->Read( buffer, (hUint32)f->GetFileSize() );

        *ppData = buffer;
        *pBytes = (UINT)f->GetFileSize();

        includes_.push_back( *ppData );

        return S_OK;
    }

    STDMETHOD(Close)(THIS_ LPCVOID pData)
    //HRESULT Close(LPCVOID pData)
    {
        delete pData;
        includes_.remove( pData );

        return S_OK;
    }

    typedef std::list< const void* > IncludedDataArray;

    ShaderProgramBuilder*   progameBuilder_;
    IncludedDataArray       includes_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ShaderProgramBuilder::ShaderProgramBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo )
    : gdResourceBuilderBase( resBuilderInfo )
    , cgFXSource(NULL)
    , result_(NULL)
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ShaderProgramBuilder::~ShaderProgramBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ShaderProgramBuilder::BuildResource()
{
    HRESULT hr;
    hUint32 compileFlags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;
    compileFlags |= 0;//GetParameter( ParameterName_DebugInfo ).GetAsBool() ? D3DCOMPILE_DEBUG : 0;
    ID3DBlob* errors;
    D3D_SHADER_MACRO macros[] =
    {
        { "HEART_USING_HLSL", "1" },
        { "HEART_USING_HLSL_COMPILER", "1" },
        { NULL, NULL }
    };
    FXIncludeHandler include;
    include.progameBuilder_ = this;
    const char* entry = GetParameter( ParameterName_Entry ).GetAsString();
    const char* profile = GetParameter( ParameterName_Profile ).GetAsString();
    hr = D3DX11CompileFromFile( 
        GetInputFile()->GetPath(), 
        macros, 
        &include, //Includes
        GetParameter( ParameterName_Entry ).GetAsString(),
        GetParameter( ParameterName_Profile ).GetAsString(), 
        compileFlags, 
        0, 
        NULL, 
        &result_, 
        &errors, 
        NULL );

    if ( FAILED( hr ) )
    {
        ThrowFatalError( "D3DX Shader Compile failed! Error Msg :: %s", errors->GetBufferPointer() );
    }

    ID3D11ShaderReflection* reflect;
    hr = D3DReflect( result_->GetBufferPointer(), result_->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflect );
    if ( FAILED( hr ) )
    {
        ThrowFatalError( "Couldn't create reflection information." );
    }

    outputProgram_.shaderProgramLength_ = result_->GetBufferSize();
    outputProgram_.shaderProgram_ = hNEW_ARRAY(hGeneralHeap, hByte, outputProgram_.shaderProgramLength_);
    outputProgram_.shaderType_ = (Heart::ShaderType)GetParameter( ParameterName_Type ).GetAsEnumValue().value_;
    Heart::hMemCpy( outputProgram_.shaderProgram_, result_->GetBufferPointer(), result_->GetBufferSize() );

    D3D11_SHADER_DESC desc;
    reflect->GetDesc( &desc );

    outputProgram_.vertexInputLayoutFlags_ = 0;

    if ( outputProgram_.shaderType_ == Heart::ShaderType_VERTEXPROG )
    {
        ParseVertexInputFormat(desc, reflect);

    }

    outputProgram_.parameterCount_ = 0;
    outputProgram_.totalParameterSize_ = 0;
    //outputProgram_.constBuffers_.Resize( desc.ConstantBuffers );
    //CONSTANT BUFFERS
    for ( hUint32 i = 0; i < desc.ConstantBuffers; ++i )
    {
        ID3D11ShaderReflectionConstantBuffer* constInfo = reflect->GetConstantBufferByIndex( i );
        D3D11_SHADER_BUFFER_DESC bufInfo;
        constInfo->GetDesc( &bufInfo );
//         Heart::hShaderConstBuffer& outConstBuf = outputProgram_.constBuffers_[i];
//         outConstBuf.size_ = bufInfo.Size / sizeof( hFloat );
//         outConstBuf.buffer_ = new hFloat[outConstBuf.size_];
        outputProgram_.totalParameterSize_ += bufInfo.Size / sizeof( hFloat );

        //TODO: Get Parameters...
        for ( hUint32 i = 0; i < bufInfo.Variables; ++i )
        {
            ID3D11ShaderReflectionVariable* var = constInfo->GetVariableByIndex( i );
            D3D11_SHADER_VARIABLE_DESC varDesc;
            var->GetDesc(&varDesc);
            
            ++outputProgram_.parameterCount_;

            if ( varDesc.DefaultValue )
            {
                //Heart::hMemCpy( outConstBuf.buffer_+varDesc.StartOffset, varDesc.DefaultValue, varDesc.Size );
            }
        }
    }

    GetSerilaiseObject()->Serialise( GetOutputFile(), outputProgram_ );

    reflect->Release();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ShaderProgramBuilder::CleanUpFromBuild()
{
    delete cgFXSource;
    cgFXSource = NULL;
    if ( result_ )
    {
        result_->Release();
        result_ = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ShaderProgramBuilder::ParseVertexInputFormat( D3D11_SHADER_DESC &desc, ID3D11ShaderReflection* reflect )
{
    for ( hUint32 i = 0; i < desc.InputParameters; ++i )
    {
        D3D11_SIGNATURE_PARAMETER_DESC inputDesc;
        reflect->GetInputParameterDesc( i, &inputDesc );

        if ( Heart::hStrCmp( inputDesc.SemanticName, "POSITION" ) == 0 )
        {
            outputProgram_.vertexInputLayoutFlags_ |= Heart::hrVF_XYZ;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "POSITIONT" ) == 0 )
        {
            outputProgram_.vertexInputLayoutFlags_ |= Heart::hrVF_XYZW;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "NORMAL" ) == 0 )
        {
            outputProgram_.vertexInputLayoutFlags_ |= Heart::hrVF_NORMAL;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "TANGENT" ) == 0 )
        {
            outputProgram_.vertexInputLayoutFlags_ |= Heart::hrVF_TANGENT;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "BINORMAL" ) == 0 )
        {
            outputProgram_.vertexInputLayoutFlags_ |= Heart::hrVF_BINORMAL;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "COLOR" ) == 0 )
        {
            outputProgram_.vertexInputLayoutFlags_ |= Heart::hrVF_COLOR;
        }
        else if ( Heart::hStrCmp( inputDesc.SemanticName, "TEXCOORD" ) == 0 )
        {
            outputProgram_.vertexInputLayoutFlags_ |= Heart::hrVF_1UV << inputDesc.SemanticIndex;
        }
        else
        {
            AppendWarning( "Unknown input semantic %s for vertex program", inputDesc.SemanticName );
        }
    }
}
