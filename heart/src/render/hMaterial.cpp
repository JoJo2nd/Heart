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

    HEART_DLLEXPORT
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

    HEART_DLLEXPORT
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
        hcAssertMsg(instanceCount_.value_ == 0, 
            "Not all material instances created from this material were released");

        for (hUint32 group = 0; group < groups_.GetSize(); ++group) {
            for (hUint32 tech = 0; tech < groups_[group].techniques_.GetSize(); ++tech) {
                for (hUint32 pass = 0; pass < groups_[group].techniques_[tech].passes_.GetSize(); ++pass) {
                    hMaterialTechniquePass* passptr = &(groups_[group].techniques_[tech].passes_[pass]);
                    passptr->ReleaseResources(renderer_);
                }
            }
        }
        for (hUint32 i = 0, c = constBlocks_.GetSize(); i < c; ++i) {
            hdParameterConstantBlock* globalCB = manager_->GetGlobalConstantBlockParameterID(constBlocks_[i].paramid);
            if (!globalCB) {
                //Created for material so delete
                renderer_->DestroyConstantBlocks(constBlocks_[i].constBlock, 1);
            }
        }
        if (renderer_) {
            for (hUint32 i = 0, c = defaultSamplers_.GetSize(); i < c; ++i) {
                renderer_->DestroySamplerState(defaultSamplers_[i].samplerState_);
                defaultSamplers_[i].samplerState_ = NULL;
            }
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

    void hMaterial::AddSamplerParameter(const hSamplerParameter& samp)
    {
        defaultSamplers_.PushBack(samp);
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
        renderer_ = renderer;
        manager_ = matManager;

        for (hUint32 i = 0; i < defaultSamplers_.GetSize(); ++i) {
            if (defaultSamplers_[i].defaultTextureID_) {
                defaultSamplers_[i].boundTexture_ = static_cast<hTexture*>(resManager->ltGetResource(defaultSamplers_[i].defaultTextureID_));
                //Not loaded yet...?
                if (defaultSamplers_[i].boundTexture_ == NULL) return hFalse;
            }
        }

        // Grab all the shader programs
        for (hUint32 group = 0; group < groups_.GetSize(); ++group) {
            for (hUint32 tech = 0; tech < groups_[group].techniques_.GetSize(); ++tech) {
                groups_[group].techniques_[tech].mask_ = matManager->AddRenderTechnique( groups_[group].techniques_[tech].name_ )->mask_;
                for (hUint32 pass = 0; pass < groups_[group].techniques_[tech].passes_.GetSize(); ++pass) {
                    hMaterialTechniquePass* passptr = &(groups_[group].techniques_[tech].passes_[pass]);
                    passptr->vertexProgram_ = static_cast<hShaderProgram*>(resManager->ltGetResource(passptr->vertexProgramID_));
                    passptr->fragmentProgram_ = static_cast<hShaderProgram*>(resManager->ltGetResource(passptr->fragmentProgramID_));
                    if (passptr->vertexProgramID_ && !passptr->vertexProgram_) return hFalse;
                    if (passptr->fragmentProgramID_ && !passptr->fragmentProgram_) return hFalse;
                }
            }
        }

        // All resources linked...
        // Bind programs 
        for (hUint32 group = 0; group < groups_.GetSize(); ++group) {
            for (hUint32 tech = 0; tech < groups_[group].techniques_.GetSize(); ++tech) {
                for (hUint32 pass = 0; pass < groups_[group].techniques_[tech].passes_.GetSize(); ++pass) {
                    hMaterialTechniquePass* passptr = &groups_[group].techniques_[tech].passes_[pass];
                    for (hUint32 progidx = 0; progidx < passptr->GetProgramCount(); ++progidx) {
                        passptr->BindShaderProgram(passptr->GetProgram(progidx));
                    }
                }
            }
        }
        // Grab and Bind const block info
        struct {
            hUint32 hash;
            hChar name[hMAX_PARAMETER_NAME_LEN];
            hUint32 size;
        }cbHashes[HEART_MAX_CONSTANT_BLOCKS] = {0};
        hUint32 cbcount = 0;
        for (hUint32 group = 0; group < groups_.GetSize(); ++group) {
            for (hUint32 tech = 0; tech < groups_[group].techniques_.GetSize(); ++tech) {
                for (hUint32 pass = 0; pass < groups_[group].techniques_[tech].passes_.GetSize(); ++pass) {
                    hMaterialTechniquePass* passptr = &groups_[group].techniques_[tech].passes_[pass];
                    for (hUint32 progidx = 0; progidx < passptr->GetProgramCount(); ++progidx) {
                        hShaderProgram* prog = passptr->GetProgram(progidx);
                        for (hUint32 cb = 0; prog && cb < prog->GetConstantBlockCount(); ++cb) {
                            hConstantBlockDesc desc;
                            prog->GetConstantBlockDesc(cb, &desc);
#ifdef HEART_DEBUG
                            hUint32 i;
                            for (i = 0; i < cbcount; ++i) {
                                if (hStrCmp(cbHashes[i].name, desc.name_) == 0 ){
                                    hcAssertMsg(cbHashes[i].hash, desc.hash_ == 0,
                                        "WARNING: Constant blocks don't match across shaders. This will produce undesirable results."
                                        "Make sure constant blocks, that share the same name, share the same variables across all shaders in the same"
                                        "material.");
                                }
                            }
                            if (i == cbcount){
                                hStrCopy(cbHashes[cbcount].name, 32, desc.name_); 
                                cbHashes[cbcount].hash = desc.hash_;
                                cbHashes[cbcount].size = desc.size_;
                                ++cbcount;
                                hcAssertMsg(cbcount < HEART_MAX_CONSTANT_BLOCKS, "Buffer overrun");
                            }
#endif // HEART_DEBUG
                            hShaderParameterID cbID = hCRC32::StringCRC(desc.name_);
                            hdParameterConstantBlock* globalCB = matManager->GetGlobalConstantBlockByAlias(desc.name_);
                            hBool alreadyAdded = hFalse;
                            if (!globalCB) {
                                for (hUint mcb=0, mcbc=constBlocks_.GetSize(); mcb < mcbc; ++mcb) {
                                    if (constBlocks_[mcb].paramid == cbID) {
                                        alreadyAdded = hTrue;
                                    }
                                }
                                if (!alreadyAdded) globalCB = renderer_->CreateConstantBlocks(&desc.size_, 1);
                            }
                            if (globalCB) BindConstanstBuffer(cbID, globalCB);
                        }
                    }
                }
            }
        }

        // Bind the default samplers to the programs
        for (hUint32 si = 0; si < defaultSamplers_.GetSize(); ++si) {
            BindTexture(hCRC32::StringCRC(defaultSamplers_[si].name_), defaultSamplers_[si].boundTexture_, defaultSamplers_[si].samplerState_);
        }

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::BindConstanstBuffer(hShaderParameterID id, hdParameterConstantBlock* cb)
    {
        hBool succ = true;
        for (hUint32 tech = 0, nTech = activeTechniques_->GetSize(); tech < nTech; ++tech) {
            for (hUint32 passIdx = 0, nPasses = (*activeTechniques_)[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                hMaterialTechniquePass* pass = (*activeTechniques_)[tech].GetPass(passIdx);
                succ &= pass->BindConstantBuffer(id, cb);
            }
        }

        if (succ) {
            hBool added = hFalse;
            for (hUint32 i = 0, c = constBlocks_.GetSize(); i < c; ++i) {
                if (constBlocks_[i].paramid == id) {
                    constBlocks_[i].constBlock = cb;
                    added = hTrue;
                }
            }
            if (!added) {
                hBoundConstBlock bcb;
                bcb.paramid = id;
                bcb.constBlock = cb;
                constBlocks_.PushBack(bcb);
            }
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::BindTexture(hShaderParameterID id, hTexture* tex, hdSamplerState* samplerState)
    {
        hBool succ = true;
        for (hUint32 tech = 0, nTech = activeTechniques_->GetSize(); tech < nTech; ++tech) {
            for (hUint32 passIdx = 0, nPasses = (*activeTechniques_)[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                hMaterialTechniquePass* pass = (*activeTechniques_)[tech].GetPass(passIdx);
                succ &= pass->BindSamplerInput(id, samplerState);
                succ &= pass->BindResourceView(id, tex);
            }
        }

        if (succ) {
            hBool added = hFalse;
            for (hUint32 i = 0, c = boundTextures_.GetSize(); i < c; ++i) {
                if (boundTextures_[i].paramid == id) {
                    boundTextures_[i].texture = tex;
                    boundTextures_[i].state = samplerState;
                    added = hTrue;
                }
            }
            if (!added) {
                hBoundTexture bt;
                bt.paramid = id;
                bt.texture = tex;
                bt.state = samplerState;
                boundTextures_.PushBack(bt);
            }
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdParameterConstantBlock* hMaterial::GetParameterConstBlock( hShaderParameterID cbid )
    {
        for (hUint32 i = 0, c = constBlocks_.GetSize(); i < c; ++i) {
            if (constBlocks_[i].paramid == cbid) {
                return constBlocks_[i].constBlock;
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialInstance* hMaterial::createMaterialInstance(hUint32 flags)
    {
        hBool dontcreatecb=(hMatInst_DontInstanceConstantBuffers&flags) == hMatInst_DontInstanceConstantBuffers;;
        hMaterialInstance* matInst = hNEW(memHeap_, hMaterialInstance) (memHeap_, this);
        activeTechniques_->CopyTo(&matInst->techniques_);

        for (hUint32 group = 0; group < groups_.GetSize(); ++group) {
            for (hUint32 tech = 0; tech < groups_[group].techniques_.GetSize(); ++tech) {
                for (hUint32 pass = 0; pass < groups_[group].techniques_[tech].passes_.GetSize(); ++pass) {
                    hMaterialTechniquePass* passptr = &groups_[group].techniques_[tech].passes_[pass];
                    for (hUint32 progidx = 0; progidx < passptr->GetProgramCount(); ++progidx) {
                        hShaderProgram* prog = passptr->GetProgram(progidx);
                        for (hUint32 cb = 0; prog && cb < prog->GetConstantBlockCount(); ++cb) {
                            hConstantBlockDesc desc;
                            prog->GetConstantBlockDesc(cb, &desc);
                            hShaderParameterID cbID = hCRC32::StringCRC(desc.name_);
                            hdParameterConstantBlock* globalCB = manager_->GetGlobalConstantBlockByAlias(desc.name_);
                            hBool alreadyAdded = hFalse;
                            if (!globalCB) {
                                for (hUint mcb=0, mcbc=matInst->constBlocks_.GetSize(); mcb < mcbc; ++mcb) {
                                    if (matInst->constBlocks_[mcb].paramid == cbID) {
                                        alreadyAdded = hTrue;
                                    }
                                }
                                if (!alreadyAdded && !dontcreatecb) globalCB = renderer_->CreateConstantBlocks(&desc.size_, 1);
                            }
                            if (globalCB) matInst->BindConstanstBuffer(cbID, globalCB);
                        }
                    }
                }
            }
        }

        // Bind the default samplers to the programs
        for (hUint32 si = 0; si < defaultSamplers_.GetSize(); ++si) {
            matInst->BindTexture(hCRC32::StringCRC(defaultSamplers_[si].name_), defaultSamplers_[si].boundTexture_, defaultSamplers_[si].samplerState_);
        }

        hAtomic::Increment(instanceCount_);
        matInst->flags_=flags;
        return matInst;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::destroyMaterialInstance(hMaterialInstance* matInst)
    {
        hcAssert(matInst);
        hBool didntcreatecb=(hMatInst_DontInstanceConstantBuffers&matInst->flags_) == hMatInst_DontInstanceConstantBuffers;
        for (hUint32 i = 0, c = matInst->constBlocks_.GetSize(); i < c; ++i) {
            hdParameterConstantBlock* globalCB = manager_->GetGlobalConstantBlockParameterID(matInst->constBlocks_[i].paramid);
            if (!globalCB && !didntcreatecb) { //Created for material so delete
                renderer_->DestroyConstantBlocks(matInst->constBlocks_[i].constBlock, 1);
            }
        }
        hDELETE_SAFE(memHeap_, matInst);
        hAtomic::Decrement(instanceCount_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialInstance::BindConstanstBuffer(hShaderParameterID id, hdParameterConstantBlock* cb)
    {
        hcAssert(id);
        hcAssert(cb);
        hBool succ = true;
        for (hUint32 tech = 0, nTech = techniques_.GetSize(); tech < nTech; ++tech) {
            for (hUint32 passIdx = 0, nPasses = techniques_[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                hMaterialTechniquePass* pass = techniques_[tech].GetPass(passIdx);
                succ &= pass->BindConstantBuffer(id, cb);
            }
        }

        if (succ) {
            hBool added = hFalse;
            for (hUint32 i = 0, c = constBlocks_.GetSize(); i < c; ++i) {
                if (constBlocks_[i].paramid == id) {
                    constBlocks_[i].constBlock = cb;
                    added = hTrue;
                }
            }
            if (!added) {
                hBoundConstBlock bcb;
                bcb.paramid = id;
                bcb.constBlock = cb;
                constBlocks_.PushBack(bcb);
            }
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialInstance::BindTexture(hShaderParameterID id, hTexture* tex, hdSamplerState* samplerState)
    {
        hBool succ = true;
        for (hUint32 tech = 0, nTech = techniques_.GetSize(); tech < nTech; ++tech) {
            for (hUint32 passIdx = 0, nPasses = techniques_[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                hMaterialTechniquePass* pass = techniques_[tech].GetPass(passIdx);
                succ &= pass->BindSamplerInput(id, samplerState);
                succ &= pass->BindResourceView(id, tex);
            }
        }

        if (succ) {
            hBool added = hFalse;
            for (hUint32 i = 0, c = boundTextures_.GetSize(); i < c; ++i) {
                if (boundTextures_[i].paramid == id) {
                    boundTextures_[i].texture = tex;
                    boundTextures_[i].state = samplerState;
                    added = hTrue;
                }
            }
            if (!added) {
                hBoundTexture bt;
                bt.paramid = id;
                bt.texture = tex;
                bt.state = samplerState;
                boundTextures_.PushBack(bt);
            }
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialInstance::bindInputStreams(PrimitiveType type, hIndexBuffer* idx, hVertexBuffer** vtxs, hUint streamCnt) {
        hBool succ = true;
        for (hUint32 tech = 0, nTech = techniques_.GetSize(); tech < nTech; ++tech) {
            for (hUint32 passIdx = 0, nPasses = techniques_[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                techniques_[tech].GetPass(passIdx)->bindInputStreams(type, idx, vtxs, streamCnt);
            }
        }
        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialInstance::bindVertexStream(hUint inputSlot, hVertexBuffer* vtxBuf) {
        hBool succ = true;
        for (hUint32 tech = 0, nTech = techniques_.GetSize(); tech < nTech; ++tech) {
            for (hUint32 passIdx = 0, nPasses = techniques_[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                techniques_[tech].GetPass(passIdx)->bindInputStream(inputSlot, vtxBuf);
            }
        }
        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdParameterConstantBlock* hMaterialInstance::GetParameterConstBlock(hShaderParameterID cbid)
    {
        for (hUint32 i = 0, c = constBlocks_.GetSize(); i < c; ++i) {
            if (constBlocks_[i].paramid == cbid) {
                return constBlocks_[i].constBlock;
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialTechnique* hMaterialInstance::GetTechniqueByName( const hChar* name )
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

    hMaterialTechnique* hMaterialInstance::GetTechniqueByMask( hUint32 mask )
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

    void hMaterialInstance::destroyMaterialInstance(hMaterialInstance* inst)
    {
        if (!inst) return;
        hcAssertMsg(inst->material_, "Parent material is NULL");
        inst->material_->destroyMaterialInstance(inst);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hMaterialInstance::getMaterialKey() const {
        return material_->GetMatKey();
    }

}//Heart
