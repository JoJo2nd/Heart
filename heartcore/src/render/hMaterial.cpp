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

    HEARTCORE_SLIBEXPORT
    hUint32 HEART_API hFloatToFixed(hFloat input, hUint32 totalbits, hUint32 fixedpointbits)
    {
        hUint32 factor = 1 << fixedpointbits;
        hUint32 fp = (hUint32)(input*factor);
        
        //discard the LSB, this means that fixedpointbits will be reduced
        return fp >> (32-totalbits);
    }

    /*
     * See - http://realtimecollisiondetection.net/blog/?p=86/
     * MSB is our leading sort priority, LSB is last sort priority.
     * Our key is using 
     * 63 - 60 = camera / viewport / render target (0-15) - 4 bits = 64
     * 59 - 54 = sort layer (0-15) - 4 bits = 60
     * 53      = transparent flag - force transparent bits to end - 1 bits = 56
     * 52 - 29 = depth - 32 bits as 16:16 fixed point number - 32 bits = 55
     * 28 - 4  = material ID            - 19 bits = 23
     *  3 - 0  = material pass ID (0-15) - 4 bits = 4
     *  
     *  NOTE:
     *  When transparent material ID & pass are swapped with depth.
     */
    typedef hUint64 hMaterialSortKey;

    HEARTCORE_SLIBEXPORT
    hMaterialSortKey HEART_API hBuildRenderSortKey(hByte cameraID, hByte sortLayer, hBool transparent, hFloat viewSpaceDepth, hUint32 materialID, hByte pass)
    {
        hUint64 msb1  = (hUint64)(cameraID&0xF)  << 60;
        hUint64 msb2  = (hUint64)(sortLayer&0xF) << 56;
        hUint32 msb3  = (hUint64)(transparent&0x1) << 55;
        hUint64 msb4  = (hUint64)(hFloatToFixed(viewSpaceDepth,32,16)&0xFFFFFFFF) << 19;
        hUint64 msb4t = (hUint64)((~hFloatToFixed(viewSpaceDepth,32,16))&0xFFFFFFFF) << 19;
        hUint64 msb5  = (hUint64)(materialID&0x3FFFF) << 4;
        hUint64 msb6  = (hUint64)(pass&0xF);
        return transparent ? (msb1 | msb2 | msb3 | msb4 | msb5 | msb6) : (msb1 | msb2 | msb3 | msb5 | msb6 | msb4t);
    }

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
//         for (hUint32 i = 0, c = activeTechniques_->GetSize(); i < c; ++i)
//         {
//             for (hUint32 i2 = 0, c2 = activeTechniques_[i]->GetPassCount(); i2 < c2; ++i2)
//             {
//                 activeTechniques_[i].GetPass(i2)->ReleaseResources(renderer_);
//             }
//         }

        if (renderer_)
        {
            for (hUint32 i = 0, c = samplers_.GetSize(); i < c; ++i)
            {
                renderer_->DestroySamplerState(samplers_[i].samplerState_);
                samplers_[i].samplerState_ = NULL;

                 if (samplers_[i].boundTexture_)
                 {
                     samplers_[i].boundTexture_->DecRef();
                 }
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
        for ( hUint32 i = 0; i < activeTechniques_->GetSize(); ++i )
        {
            if ( hStrCmp( (*activeTechniques_)[i].GetName(), name ) == 0 )
            {
                return &(*activeTechniques_)[i];
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialTechnique* hMaterial::GetTechniqueByMask( hUint32 mask )
    {
        for ( hUint32 i = 0; i < activeTechniques_->GetSize(); ++i )
        {
            if ( (*activeTechniques_)[i].GetMask() == mask )
            {
                return &(*activeTechniques_)[i];
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialInstance* hMaterial::CreateMaterialInstance()
    {
        return hNEW(GetGlobalHeap()/*!heap*/, hMaterialInstance)(this, renderer_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::DestroyMaterialInstance( hMaterialInstance* inst )
    {
        hDELETE(GetGlobalHeap()/*!heap*/, inst);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialGroup* hMaterial::AddGroup( const hChar* name )
    {
        hMaterialGroup newGroup;
        hStrCopy(newGroup.name_.GetBuffer(), newGroup.name_.GetMaxSize(), name);
        groups_.PushBack(newGroup);
        return &groups_[groups_.GetSize()-1];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::SetActiveGroup( const hChar* name )
    {
        hUint32 groups = groups_.GetSize();
        for ( hUint32 i = 0; i < groups; ++i)
        {
            if (hStrICmp(name, groups_[i].name_) == 0)
            {
                activeTechniques_ = &groups_[i].techniques_;
                return;
            }
        }

        hcAssertFailMsg("Couldn't find group %s", name);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::AddSamplerParameter( const hSamplerParameter& sampler )
    {
        samplers_.PushBack(sampler);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::Link(hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager)
    {
        hUint32 nSamp = samplers_.GetSize();
        for ( hUint32 samp = 0; samp < nSamp; ++samp )
        {
            samplers_[samp].boundTexture_ = static_cast< hTexture* >( resManager->ltGetResource(samplers_[samp].defaultTextureID_) );
            if (samplers_[samp].defaultTextureID_)
            {
                if ((samplers_[samp].boundTexture_ && !samplers_[samp].samplerState_))
                {
                    samplers_[samp].samplerState_ = renderer->CreateSamplerState( samplers_[samp].samplerDesc_ );
                }
                else
                {
                    return hTrue;
                }
            }
            else
            {
                if (!samplers_[samp].samplerState_)
                {
                    samplers_[samp].samplerState_ = renderer->CreateSamplerState( samplers_[samp].samplerDesc_ );
                }
            }
        }

        activeTechniques_ = &groups_.GetBuffer()->techniques_;
        hUint32 nTech = activeTechniques_->GetSize();
        for ( hUint32 tech = 0; tech < nTech; ++tech )
        {
            if (!(*activeTechniques_)[tech].Link(resManager, renderer, matManager))
            {
                return hFalse;
            }

            hUint32 nPasses = (*activeTechniques_)[tech].GetPassCount();
            for(hUint32 pass = 0; pass < nPasses; ++pass)
            {
                hMaterialTechniquePass* passPtr = (*activeTechniques_)[tech].GetPass(pass);
                hShaderProgram* vp = passPtr->GetVertexShader();
                hShaderProgram* fp = passPtr->GetFragmentShader();

                hdShaderProgram* prog = vp->pImpl();
                for ( hUint32 i = 0; i < prog->GetConstantBufferCount(); ++i )
                {
                    AddConstBufferDesc( prog->GetConstantBufferName( i ), prog->GetConstantBufferReg( i ), prog->GetConstantBufferSize( i ) );
                }
                hUint32 parameterCount = vp->GetParameterCount();
                for ( hUint32 i = 0; i < parameterCount; ++i )
                {
                    hShaderParameter param;
                    hBool ok = prog->GetShaderParameter( i, &param );
                    hcAssertMsg( ok, "Shader Parameter Look up Out of Bounds" );
                    FindOrAddShaderParameter( param, prog->GetShaderParameterDefaultValue( i ) );
                }
                for ( hUint32 samp = 0; samp < nSamp; ++samp )
                {
                    hUint32 reg = prog->GetSamplerRegister( samplers_[samp].name_ );
                    if ( reg != ~0U )
                    {
                        hcAssert( reg == samplers_[samp].samplerReg_ || ~0U == samplers_[samp].samplerReg_ );
                        samplers_[samp].samplerReg_ = reg;
                    }
                }

                prog = fp->pImpl();
                for ( hUint32 i = 0; i < prog->GetConstantBufferCount(); ++i )
                {
                    AddConstBufferDesc( prog->GetConstantBufferName( i ), prog->GetConstantBufferReg( i ), prog->GetConstantBufferSize( i ) );
                }
                parameterCount = fp->GetParameterCount();
                for ( hUint32 i = 0; i < parameterCount; ++i )
                {

                    hShaderParameter param;
                    hBool ok = prog->GetShaderParameter( i, &param );
                    hcAssertMsg( ok, "Shader Parameter Look up Out of Bounds" );
                    FindOrAddShaderParameter( param, prog->GetShaderParameterDefaultValue( i ) );
                }
                for ( hUint32 samp = 0; samp < nSamp; ++samp )
                {
                    hUint32 reg = prog->GetSamplerRegister( samplers_[samp].name_ );
                    if ( reg != ~0U )
                    {
                        hcAssert( reg == samplers_[samp].samplerReg_ || ~0U == samplers_[samp].samplerReg_ );
                        samplers_[samp].samplerReg_ = reg;
                    }
                }
            }
        }

        //TODO: Create technique flags
        //matManager->OnMaterialLoad( resource, resID );
        return hTrue;
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
        constBuffers_ = renderer->CreateConstantBlocks( sizes, regs, parentMat->GetConstantBufferCount() );

        parentMaterial_->GetSamplerArray().CopyTo( &samplers_ );

        matKey_ = parentMaterial_->GetMatKey();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialInstance::~hMaterialInstance()
    {
        renderer_->DestroyConstantBlocks(constBuffers_, parentMaterial_->GetConstantBufferCount());
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
        p->boundTexture_ = tex;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialTechniquePass::ReleaseResources(hRenderer* renderer)
    {
        HEART_RESOURCE_SAFE_RELEASE(vertexProgram_);
        HEART_RESOURCE_SAFE_RELEASE(fragmentProgram_);
        if (blendState_)
            renderer->DestroyBlendState(blendState_);
        blendState_ = NULL;
        if (depthStencilState_)
            renderer->DestoryDepthStencilState(depthStencilState_);
        depthStencilState_ = NULL;
        if (rasterizerState_)
            renderer->DestoryRasterizerState(rasterizerState_);
        rasterizerState_ = NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::Link( hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager )
    {
        vertexProgram_   = static_cast< hShaderProgram* >( resManager->ltGetResource(vertexProgramID_) );
        fragmentProgram_ = static_cast< hShaderProgram* >( resManager->ltGetResource(fragmentProgramID_) );

        if (!vertexProgram_ || !fragmentProgram_) return hFalse;

        blendState_        = renderer->CreateBlendState(blendStateDesc_);
        rasterizerState_   = renderer->CreateRasterizerState(rasterizerStateDesc_);
        depthStencilState_ = renderer->CreateDepthStencilState(depthStencilStateDesc_);

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialTechnique::AppendPass( const hMaterialTechniquePass& pass )
    {
        passes_.PushBack(pass);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechnique::Link( hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager )
    {
        hBool linkok = hTrue;
        hUint32 nPasses = passes_.GetSize();
        mask_ = matManager->AddRenderTechnique( name_ )->mask_;
        for ( hUint32 pass = 0; pass < nPasses; ++pass )
        {
            linkok &= passes_[pass].Link(resManager, renderer, matManager);
        }

        return linkok;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hShaderProgram::CopyShaderBlob( void* blob, hUint32 len )
    {
        shaderProgram_ = (hByte*)hHeapRealloc(GetGlobalHeap(), shaderProgram_, len);
        hMemCpy(shaderProgram_, blob, len);
        shaderProgramLength_ = len;
    }

}//Heart
