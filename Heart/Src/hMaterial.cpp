/********************************************************************

	filename: 	hMaterial.cpp	
	
	Copyright (c) 1:4:2012 James Moran
	
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

    static const hUint32 VIEWPORT_CONST_BUFFER_ID = hCRC32::StringCRC( "ViewportConstants" );
    static const hUint32 INSTANCE_CONST_BUFFER_ID = hCRC32::StringCRC( "InstanceConstants" );

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hShaderParameter* hMaterial::GetShaderParameter( const hChar* name ) const
    {
        hUint32 count = constParameters_.GetSize();
	    for ( hUint32 i = 0; i < count; ++i )
	    {
		    if ( hStrCmp( name, constParameters_[i].name_.GetBuffer() ) == 0 )
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
     	hcAssert( index < constParameters_.GetSize() );
     	
        return &constParameters_[index];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterial::~hMaterial()
    {
        for (hUint32 i = 0, c = techniques_.GetSize(); i < c; ++i)
        {
            for (hUint32 i2 = 0, c2 = techniques_[i].GetPassCount(); i2 < c2; ++i2)
            {
                techniques_[i].GetPass(i2)->ReleaseResources();
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::FindOrAddShaderParameter( const hShaderParameter& newParam, const hFloat* defaultVal )
    {
        if ( newParam.cBuffer_ == VIEWPORT_CONST_BUFFER_ID || newParam.cBuffer_ == INSTANCE_CONST_BUFFER_ID )
            return;

        hUint32 constBufIdx = FindConstBufferIndexFromID( newParam.cBuffer_ );
        hUint32 size = constParameters_.GetSize();
        for ( hUint32 i = 0; i < size; ++i )
        {
            if ( Heart::hStrCmp( constParameters_[i].name_.GetBuffer(), newParam.name_.GetBuffer() ) == 0 )
            {
                hcAssertMsg( constParameters_[i].cBuffer_ == constBufIdx && 
                             constParameters_[i].cReg_ == newParam.cReg_,
                             "Shader Const Parameter registers must match across material techniques and passes." );
                return;
            }
        }

        hShaderParameter copyParam = newParam;
        copyParam.cBuffer_ = constBufIdx;
        constParameters_.PushBack( copyParam );

        if ( defaultVal )
        {
            // Add the default value
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::AddConstBufferDesc( const hChar* name, hUint32 reg, hUint32 size )
    {
        hUint32 crc = hCRC32::StringCRC( name );

        if ( crc == VIEWPORT_CONST_BUFFER_ID || crc == INSTANCE_CONST_BUFFER_ID )
            return;

        hUint32 cbdCount = constBufferDescs_.GetSize();
        hBool added = hFalse;

        for ( hUint32 i = 0; i < cbdCount; ++i )
        {
            if ( crc == constBufferDescs_[i].nameCRC_ )
            {
                hcAssert( constBufferDescs_[i].reg_ == reg );
                constBufferDescs_[i].size_ = hMax( constBufferDescs_[i].size_, size );
                added = hTrue;
            }
            
        }

        if ( !added )
        {
            hConstBufferDesc desc;
            desc.nameCRC_   = crc;
            desc.reg_       = reg;
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

    hMaterialTechnique* hMaterial::GetTechniqueByName( const hChar* name )
    {
        for ( hUint32 i = 0; i < techniques_.GetSize(); ++i )
        {
            if ( hStrCmp( techniques_[i].GetName(), name ) == 0 )
            {
                return &techniques_[i];
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialTechnique* hMaterial::GetTechniqueByMask( hUint32 mask )
    {
        for ( hUint32 i = 0; i < techniques_.GetSize(); ++i )
        {
            if ( techniques_[i].GetMask() == mask )
            {
                return &techniques_[i];
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialInstance* hMaterial::CreateMaterialInstance()
    {
        return hNEW(hGeneralHeap, hMaterialInstance(this, pRenderer_));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::DestroyMaterialInstance( hMaterialInstance* inst )
    {
        hDELETE(hGeneralHeap, inst);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSamplerParameter::DefaultState()
    {
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
        hUint32* regs = (hUint32*)hAlloca( cbCount*sizeof(hUint32) );
        for ( hUint32 i = 0; i < cbCount; ++i )
        {
            sizes[i] = parentMat->GetConstantBufferSize( i );
            regs[i] = parentMat->GetConstantBufferRegister( i );
        }
        constBuffers_ = renderer->CreateConstantBuffers( sizes, regs, parentMat->GetConstantBufferCount() );
        parentMaterial_->GetSamplerArray().CopyTo( &samplers_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialInstance::~hMaterialInstance()
    {
        renderer_->DestroyConstantBuffers(constBuffers_, parentMaterial_->GetConstantBufferCount());
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialInstance::SetShaderParameter( const hShaderParameter* param, hFloat* val, hUint32 size )
    {
        hcAssert( param && val && size );
        hcAssert( param >= parentMaterial_->GetShaderParameterByIndex(0) && 
                  param <= parentMaterial_->GetShaderParameterByIndex(parentMaterial_->GetShaderParameterCount()-1) );

        hdParameterConstantBlock& cb = constBuffers_[param->cBuffer_];
        hFloat* dst = cb.GetBufferAddress() + param->cReg_;
        for ( hUint32 i = 0; i < size && i < param->size_; ++i )
        {
            dst[i] = val[i];
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hSamplerParameter* hMaterialInstance::GetSamplerParameterByName( const hChar* name )
    {
        hUint32 count = samplers_.GetSize();
        for ( hUint32 i = 0; i < count; ++i )
        {
            if ( hStrCmp( samplers_[i].name_, name ) == 0 )
            {
                return &samplers_[i];
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialInstance::SetSamplerParameter( const hSamplerParameter* param, hTexture* tex )
    {
        hSamplerParameter* p = const_cast< hSamplerParameter* >( param );
        if ( p->boundTexture_ )
        {
            p->boundTexture_->DecRef();
        }
        if ( tex )
        {
            tex->AddRef();
        }
        p->boundTexture_ = tex;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialTechniquePass::ReleaseResources()
    {
        HEART_RESOURCE_SAFE_RELEASE(vertexProgram_);
        HEART_RESOURCE_SAFE_RELEASE(fragmentProgram_);
    }

}//Heart
