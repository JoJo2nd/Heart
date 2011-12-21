/********************************************************************
	created:	2009/11/16
	created:	16:11:2009   22:29
	filename: 	hiMaterial.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hMaterial.h"
#include "hRendererConstants.h"
#include "hRenderer.h"
#include "hRenderCommon.h"
#include "hRenderState.h"


namespace Heart
{

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//void hMaterial::InitShaderParameters( hRenderState* pRenderStateCache )
//{
	// pass render cache through to this call and grab the parameter pointers that
	// are updated by render cahce
// 	for ( hUint32 i = 0; i != nShaderParameters_; ++i )
// 	{
// 		pShaderParameters_[i].pName_ = pImpl()->GetParameterName( pShaderParameters_[i].guidIndex_ );
// 		void* volatileData = pRenderStateCache->GetSemanticIDParameterData( pShaderParameters_[i].semanticID_ );
// 		if ( volatileData )
// 		{
// 			pShaderParameters_[i].paramData_ = (float*)volatileData;
// 		}
// 		else
// 		{
// 			pShaderParameters_[i].paramData_ = (float*)(parameterData_ + (hUint32)pShaderParameters_[i].paramData_);
// 		}
// 		//force a first time set
// 		pShaderParameters_[i].dirty_ = true;
// 	}
//}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMaterial::SetFloatArrayParameter( const hShaderParameter* param, hFloat* val, hUint32 nCount )
{
// 	hcAssert( hRenderer::IsRenderThread() );
// 	hcAssertMsg( param->paramSize_ >= nCount*sizeof(float), "Data overflow setting shader parameter \"%s\"", param->pName_ );
// 	memcpy( pShaderParameters_[param->guidIndex_].paramData_, val, param->paramSize_ );
// 	param->dirty_ = true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMaterial::BindTextureParameter( const hShaderParameter* param, const hResourceHandle< hTextureBase >& tex )
{
// 	if ( param->boundTexture_.HasData() )
// 	{
// 		param->boundTexture_.Release();
// 	}
// 	tex.Acquire();
// 	//bit of a kludge 
// 	pShaderParameters_[param->guidIndex_].boundTexture_ = tex;
// 	pShaderParameters_[param->guidIndex_].dirty_ = true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMaterial::Release()
{
//	pRenderer_->NewRenderCommand< Cmd::ReleaseMaterial >( this );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const hShaderParameter* hMaterial::GetShaderParameter( const hChar* name ) const
{
// 	for ( hUint32 i = 0; i < nShaderParameters_; ++i )
// 	{
// 		if ( strcmp( name, pShaderParameters_[i].pName_ ) == 0 )
// 		{
// 			return &pShaderParameters_[i];
// 		}
// 	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const hShaderParameter* hMaterial::GetShaderParameter( EffectSemantics::EffectSemanticID id ) const
{
// 	for ( hUint32 i = 0; i < nShaderParameters_; ++i )
// 	{
// 		if ( id == pShaderParameters_[i].semanticID_ )
// 		{
// 			return &pShaderParameters_[i];
// 		}
// 	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const hShaderParameter* hMaterial::GetShaderParameter( hUint32 index ) const
{
// 	if ( index < nShaderParameters_ )
// 	{
// 		return &pShaderParameters_[index];
// 	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMaterial::FindOrAddShaderParameter( const hShaderParameter& newParam )
{
    hUint32 size = constParameters_.GetSize();
    for ( hUint32 i = 0; i < size; ++i )
    {
        if ( Heart::hStrCmp( constParameters_[i].name_, newParam.name_ ) == 0 )
        {
            hcAssertMsg( constParameters_[i].cBuffer_ == newParam.cBuffer_ && 
                         constParameters_[i].cReg_ == newParam.cReg_,
                         "Shader Const Parameter registers must match across material techniques and passes." );
            return;
        }
    }

    constParameters_.PushBack( newParam );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMaterial::Serialise( hSerialiser* ser ) const
{
    SERIALISE_ELEMENT( techniques_ );
    SERIALISE_ELEMENT( samplers_ );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMaterial::Deserialise( hSerialiser* ser )
{
    DESERIALISE_ELEMENT( techniques_ );
    DESERIALISE_ELEMENT( samplers_ );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMaterialTechniquePass::DefaultState()
{
    alphaBlendEnable_ = RSV_DISABLE;
    alphaSrcBlend_ = RSV_BLEND_OP_ONE;
    alphaDstBlend_ = RSV_BLEND_OP_ZERO;
    alphaBlendFunction_ = RSV_BLEND_FUNC_ADD;
    zTestEnable_ = RSV_DISABLE;
    zWriteEnable_ = RSV_DISABLE;
    zCompareFunction_ = RSV_Z_CMP_LESS;
    cullMode_ = RSV_CULL_MODE_NONE;
    fillMode_ =RSV_FILL_MODE_SOLID;
    stencilTest_ = RSV_DISABLE;
    stencilFail_ = RSV_SO_KEEP;
    stencilZFail_ = RSV_SO_KEEP;
    stencilPass_ = RSV_SO_KEEP;
    stencilFunc_ = RSV_SF_CMP_ALWAYS;
    stencilRef_ = 0;
    stencilWriteMask_ = 0xFFFFFFFF;
    stencilMask_ = 0xFFFFFFFF;
    colourMask1_ = RSV_COLOUR_WRITE_FULL;
    colourMask2_ = RSV_COLOUR_WRITE_FULL;
    colourMask3_ = RSV_COLOUR_WRITE_FULL;
    colourMask4_ = RSV_COLOUR_WRITE_FULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hSamplerParameter::DefaultState()
{
    semanticID_ = EffectSemantics::EFFECTSEMANTICID_MAX;
    semanticName_[0] = 0;
    boundTexture_ = NULL;
    borderColour_ = WHITE;
    minFilter_ = SSV_POINT;
    magFilter_ = SSV_POINT;
    mipFilter_ = SSV_POINT;
    addressU_ = SSV_CLAMP;
    addressV_ = SSV_CLAMP;
    addressW_ = SSV_CLAMP;
    mipLODBias_ = 0.f;
    maxAnisotropy_ = 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMaterialTechnique::Serialise( hSerialiser* ser ) const
{
    SERIALISE_ELEMENT( name_ );
    SERIALISE_ELEMENT( passes_ );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hMaterialTechnique::Deserialise( hSerialiser* ser )
{
    DESERIALISE_ELEMENT( name_ );
    DESERIALISE_ELEMENT( passes_ );
}

}//Heart
