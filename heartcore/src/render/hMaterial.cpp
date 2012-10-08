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

    hMaterial::~hMaterial()
    {
        if (renderer_)
        {
            /* - TODO: FIX ME
            for (hUint32 i = 0, c = samplers_.GetSize(); i < c; ++i)
            {
                renderer_->DestroySamplerState(samplers_[i].samplerState_);
                samplers_[i].samplerState_ = NULL;

                 if (samplers_[i].boundTexture_)
                 {
                     samplers_[i].boundTexture_->DecRef();
                 }
            }
            }*/
        }
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
        hMaterialInstance* mat = hNEW(GetGlobalHeap()/*!heap*/, hMaterialInstance)(this, renderer_);

        //Copy the samplers
        mat->samplers_.Reserve(samplers_.GetSize());
        mat->samplers_.Resize(samplers_.GetSize());
        for (hUint32 i = 0; i < samplers_.GetSize(); ++i)
        {
            hStrCopy(mat->samplers_[i].name_, mat->samplers_[i].name_.GetMaxSize(), samplers_[i].name_);
            mat->samplers_[i].boundTexture_     = samplers_[i].boundTexture_;
            mat->samplers_[i].defaultTextureID_ = samplers_[i].defaultTextureID_;
            mat->samplers_[i].samplerReg_       = samplers_[i].samplerReg_;
            mat->samplers_[i].samplerState_     = samplers_[i].samplerState_;
        }

        //Create constant blocks
        hUint32* sizes = (hUint32*)hAlloca(constBlockCount_*sizeof(hUint32));
        hUint32* p = sizes;
        for (hUint32 i = 0; i < HEART_MAX_CONSTANT_BLOCKS; ++i)
        {
            if (constantBlockSizes_[i] > 0)
            {
                *p = constantBlockSizes_[i];
                ++p;
            }
        }

        // Copy regs
        hMemCpy(mat->constantBlockRegs_, constantBlockRegs_, HEART_MAX_CONSTANT_BLOCKS);

        mat->constBufferCount_ = constBlockCount_;
        mat->constBuffers_ = renderer_->CreateConstantBlocks(sizes, mat->constBufferCount_);

        //Create CPU data
        mat->cpuDataSizeBytes_ = totalParameterDataSize_;
        mat->cpuData_ = (hByte*)hHeapMalloc(GetGlobalHeap(), totalParameterDataSize_);
        hZeroMem(mat->cpuData_, mat->cpuDataSizeBytes_);

        //Copy mappings
        mat->parameterMappingCount_ = defaultMappings_.GetSize();
        mat->parameterMappings_ = hNEW_ARRAY(GetGlobalHeap(), hParameterMapping, mat->parameterMappingCount_);
        hMemCpy(mat->parameterMappings_, defaultMappings_.GetBuffer(), sizeof(hParameterMapping)*mat->parameterMappingCount_);
        for (hUint32 i = 0; i < mat->parameterMappingCount_; ++i)
        {
            mat->parameterMappings_[i].cpuData_ = (mat->cpuData_)+(hUint32)mat->parameterMappings_[i].cpuData_;
        }

        // TODO: Copy/Set Defaults

        return mat;
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

    hBool hMaterial::Link(hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager)
    {
        /*
         * This code ain't pretty, so do it here in the link where we are on the loader
         * thread and have time to do it.
         **/
        //
        for (hUint32 i = 0; i < samplers_.GetSize(); ++i)
        {
            if (samplers_[i].defaultTextureID_)
            {
                samplers_[i].samplerReg_ = hErrorCode;
                samplers_[i].boundTexture_ = static_cast<hTexture*>(resManager->ltGetResource(samplers_[i].defaultTextureID_));
                //Not loaded yet...?
                if (samplers_[i].boundTexture_ == NULL)
                {
                    return hFalse;
                }
            }
        }

        // Grab all the shader programs
        for (hUint32 group = 0; group < groups_.GetSize(); ++group)
        {
            for (hUint32 tech = 0; tech < groups_[group].techniques_.GetSize(); ++tech)
            {
                groups_[group].techniques_[tech].mask_ = matManager->AddRenderTechnique( groups_[group].techniques_[tech].name_ )->mask_;
                for (hUint32 pass = 0; pass < groups_[group].techniques_[tech].passes_.GetSize(); ++pass)
                {
                    hMaterialTechniquePass* passptr = &(groups_[group].techniques_[tech].passes_[pass]);
                    passptr->vertexProgram_ = static_cast<hShaderProgram*>(resManager->ltGetResource(passptr->vertexProgramID_));
                    passptr->fragmentProgram_ = static_cast<hShaderProgram*>(resManager->ltGetResource(passptr->fragmentProgramID_));
                    if (passptr->vertexProgramID_ && !passptr->vertexProgram_)
                    {
                        return hFalse;
                    }
                    if (passptr->fragmentProgramID_ && !passptr->fragmentProgram_)
                    {
                        return hFalse;
                    }
                }
            }
        }

        // All resources linked...grab const block info
        hZeroMem(constantBlockSizes_, sizeof(constantBlockSizes_));
#ifdef HEART_DEBUG
        hZeroMem(constantBlockHashes_, sizeof(constantBlockHashes_));
#endif
        for (hUint32 group = 0; group < groups_.GetSize(); ++group)
        {
            for (hUint32 tech = 0; tech < groups_[group].techniques_.GetSize(); ++tech)
            {
                for (hUint32 pass = 0; pass < groups_[group].techniques_[tech].passes_.GetSize(); ++pass)
                {
                    hMaterialTechniquePass* passptr = &groups_[group].techniques_[tech].passes_[pass];
                    for (hUint32 progidx = 0; progidx < passptr->GetProgramCount(); ++progidx)
                    {
                        hShaderProgram* prog = passptr->GetProgram(progidx);
                        for (hUint32 cb = 0; prog && cb < prog->GetConstantBlockCount(); ++cb)
                        {
                            hConstantBlockDesc desc;
                            prog->GetConstantBlockDesc(cb, &desc);
                            if (desc.reg_ < HEART_MIN_RESERVED_CONSTANT_BLOCK)
                            {
                                constantBlockSizes_[desc.reg_] = desc.size_;
#ifdef HEART_DEBUG
                                hcAssertMsg(constantBlockHashes_[desc.reg_] == desc.hash_ || constantBlockHashes_[desc.reg_] == 0,
                                    "WARNING: Constant blocks don't match across shaders. This will produce undesirable results."
                                    "Make sure constant blocks assigned are to the same registers match accross all shaders in the same"
                                    "material.");
                                constantBlockHashes_[desc.reg_] = desc.hash_;
#endif // HEART_DEBUG
                            }
                        }
                        // Add Sampler register
                        for (hUint32 si = 0; si < samplers_.GetSize(); ++si)
                        {
                            if (samplers_[si].samplerReg_ == hErrorCode)
                            {
                                samplers_[si].samplerReg_ = prog->GetSamplerRegister(samplers_[si].name_);
                            }
                        }
                        // Add parameter mappings.
                        for (hUint32 sp = 0; prog && sp < prog->GetShaderParameterCount(); ++sp)
                        {
                            hShaderParameter param;
                            hBool found = hFalse;
                            prog->GetShaderParameter(sp, &param);
                            
                            for (hUint32 pm = 0, pms = defaultMappings_.GetSize(); pm < pms; ++pm)
                            {
                                if (param.cReg_ == defaultMappings_[pm].cOffset_ && param.cBuffer_ == defaultMappings_[pm].cBuffer_)
                                {
                                    found = hTrue;
                                    continue;
                                }
                            }

                            if (!found)
                            {
                                hProgramOutput* output = NULL;
                                //Find the material output that matches, 
                                for (hUint32 mo = 0; !output && mo < programOutputs_.GetSize(); ++mo)
                                {
                                    if (hStrCmp(programOutputs_[mo].name_, param.name_) == 0)
                                    {
                                        output = &programOutputs_[mo];
                                    }
                                }

                                if (output)
                                {
                                    hParameterMapping newmapping;
                                    newmapping.cBuffer_ = param.cBuffer_;
                                    newmapping.cOffset_ = param.cReg_;
                                    newmapping.sizeBytes_ = param.size_;
                                    newmapping.cpuData_ = (hByte*)materialParameters_[(hUint32)output->parameterID_].dataOffset_;

                                    defaultMappings_.PushBack(newmapping);
                                }
                            }
                        }
                    }
                }
            }
        }

        constBlockCount_ = 0;
        for (hUint32 i = 0; i < HEART_MAX_CONSTANT_BLOCKS; ++i)
        {
            if (constantBlockSizes_[i] > 0)
            {
                constantBlockRegs_[constBlockCount_] = i;
                ++constBlockCount_;
            }
        }

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::SetParameterInputOutputReserves( hUint32 totalIn, hUint32 totalOut )
    {
        materialParameters_.Reserve(totalIn);
        programOutputs_.Reserve(totalOut);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialParameterID hMaterial::AddMaterialParameter( hChar* name, hParameterType type )
    {
        //Don't want to grow.
        hcAssert(materialParameters_.GetSize() < materialParameters_.GetReserve());
        hMaterialParameterID id = materialParameters_.GetSize();
        hMaterialParameter param;
        hStrCopy(param.name_, hMAX_PARAMETER_NAME_LEN, name);
        param.type_ = type;
        param.dataOffset_ = totalParameterDataSize_;
        switch(type)
        {
        case ePTFloat1:
            totalParameterDataSize_ += sizeof(hFloat); break;
        case ePTFloat2:
            totalParameterDataSize_ += sizeof(hFloat)*2; break;
        case ePTFloat3:
            totalParameterDataSize_ += sizeof(hFloat)*3; break;
        case ePTFloat4:
            totalParameterDataSize_ += sizeof(hFloat)*4; break;
        case ePTFloat3x3:
            totalParameterDataSize_ += sizeof(hFloat)*9; break;
        case ePTFloat4x4:
            totalParameterDataSize_ += sizeof(hFloat)*16; break;
        }

        materialParameters_.PushBack(param);

        return id;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::AddProgramOutput( hChar* name, hMaterialParameterID parameterID )
    {
        hcAssert(programOutputs_.GetSize() < programOutputs_.GetReserve());

        hProgramOutput outp;
        hStrCopy(outp.name_, hMAX_PARAMETER_NAME_LEN, name);
        outp.parameterID_ = parameterID;

        programOutputs_.PushBack(outp);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::SetSamplerCount( hUint32 val )
    {
        samplers_.Reserve(val);
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

    hMaterialParameterID hMaterial::GetMaterialParameter(const hChar* name)
    {
        for (hUint32 i = 0; i < materialParameters_.GetSize(); ++i)
        {
            if (hStrCmp(materialParameters_[i].name_, name) == 0)
                return (hMaterialParameterID)i;
        }

        return (hMaterialParameterID)hErrorCode;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialInstance::hMaterialInstance( hMaterial* parentMat, hRenderer* renderer ) 
        : renderer_(renderer)
        , parentMaterial_(parentMat)
        , constBlockDirty_(0)
    {
        hcAssert( parentMat );
        matKey_ = parentMaterial_->GetMatKey();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialInstance::~hMaterialInstance()
    {
        hHeapFreeSafe(GetGlobalHeap(), cpuData_);
        hDELETE_ARRAY_SAFE(GetGlobalHeap(), parameterMappings_);
        renderer_->DestroyConstantBlocks(constBuffers_, constBufferCount_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
/*
    void hMaterialInstance::SetShaderParameter( const hShaderParameter* param, hFloat* val, hUint32 size )
    {
//         hcAssert( param && val && size );
//         hcAssert( param >= parentMaterial_->GetShaderParameterByIndex(0) && 
//                   param <= parentMaterial_->GetShaderParameterByIndex(parentMaterial_->GetShaderParameterCount()-1) );
// 
//         hdParameterConstantBlock& cb = constBuffers_[param->cBuffer_];
//         hFloat* dst = cb.GetBufferAddress() + param->cReg_;
//         for ( hUint32 i = 0; i < size && i < param->size_; ++i )
//         {
//             dst[i] = val[i];
//         }
    }
*/
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

    void hMaterialInstance::FlushParameters(hRenderSubmissionCtx* ctx)
    {
        hUint32 mask = 1;
        if (constBlockDirty_ == 0)
            return;

        // Update parameters
        for (hUint32 i = 0; i < parameterMappingCount_; ++i)
        {
            hMemCpy(((hByte*)constBuffers_[parameterMappings_[i].cBuffer_].mapData_)+parameterMappings_[i].cOffset_, parameterMappings_[i].cpuData_, parameterMappings_[i].sizeBytes_);
        }

        // Flush to const blocks
        for (hUint32 i = 0; i < constBufferCount_; ++i, mask <<= 1)
        {
            if (constBlockDirty_ & mask)
            {
                ctx->Update(constBuffers_+i);
            }
        }

        constBlockDirty_ = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialInstance::SetMaterialParameter( hMaterialParameterID param, const void* val, hUint32 size )
    {
        if (param == hErrorCode)
            return;

        hMemCpy(cpuData_+parentMaterial_->GetMaterialParameterData(param)->dataOffset_, val, size);
    }

}//Heart
