/********************************************************************
	created:	2010/10/17
	created:	17:10:2010   11:46
	filename: 	MaterialBuilder.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "MaterialBuilder.h"
#include "Heart.h"


//typedef void (*CGIncludeCallbackFunc)( CGcontext context, const char *filename );

void cgIncludeCallback( CGcontext context, const char *filename )
{
    cgSetCompilerIncludeFile( context, filename, filename );
}

//void cgSetCompilerIncludeCallback( CGcontext context, CGIncludeCallbackFunc func );


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MaterialEffectBuilder::MaterialEffectBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo )
    : gdResourceBuilderBase( resBuilderInfo )
    , cgFXSource(NULL)
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MaterialEffectBuilder::~MaterialEffectBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MaterialEffectBuilder::BuildResource()
{
    CGcontext cgCtx = cgCreateContext();
    cgD3D9RegisterStates( cgCtx );
    cgD3D10RegisterStates( cgCtx );
    cgD3D11RegisterStates( cgCtx );
    cgGLRegisterStates( cgCtx );

    const char* cgDefines = "#define HEART_USING_HLSL (1)\r\n";

    hUint32 len = (hUint32)GetInputFile()->GetFileSize();
    cgFXSource = new hChar[len+1+strlen(cgDefines)];
    strcpy( cgFXSource, cgDefines );
    GetInputFile()->Read( cgFXSource+strlen(cgDefines), len );
    cgFXSource[len+strlen(cgDefines)] = 0;

//     for ( CGstate st = cgGetFirstState( cgCtx ); st != 0; st = cgGetNextState( st ) )
//     {
//         hcPrintf( "%s (Type:%s)\n", cgGetStateName( st ), cgGetTypeString( cgGetStateType(st) ) );
//         switch ( cgGetStateType(st) )
//         {
//         case CG_INT:
//         case CG_INT1:
//         case CG_INT2:
//         case CG_INT3:
//         case CG_INT4:
//             hcPrintf( "{" );
//             for ( int i = 0; i < cgGetNumStateEnumerants( st ); ++i )
//             {
//                 int val;
//                 const char* name;
//                 name = cgGetStateEnumerant( st, i, &val );
//                 hcPrintf( "    { \"%s\" ,  },", name );
//             }
//             hcPrintf( "};" );
//             break;
//         default:
//             break;
//         }
//     }
//     
    //cgSetCompilerIncludeCallback( cgCtx, cgIncludeCallback );
    
    Heart::hMaterial hMat(NULL);
    CGeffect cgFX = cgCreateEffect( cgCtx, cgFXSource, NULL );
    if ( !cgFX )
    {
        ThrowFatalError( "Failed to compile CgFX file. Cg Output:: \n%s", cgGetLastListing( cgCtx ) );
    }

    std::set< std::string > sharedParameterNames;
    std::set< std::string > sharedSamplerNames;

    hUint32 techCount = 0;
    for ( CGtechnique tech = cgGetFirstTechnique( cgFX ); tech != 0; tech = cgGetNextTechnique( tech ), ++techCount )
    {
        const hChar* tname = cgGetTechniqueName( tech );
        Heart::hMaterialTechnique hTech;
        Heart::hStrNCopy( hTech.name_.GetBuffer(), Heart::hMaterialTechnique::MAX_NAME_LEN, tname );

        hUint32 passCount = 0;
        for ( CGpass pass = cgGetFirstPass( tech ); pass != 0; pass = cgGetNextPass( pass ), ++passCount )
        {
            Heart::hMaterialTechniquePass hPass;
            hPass.DefaultState();

            CGprogram vp = cgGetPassProgram( pass, CG_VERTEX_DOMAIN );
            CGprogram fp = cgGetPassProgram( pass, CG_FRAGMENT_DOMAIN );
            const hChar* vpProfileName = cgGetProgramString( vp, CG_PROGRAM_PROFILE );
            const hChar* vpEntry = cgGetProgramString( vp, CG_PROGRAM_ENTRY );
            const hChar* fpProfileName  = cgGetProgramString( fp, CG_PROGRAM_PROFILE );
            const hChar* fpEntry = cgGetProgramString( fp, CG_PROGRAM_ENTRY );

            hChar depName[64];

            Heart::hStrPrintf( depName, 64, "t%02up%02uvp", techCount, passCount );
            hUint32 vpResId = AddBuildDependency( "Shader Program", depName, GetInputFile()->GetPath() );
            GetDependencyParameter( vpResId, "Entry Point" )->Set( vpEntry );
            GetDependencyParameter( vpResId, "Profile" )->Set( vpProfileName );
            GetDependencyParameter( vpResId, "Program Type" )->SetEnumByValue( Heart::ShaderType_VERTEXPROG );

            Heart::hStrPrintf( depName, 64, "t%02up%02ufp", techCount, passCount );
            hUint32 fpResId = AddBuildDependency( "Shader Program", depName, GetInputFile()->GetPath() );
            GetDependencyParameter( fpResId, "Entry Point" )->Set( fpEntry );
            GetDependencyParameter( fpResId, "Profile" )->Set( fpProfileName );
            GetDependencyParameter( fpResId, "Program Type" )->SetEnumByValue( Heart::ShaderType_FRAGMENTPROG );

            hPass.vertexProgram_   = (Heart::hShaderProgram*)vpResId;
            hPass.fragmentProgram_ = (Heart::hShaderProgram*)fpResId;

            //Find Shared Parameters
            for ( CGparameter param = cgGetFirstParameter( fp, CG_GLOBAL ); param != 0; param = cgGetNextParameter( param ) )
            {
                const hChar* parname = cgGetParameterName( param );
                const char* paramTypeName = cgGetTypeString( cgGetParameterType( param ) );

                for ( CGprogram prog = cgGetFirstProgram( cgCtx ); prog != 0; prog = cgGetNextProgram( prog ) )
                {
                    CGtype basety = cgGetParameterBaseType( param );
                    CGtype ty = cgGetParameterType( param );

                    if ( cgIsParameterUsed( param, prog ) )
                    {      
                        if ( ty != CG_PROGRAM_TYPE && 
                            Heart::hStrCmp( parname, vpEntry ) &&
                            Heart::hStrCmp( parname, fpEntry ) &&
                            sharedParameterNames.find( parname ) == sharedParameterNames.end() )
                        {
                            sharedParameterNames.insert( parname );
                        }
                    }

                    if ( strstr( paramTypeName, "texture" ) )
                    {
                        for ( CGparameter leafParam = cgGetFirstLeafEffectParameter( cgFX ); leafParam != 0; leafParam = cgGetNextLeafParameter( leafParam ) )
                        {
                            const char* ps = cgGetParameterName(leafParam);
                            Heart::hSamplerParameter samplerParameter;
                            samplerParameter.DefaultState();
                            if ( cgGetFirstSamplerStateAssignment(leafParam) && sharedSamplerNames.find( ps ) == sharedSamplerNames.end() )
                            {
                                for ( CGstateassignment lpsa = cgGetFirstSamplerStateAssignment(leafParam); lpsa != 0; lpsa = cgGetNextStateAssignment( lpsa ) )
                                {
                                    MapCgSamplerStateToRuntimeState( &samplerParameter, lpsa );
                                }

                                hMat.samplers_.PushBack( samplerParameter );
                                sharedSamplerNames.insert( ps );
                            }
                        }
                    }
                }
            }

            for ( CGstateassignment sa = cgGetFirstStateAssignment( pass ); sa != 0; sa = cgGetNextStateAssignment( sa ) )
            {
               CGstate st = cgGetStateAssignmentState( sa );
               const hChar* statename = cgGetStateName( st );
               CGtype sttype = cgGetStateType( st );
               
               MapCgPassStateToRuntimeState( &hPass, sa );
            }

            hTech.passes_.PushBack( hPass );
        }

        hMat.techniques_.PushBack( hTech );
    }

    AppendWarning( "The Material Builder is not complete yet!" );

    GetSerilaiseObject()->Serialise( GetOutputFile(), hMat );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MaterialEffectBuilder::CleanUpFromBuild()
{
    delete cgFXSource;
    cgFXSource = NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "CgStateConverters.h"

void MaterialEffectBuilder::MapCgPassStateToRuntimeState( Heart::hMaterialTechniquePass* lhs, CGstateassignment stas )
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

void MaterialEffectBuilder::MapCgSamplerStateToRuntimeState( Heart::hSamplerParameter* lhs, CGstateassignment rhs )
{
    hBool foundState = hFalse;
    struct EnumRemap
    {
        const char*                 name;
        Heart::hSAMPLER_STATE_VALUE value;
    };
    EnumRemap remap[] =
    {
        { "repeat",                  Heart::SSV_WRAP },
        { "wrap",                    Heart::SSV_WRAP },
        { "clamp",                   Heart::SSV_CLAMP },
        { "clamptoedge",             Heart::SSV_CLAMP },
        { "mirror",                  Heart::SSV_MIRROR },
        { "border",                  Heart::SSV_BORDER },
        { "anisotropic",             Heart::SSV_ANISOTROPIC },
        { "point",                   Heart::SSV_POINT },
        { "linear",                  Heart::SSV_LINEAR },
        { "linearmipmaplinear",      Heart::SSV_LINEAR },
    };

    CONVERT_CG_INT_ENUM_STATE_SAMP( "AddressU", lhs->addressU_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE_SAMP( "AddressV", lhs->addressV_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE_SAMP( "AddressW", lhs->addressW_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE_SAMP( "WrapS", lhs->addressU_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE_SAMP( "WrapT", lhs->addressV_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE_SAMP( "WrapR", lhs->addressW_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE_SAMP( "MinFilter", lhs->minFilter_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE_SAMP( "MagFilter", lhs->magFilter_, rhs, remap, foundState );
    CONVERT_CG_INT_ENUM_STATE_SAMP( "MipFilter", lhs->mipFilter_, rhs, remap, foundState );
    CONVERT_CG_FLOAT_STATE_SAMP( "MipMapLodBias", lhs->mipLODBias_, rhs, foundState );
    CONVERT_CG_FLOAT_STATE_SAMP( "LODBias", lhs->mipLODBias_, rhs, foundState );
    CONVERT_CG_COLOUR_STATE_SAMP( "BorderColor", lhs->borderColour_, rhs, foundState );
    CONVERT_CG_INT_STATE_SAMP( "MaxAnisotropy", lhs->maxAnisotropy_, rhs, foundState );
    {
        CGstate cgst = cgGetSamplerStateAssignmentState( rhs ); 
        if ( cgst && Heart::hStrCmp( cgGetStateName( cgst ), "Texture" ) == 0 ) 
        {
            CGparameter atp = cgGetTextureStateAssignmentValue( rhs );
            CGannotation at = cgGetNamedParameterAnnotation( atp, "ResourceName" );
            if ( at != 0 && cgGetAnnotationType( at ) == CG_STRING )
            {
                const hChar* texturename = cgGetStringAnnotationValue( at );
                hUint32 texResId = AddBuildDependency( "texture", cgGetParameterName( atp ), texturename );
                lhs->boundTexture_ = (Heart::hTextureBase*)texResId;
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




