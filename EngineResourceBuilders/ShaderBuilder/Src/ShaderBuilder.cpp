/********************************************************************
	created:	2010/10/17
	created:	17:10:2010   11:46
	filename: 	MaterialBuilder.cpp	
	author:		James
	
	purpose:	
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
    hUint32 compileFlags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY /*| D3DCOMPILE_DEBUG*/;
    ID3DBlob* errors;
    D3D_SHADER_MACRO macros[] =
    {
        { "HEART_USING_HLSL", "1" },
        { NULL, NULL }
    };
    const char* entry = GetParameter( ParameterName_Entry ).GetAsString();
    const char* profile = GetParameter( ParameterName_Profile ).GetAsString();
    hr = D3DX11CompileFromFile( 
        GetInputFile()->GetPath(), 
        macros, 
        NULL, //Includes
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
    outputProgram_.shaderProgram_ = new hByte[outputProgram_.shaderProgramLength_];
    outputProgram_.shaderType_ = (Heart::ShaderType)GetParameter( ParameterName_Type ).GetAsEnumValue().value_;
    Heart::hMemCpy( outputProgram_.shaderProgram_, result_->GetBufferPointer(), result_->GetBufferSize() );

    D3D11_SHADER_DESC desc;
    reflect->GetDesc( &desc );

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
/*
//#include "CgStateConverters.h"

void ShaderProgramBuilder::MapCgPassStateToRuntimeState( Heart::hMaterialTechniquePass* lhs, CGstateassignment stas )
{
    using namespace Heart;

    hBool foundState = hFalse;
    struct EnumRemap
    {
        const char*                 name;
        Heart::RENDER_STATE_VALUE   value;
    };
    EnumRemap blendFuncEnum[] = 
    {
        { "funcadd" , RSV_BLEND_FUNC_ADD },
        { "funcsubtract" , RSV_BLEND_FUNC_SUB },
        { "add" , RSV_BLEND_FUNC_ADD },
        { "subtract" , RSV_BLEND_FUNC_SUB },
        { "min" , RSV_BLEND_FUNC_MIN },
        { "max" , RSV_BLEND_FUNC_MAX },
    };
    EnumRemap blendOpEnum[] =
    {
        { "zero" , RSV_BLEND_OP_ZERO },
        { "one" , RSV_BLEND_OP_ONE },
        { "destcolor" , RSV_BLEND_OP_DEST_COLOUR },
        { "invdestcolor" , RSV_BLEND_OP_INVDEST_COLOUR },
        { "srcalpha" , RSV_BLEND_OP_SRC_ALPHA },
        { "invsrcalpha" , RSV_BLEND_OP_INVSRC_ALPHA },
        { "dstalpha" , RSV_BLEND_OP_DEST_ALPHA },
        { "invdestalpha" , RSV_BLEND_OP_INVDEST_ALPHA },
        { "srccolor" , RSV_BLEND_OP_SRC_COLOUR },
        { "invsrccolor" , RSV_BLEND_OP_INVSRC_COLOUR },
    };
    EnumRemap depthEnum[] = 
    {
        { "never" , RSV_Z_CMP_NEVER },
        { "less" , RSV_Z_CMP_LESS },
        { "lequal" , RSV_Z_CMP_LESSEQUAL },
        { "lessequal" , RSV_Z_CMP_LESSEQUAL },
        { "equal" , RSV_Z_CMP_EQUAL },
        { "greater" , RSV_Z_CMP_GREATER },
        { "notequal" , RSV_Z_CMP_NOT_EQUAL },
        { "gequal" , RSV_Z_CMP_GREATER_EQUAL },
        { "greaterequal" , RSV_Z_CMP_GREATER_EQUAL },
        { "always" , RSV_Z_CMP_ALWAYS },
    };
    EnumRemap fillModeEnum[] = 
    {
        { "solid" , RSV_FILL_MODE_SOLID },
        { "wireframe" , RSV_FILL_MODE_WIREFRAME },
    };
    EnumRemap cullModeEnum[] =
    {
        { "none" , RSV_CULL_MODE_NONE },
        { "cw" , RSV_CULL_MODE_CW },
        { "ccw" , RSV_CULL_MODE_CCW },
    };
    EnumRemap stencilFuncEnum[] =
    {
        { "never" , RSV_SF_CMP_NEVER },
        { "less" , RSV_SF_CMP_LESS },
        { "lequal" , RSV_SF_CMP_LESSEQUAL },
        { "lessequal" , RSV_SF_CMP_LESSEQUAL },
        { "equal" , RSV_SF_CMP_EQUAL },
        { "greater" , RSV_SF_CMP_GREATER },
        { "notequal" , RSV_SF_CMP_NOT_EQUAL },
        { "gequal" , RSV_SF_CMP_GREATER_EQUAL },
        { "greaterequal" , RSV_SF_CMP_GREATER_EQUAL },
        { "always" , RSV_SF_CMP_ALWAYS },
    };
    EnumRemap stencilOpEnum[] = 
    {
        { "keep" ,      RSV_SO_KEEP, },
        { "zero" ,      RSV_SO_ZERO, },
        { "replace" ,   RSV_SO_REPLACE, },
        { "incr" ,      RSV_SO_INCR, },
        { "decr" ,      RSV_SO_DECR, },
        { "invert" ,    RSV_SO_INVERT, },
        { "incrsat" ,   RSV_SO_INCRSAT },
        { "decrsat" ,   RSV_SO_DECRSAT },
    };

    CONVERT_CG_BOOL_STATE( "BlendEnable", lhs->alphaBlendEnable_, stas, foundState );
    CONVERT_CG_INT_ENUM_STATE( "BlendOp", lhs->alphaBlendFunction_, stas, blendFuncEnum, foundState );
    CONVERT_CG_INT_ENUM_STATE( "BlendEquation", lhs->alphaBlendFunction_, stas, blendFuncEnum, foundState );
    CONVERT_CG_INT_ENUM_STATE( "SrcBlend", lhs->alphaSrcBlend_, stas, blendOpEnum, foundState );
    CONVERT_CG_INT_ENUM_STATE( "DestBlend", lhs->alphaDstBlend_, stas, blendOpEnum, foundState );
    CONVERT_CG_INTx2_ENUM_STATE( "BlendFucn", lhs->alphaSrcBlend_, lhs->alphaDstBlend_, stas, blendOpEnum, foundState );
    CONVERT_CG_INT_ENUM_STATE( "DepthFunc", lhs->zCompareFunction_, stas, depthEnum, foundState );
    CONVERT_CG_INT_ENUM_STATE( "ZFunc", lhs->zCompareFunction_, stas, depthEnum, foundState );
    CONVERT_CG_BOOL_STATE( "DepthMask", lhs->zWriteEnable_, stas, foundState );
    CONVERT_CG_BOOL_STATE( "ZWriteEnable", lhs->zWriteEnable_, stas, foundState );
    CONVERT_CG_BOOL_STATE( "DepthTestEnable", lhs->zTestEnable_, stas, foundState );
    CONVERT_CG_BOOL_STATE( "ZEnable", lhs->zTestEnable_, stas, foundState );
    CONVERT_CG_INT_ENUM_STATE( "FillMode", lhs->fillMode_, stas, fillModeEnum, foundState );
    CONVERT_CG_INT_ENUM_STATE( "CullMode", lhs->cullMode_, stas, cullModeEnum, foundState );
    CONVERT_CG_BOOL_STATE( "StencilEnable", lhs->stencilTest_, stas, foundState );
    CONVERT_CG_INT_ENUM_STATE( "StencilFunc", lhs->stencilFunc_, stas, stencilFuncEnum, foundState );
    CONVERT_CG_INT_ENUM_STATE( "StencilFail", lhs->stencilFail_, stas, stencilOpEnum, foundState );
    CONVERT_CG_INT_STATE( "StencilRef", lhs->stencilRef_, stas, foundState );
    CONVERT_CG_INT_ENUM_STATE( "StencilPass", lhs->stencilPass_, stas, stencilOpEnum, foundState );
    CONVERT_CG_INT_ENUM_STATE( "StencilZFail", lhs->stencilZFail_, stas, stencilOpEnum, foundState );
    CONVERT_CG_INT_STATE( "StencilWriteMask", lhs->stencilWriteMask_, stas, foundState );
    CONVERT_CG_INT_STATE( "StencilMask", lhs->stencilMask_, stas, foundState );
    CONVERT_CG_BOOL_COLOURWRITE_STATE( 
        "ColourWriteEnable", 
        lhs->colourMask1_, 
        Heart::RSV_COLOUR_WRITE_RED, 
        Heart::RSV_COLOUR_WRITE_GREEN, 
        Heart::RSV_COLOUR_WRITE_BLUE, 
        Heart::RSV_COLOUR_WRITE_ALPHA, stas, 
        foundState );
    CONVERT_CG_BOOL_COLOURWRITE_STATE( 
        "ColourWriteEnable1", 
        lhs->colourMask2_, 
        Heart::RSV_COLOUR_WRITE_RED, 
        Heart::RSV_COLOUR_WRITE_GREEN, 
        Heart::RSV_COLOUR_WRITE_BLUE, 
        Heart::RSV_COLOUR_WRITE_ALPHA, stas,
        foundState );
    CONVERT_CG_BOOL_COLOURWRITE_STATE( 
        "ColourWriteEnable2", 
        lhs->colourMask3_, 
        Heart::RSV_COLOUR_WRITE_RED, 
        Heart::RSV_COLOUR_WRITE_GREEN, 
        Heart::RSV_COLOUR_WRITE_BLUE, 
        Heart::RSV_COLOUR_WRITE_ALPHA, stas,
        foundState );
    CONVERT_CG_BOOL_COLOURWRITE_STATE( 
        "ColourWriteEnable3", 
        lhs->colourMask4_, 
        Heart::RSV_COLOUR_WRITE_RED, 
        Heart::RSV_COLOUR_WRITE_GREEN, 
        Heart::RSV_COLOUR_WRITE_BLUE, 
        Heart::RSV_COLOUR_WRITE_ALPHA, stas,
        foundState );

    // Warning on Unknown State Value...
    if ( !foundState )
    {
        CGstate state = cgGetStateAssignmentState( stas );
        AppendWarning( "Unable to find match state for CG pass state \"%s\". Ignoring State", cgGetStateName( state ) );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ShaderProgramBuilder::MapCgSamplerStateToRuntimeState( Heart::hSamplerParameter* lhs, CGstateassignment rhs )
{
    hBool foundState = hFalse;
    struct EnumRemap
    {
        const char*                 name;
        Heart::hSAMPLER_STATE_VALUE value;
    };
    EnumRemap remap[] =
    {
        { "Repeat",      Heart::SSV_WRAP },
        { "Wrap",        Heart::SSV_WRAP },
        { "Clamp",       Heart::SSV_CLAMP },
        { "Mirror",      Heart::SSV_MIRROR },
        { "Border",      Heart::SSV_BORDER },
        { "Anisotropic", Heart::SSV_ANISOTROPIC },
        { "Point",       Heart::SSV_POINT },
        { "Linear",      Heart::SSV_LINEAR },
    };

    CONVERT_CG_INT_ENUM_STATE( "AddressU", lhs->addressU_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE( "AddressV", lhs->addressV_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE( "AddressW", lhs->addressW_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE( "WrapS", lhs->addressU_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE( "WrapT", lhs->addressV_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE( "WrapR", lhs->addressW_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE( "MinFilter", lhs->minFilter_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE( "MagFilter", lhs->magFilter_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE( "MipFilter", lhs->mipFilter_, rhs, remap, foundState );
    CONVERT_CG_FLOAT_STATE( "MipMapLodBias", lhs->mipLODBias_, rhs, foundState );
    CONVERT_CG_FLOAT_STATE( "LODBias", lhs->mipLODBias_, rhs, foundState );
    CONVERT_CG_COLOUR_STATE( "BorderColor", lhs->borderColour_, rhs, foundState );
    CONVERT_CG_INT_STATE( "MaxAnisotropy", lhs->maxAnisotropy_, rhs, foundState );
    {
        CGstate cgst = cgGetStateAssignmentState( rhs ); 
        if ( Heart::hStrCmp( cgGetStateName( cgst ), "Texture" ) == 0 ) 
        {
            CGparameter atp = cgGetSamplerStateAssignmentParameter( rhs );
            CGannotation at = cgGetNamedParameterAnnotation( atp, "ResourceName" );
            if ( at != 0 && cgGetAnnotationType( at ) == CG_STRING )
            {
                const hChar* texturename = cgGetStringAnnotationValue( at );
                hUint32 texResId = AddBuildDependency( "texture", cgGetParameterName( atp ), GetInputFile()->GetPath() );
                foundState = true;
            }
        }
    }

    // Warning on Unknown State Value...
    if ( !foundState )
    {
        CGstate state = cgGetStateAssignmentState( rhs );
        AppendWarning( "Unable to find match state for CG pass state \"%s\". Ignoring State", cgGetStateName( state ) );
    }
}

*/


;