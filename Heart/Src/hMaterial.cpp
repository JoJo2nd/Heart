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
        hUint32 constBufIdx = FindConstBufferIndexFromID( newParam.cBuffer_ );
        hUint32 size = constParameters_.GetSize();
        for ( hUint32 i = 0; i < size; ++i )
        {
            if ( Heart::hStrCmp( constParameters_[i].name_, newParam.name_ ) == 0 )
            {
                hcAssertMsg( constParameters_[i].cBuffer_ == constBufIdx && 
                             constParameters_[i].cReg_ == newParam.cReg_,
                             "Shader Const Parameter registers must match across material techniques and passes." );
                return;
            }
        }

        hShaderParameter copyParam = newParam;
        copyParam.cBuffer_ = constBufIdx;
        constParameters_.PushBack( newParam );

        if ( defaultVal )
        {
            // Add the default value
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::AddConstBufferDesc( const hChar* name, hUint32 size )
    {
        hUint32 crc = hCRC32::StringCRC( name );
        hUint32 cbdCount = constBufferDescs_.GetSize();
        hBool added = hFalse;

        for ( hUint32 i = 0; i < cbdCount; ++i )
        {
            if ( crc == constBufferDescs_[i].nameCRC_ )
            {
                constBufferDescs_[i].size_ = hMax( constBufferDescs_[i].size_, size );
                added = hTrue;
            }
            
        }

        if ( !added )
        {
            hConstBufferDesc desc;
            desc.nameCRC_   = crc;
            desc.size_      = size;
            constBufferDescs_.PushBack( desc );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hMaterial::FindConstBufferIndexFromID( hUint32 id )
    {
        hUint32 size = constBufferDescs_.GetSize();
        for ( hUint32 i = 0; i < size; ++i )
        {
            if ( id == constBufferDescs_[i].nameCRC_ )
            {
                return i;
            }
        }

        hcAssertFailMsg( "Can't find constant buffer to match id 0x%08X", id );
        return 0;
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
        hUint32 cbCount = parentMat->GetConstantBufferCount();
        hUint32* sizes = (hUint32*)hAlloca( cbCount*sizeof(hUint32) );
        for ( hUint32 i = 0; i < cbCount; ++i )
        {
            sizes[i] = parentMat->GetConstantBufferSize( i );
        }
        renderer->CreateConstantBuffers( sizes, parentMat->GetConstantBufferCount() );
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
