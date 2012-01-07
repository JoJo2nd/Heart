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

    const hShaderParameter* hMaterial::GetShaderParameter( const hChar* name ) const
    {
        hUint32 count = constParameters_.GetSize();
	    for ( hUint32 i = 0; i < count; ++i )
	    {
		    if ( strcmp( name, constParameters_[i].name_ ) == 0 )
		    {
			    return &constParameters_[i];
		    }
	    }
	    return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hShaderParameter* hMaterial::GetShaderParameterByIndex( hUint32 index ) const
    {
     	hcAssert( index < constParameters_.GetSize() )
     	
        return &constParameters_[index];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::FindOrAddShaderParameter( const hShaderParameter& newParam, const hFloat* defaultVal )
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

        if ( defaultVal )
        {
            // Add the default value
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::AddConstBufferDesc( hUint32 idx, hUint32 size )
    {
        if ( constBufferSizes_.GetSize() <= idx )
        {
            constBufferSizes_.Resize( idx+1 );
            constBufferSizes_[idx] = size;
        }
        else
        {
            hcAssertMsg( constBufferSizes_[idx] == size, "Constant buffers must match across techniques" );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialTechniquePass::DefaultState()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSamplerParameter::DefaultState()
    {
        semanticID_ = EffectSemantics::EFFECTSEMANTICID_MAX;
        semanticName_[0] = 0;
        boundTexture_ = NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialInstance::hMaterialInstance( hMaterial* parentMat, hRenderer* renderer ) 
        : renderer_(renderer)
        , parentMaterial_(parentMat)
    {
        hcAssert( parentMat );
        renderer->CreateConstantBuffers( parentMat->GetConstantBufferSizes(), parentMat->GetConstantBufferCount() );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialInstance::~hMaterialInstance()
    {
        
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialInstance::SetShaderParameter( const hShaderParameter* param, hFloat* val, hUint32 size )
    {
        hcAssert( param && val && size );
        hcAssert( param >= parentMaterial_->GetShaderParameterByIndex(0) && 
                  param < parentMaterial_->GetShaderParameterByIndex(parentMaterial_->GetShaderParameterCount()-1) );

        hdParameterConstantBlock& cb = constBuffers_[param->cBuffer_];
        hFloat* dst = cb.GetBufferAddress() + param->cReg_;
        for ( hUint32 i = 0; i < size; ++i )
        {
            dst[i] = val[i];
        }
    }

}//Heart
