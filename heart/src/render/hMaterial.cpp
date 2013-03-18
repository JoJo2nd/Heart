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
        hUint64 msb3  = (hUint64)(transparent&0x1) << 55;
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

    hMaterial::hMaterial(hMemoryHeapBase* heap, hRenderer* renderer) : memHeap_(heap)
        , renderer_(renderer)
        , activeTechniques_(NULL)
        , manager_(renderer->GetMaterialManager())
        , groups_(heap) 
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterial::~hMaterial()
    {
        hcAssertMsg(instanceCount_.value_ == 0, 
            "Not all material instances created from this material were released");
        unbind();
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
            if (defaultSamplers_[i].defaultTextureID_ && defaultSamplers_[i].boundTexture_==NULL) {
                defaultSamplers_[i].boundTexture_ = static_cast<hTexture*>(resManager->ltGetResource(defaultSamplers_[i].defaultTextureID_));
                //Not loaded yet...?
                if (defaultSamplers_[i].boundTexture_ == NULL) {
                    return hFalse;
                } else {
                    defaultSamplers_[i].boundTexture_->AddRef();
                    hShaderResourceViewDesc rsvd;
                    hZeroMem(&rsvd, sizeof(rsvd));
                    //TODO: this needs to check how this handles multiple formats
                    hcAssert(defaultSamplers_[i].boundTexture_->getRenderType() == eRenderResourceType_Tex2D);
                    rsvd.format_ = defaultSamplers_[i].boundTexture_->getTextureFormat();
                    rsvd.resourceType_ = defaultSamplers_[i].boundTexture_->getRenderType();
                    rsvd.tex2D_.topMip_ = 0;
                    rsvd.tex2D_.mipLevels_ = ~0;
                    renderer_->createShaderResourceView(defaultSamplers_[i].boundTexture_, rsvd, &defaultSamplers_[i].resView_);
                }
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
        return bindMaterial(matManager);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::bindConstanstBuffer(hShaderParameterID id, hParameterConstantBlock* cb)
    {
        hcAssert(activeTechniques_);
        hBool succ = true;
        for (hUint32 tech = 0, nTech = activeTechniques_->GetSize(); tech < nTech; ++tech) {
            for (hUint32 passIdx = 0, nPasses = (*activeTechniques_)[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                hMaterialTechniquePass* pass = (*activeTechniques_)[tech].GetPass(passIdx);
                succ &= pass->setConstantBuffer(id, cb);
            }
        }

        if (succ) {
            hBool added = hFalse;
            for (hUint32 i = 0, c = constBlocks_.GetSize(); i < c; ++i) {
                if (constBlocks_[i].paramid == id) {
                    if (cb) {
                        cb->AddRef();
                    }
                    if (constBlocks_[i].constBlock) {
                        constBlocks_[i].constBlock->DecRef();
                    }
                    constBlocks_[i].constBlock = cb;
                    added = hTrue;
                }
            }
            if (!added && cb) {
                hBoundConstBlock bcb;
                bcb.paramid = id;
                bcb.constBlock = cb;
                cb->AddRef();
                constBlocks_.PushBack(bcb);
            }
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hParameterConstantBlock* hMaterial::GetParameterConstBlock( hShaderParameterID cbid )
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
        matInst->techniques_=*activeTechniques_;

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
                            hParameterConstantBlock* globalCB = manager_->GetGlobalConstantBlockByAlias(desc.name_);
                            hParameterConstantBlock* tmp=NULL;
                            hBool alreadyAdded = hFalse;
                            if (!globalCB) {
                                for (hUint mcb=0, mcbc=matInst->constBlocks_.GetSize(); mcb < mcbc; ++mcb) {
                                    if (matInst->constBlocks_[mcb].paramid == cbID) {
                                        alreadyAdded = hTrue;
                                    }
                                }
                                if (!alreadyAdded && !dontcreatecb) {
                                    void* initdata=hAlloca(desc.size_);
                                    initConstBlockBufferData(prog, desc, initdata, desc.size_);
                                    renderer_->createConstantBlock(desc.size_, &initdata, &tmp);
                                    globalCB=tmp;
                                }
                            }
                            if (globalCB) {
                                matInst->bindConstanstBuffer(cbID, globalCB);
                            }
                            if (tmp) {
                                tmp->DecRef();
                            }
                        }
                    }
                }
            }
        }

        // Bind the default samplers to the programs
        for (hUint32 si = 0; si < defaultSamplers_.GetSize(); ++si) {
            hShaderParameterID paramid = hCRC32::StringCRC(defaultSamplers_[si].name_);
            matInst->bindResource(paramid, defaultSamplers_[si].resView_);
            matInst->bindSampler(paramid, defaultSamplers_[si].samplerState_);
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
#pragma message ("TODO- Fix hMaterial::destroyMaterialInstance to release references to input to streams")
        hcAssert(matInst);
        for (hUint32 tech = 0; tech < matInst->techniques_.GetSize(); ++tech) {
            for (hUint32 pass = 0; pass < matInst->techniques_[tech].passes_.GetSize(); ++pass) {
                hMaterialTechniquePass* passptr = &matInst->techniques_[tech].passes_[pass];
                passptr->unbind();
            }
        }

        for (hUint32 i = 0, c = matInst->constBlocks_.GetSize(); i < c; ++i) {
            if (matInst->constBlocks_[i].constBlock) {
                matInst->constBlocks_[i].constBlock->DecRef();
                matInst->constBlocks_[i].constBlock=NULL;
            }
        }
        for (hUint i=0,n=matInst->boundResources_.GetSize(); i<n; ++i) {
            if (matInst->boundResources_[i].srv) {
                matInst->boundResources_[i].srv->DecRef();
                matInst->boundResources_[i].srv=NULL;
            }
        }
        hDELETE_SAFE(memHeap_, matInst);
        hAtomic::Decrement(instanceCount_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::bindMaterial(hRenderMaterialManager* matManager) {
        // Bind programs 
        for (hUint32 group = 0; group < groups_.GetSize(); ++group) {
            for (hUint32 tech = 0; tech < groups_[group].techniques_.GetSize(); ++tech) {
                for (hUint32 pass = 0; pass < groups_[group].techniques_[tech].passes_.GetSize(); ++pass) {
                    hMaterialTechniquePass* passptr = &groups_[group].techniques_[tech].passes_[pass];
                    for (hUint32 progidx = 0; progidx < passptr->GetProgramCount(); ++progidx) {
                        passptr->bindShaderProgram(passptr->GetProgram(progidx));
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
                            hParameterConstantBlock* globalCB = matManager->GetGlobalConstantBlockByAlias(desc.name_);
                            hParameterConstantBlock* tmp=NULL;
                            hBool alreadyAdded = hFalse;
                            if (!globalCB) {
                                for (hUint mcb=0, mcbc=constBlocks_.GetSize(); mcb < mcbc; ++mcb) {
                                    if (constBlocks_[mcb].paramid == cbID) {
                                        alreadyAdded = hTrue;
                                    }
                                }
                                if (!alreadyAdded) {
                                    renderer_->createConstantBlock(desc.size_, NULL, &tmp);
                                    globalCB = tmp;
                                }
                            }
                            if (globalCB) {
                                bindConstanstBuffer(cbID, globalCB);
                            }
                            if (tmp) {
                                tmp->DecRef();
                            }
                        }
                    }
                }
            }
        }

        // Bind the default samplers to the programs
        for (hUint32 si = 0; si < defaultSamplers_.GetSize(); ++si) {
            hUint32 paramid=hCRC32::StringCRC(defaultSamplers_[si].name_);
            bindResource(paramid, defaultSamplers_[si].resView_);
            bindSampler(paramid, defaultSamplers_[si].samplerState_);
        }

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::bindResource(hShaderParameterID id, hShaderResourceView* srv)
    {
        hBool succ = true;
        for (hUint group=0; group < groups_.GetSize(); ++group) {
            for (hUint32 tech = 0, nTech = groups_[group].techniques_.GetSize(); tech < nTech; ++tech) {
                for (hUint32 passIdx = 0, nPasses = groups_[group].techniques_[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                    hMaterialTechniquePass* pass = groups_[group].techniques_[tech].GetPass(passIdx);
                    succ &= pass->setResourceView(id, srv);
                }
            }
        }

        if (succ) {
            hBool added = hFalse;
            for (hUint32 i = 0, c = boundResources_.GetSize(); i < c; ++i) {
                if (boundResources_[i].paramid == id) {
                    if (srv) {
                        srv->AddRef();
                    }
                    if (boundResources_[i].srv) {
                        boundResources_[i].srv->DecRef();
                    }
                    boundResources_[i].srv = srv;
                    added = hTrue;
                }
            }
            if (!added) {
                hBoundResource bt;
                bt.paramid = id;
                bt.srv = srv;
                srv->AddRef();
                boundResources_.PushBack(bt);
            }
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::bindSampler(hShaderParameterID id, hSamplerState* samplerState)
    {
        hBool succ = true;
        for (hUint group=0; group < groups_.GetSize(); ++group) {
            for (hUint32 tech = 0, nTech = groups_[group].techniques_.GetSize(); tech < nTech; ++tech) {
                for (hUint32 passIdx = 0, nPasses = groups_[group].techniques_[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                    hMaterialTechniquePass* pass = groups_[group].techniques_[tech].GetPass(passIdx);
                    succ &= pass->setSamplerInput(id, samplerState);
                }
            }
        }

        if (succ) {
            hBool added = hFalse;
            for (hUint32 i = 0, c = boundSamplers_.GetSize(); i < c; ++i) {
                if (boundSamplers_[i].paramid == id) {
                    if (samplerState) {
                        samplerState->AddRef();
                    }
                    if (boundSamplers_[i].state) {
                        boundSamplers_[i].state->DecRef();
                    }
                    boundSamplers_[i].state = samplerState;
                    added = hTrue;
                }
            }
            if (!added) {
                hBoundSampler bt;
                bt.paramid = id;
                bt.state = samplerState;
                samplerState->AddRef();
                boundSamplers_.PushBack(bt);
            }
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::addDefaultParameterValue(const hChar* paramName, void* data, hUint size) {
        hcAssert(paramName && data && size > 0);
        hDefaultParameterValue defVal={0};
        hStrCopy(defVal.parameterName, hMAX_PARAMETER_NAME_LEN, paramName);
        defVal.dataOffset=defaultDataSize_;
        defVal.dataSize=size;
        defVal.parameterNameHash=hCRC32::StringCRC(defVal.parameterName);
        defaultDataSize_+=size;
        defaultData_=(hUint8*)hHeapRealloc(memHeap_, defaultData_, defaultDataSize_);
        hMemCpy(((hUint8*)defaultData_)+defVal.dataOffset, data, size);
        defaultValues_.PushBack(defVal);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::initConstBlockBufferData(const hShaderProgram* prog, const hConstantBlockDesc& desc, void* outinitdata, hUint totaloutsize) const {
        hcAssert(prog && outinitdata);
        hZeroMem(outinitdata, totaloutsize);
        for (hUint i=0,n=prog->GetShaderParameterCount(); i<n; ++i) {
            hShaderParameter sparam;
            prog->GetShaderParameter(i, &sparam);
            for (hUint dvi=0,dvn=defaultValues_.GetSize(); dvi<dvn; ++dvi) {
                if (hStrCmp(defaultValues_[dvi].parameterName, sparam.name_)==0) {
                    hcAssertMsg(defaultValues_[dvi].dataSize <= sparam.size_, "Parameter default data is larger than actual shader data");
                    hUint copysize=hMin(defaultValues_[dvi].dataSize, sparam.size_);
                    hUint8* src=defaultData_+defaultValues_[dvi].dataOffset;
                    hUint8* dst=(hUint8*)outinitdata+sparam.cReg_;
                    hcAssertMsg(defaultValues_[dvi].dataOffset+copysize <= totaloutsize, "Input buffer isn't large enough for all data");
                    if (defaultValues_[dvi].dataOffset+copysize < totaloutsize) {
                        hMemCpy(dst, src, copysize);
                    }
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::unbind() {
        hcAssert(renderer_ && manager_);

        for (hUint32 group = 0; group < groups_.GetSize(); ++group) {
            for (hUint32 tech = 0; tech < groups_[group].techniques_.GetSize(); ++tech) {
                for (hUint32 pass = 0; pass < groups_[group].techniques_[tech].passes_.GetSize(); ++pass) {
                    hMaterialTechniquePass* passptr = &(groups_[group].techniques_[tech].passes_[pass]);
                    passptr->ReleaseResources(renderer_);
                }
            }
        }
        for (hUint32 i = 0, c = constBlocks_.GetSize(); i < c; ++i) {
            if (constBlocks_[i].constBlock) {
                constBlocks_[i].constBlock->DecRef();
                constBlocks_[i].constBlock=NULL;
            }
        }
        constBlocks_.Resize(0);
        for (hUint i=0,n=boundResources_.GetSize(); i<n; ++i) {
            if (boundResources_[i].srv) {
                boundResources_[i].srv->DecRef();
                boundResources_[i].srv=NULL;
            }
        }
        boundResources_.Resize(0);
        for (hUint i=0,n=boundSamplers_.GetSize(); i<n; ++i) {
            if (boundSamplers_[i].state) {
                boundSamplers_[i].state->DecRef();
                boundSamplers_[i].state=NULL;
            }
        }
        boundSamplers_.Resize(0);
        for (hUint32 i = 0, c = defaultSamplers_.GetSize(); i < c; ++i) {
            if (defaultSamplers_[i].boundTexture_) {
                defaultSamplers_[i].boundTexture_->DecRef();
            }
            if (defaultSamplers_[i].samplerState_) {
                defaultSamplers_[i].samplerState_->DecRef();
            }
            if (defaultSamplers_[i].resView_) {
                defaultSamplers_[i].resView_->DecRef();
            }
            defaultSamplers_[i].samplerState_=NULL;
            defaultSamplers_[i].boundTexture_=NULL;
            defaultSamplers_[i].resView_=NULL;
        }
        defaultSamplers_.Resize(0);

        defaultDataSize_=0;
        hHeapFreeSafe(memHeap_, defaultData_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialInstance::bindConstanstBuffer(hShaderParameterID id, hParameterConstantBlock* cb)
    {
        hcAssert(id);
        hcAssert(cb);
        hBool succ = true;
        for (hUint32 tech = 0, nTech = techniques_.GetSize(); tech < nTech; ++tech) {
            for (hUint32 passIdx = 0, nPasses = techniques_[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                hMaterialTechniquePass* pass = techniques_[tech].GetPass(passIdx);
                succ &= pass->setConstantBuffer(id, cb);
            }
        }

        if (succ) {
            hBool added = hFalse;
            for (hUint32 i = 0, c = constBlocks_.GetSize(); i < c; ++i) {
                if (constBlocks_[i].paramid == id) {
                    if (cb) {
                        cb->AddRef();
                    }
                    if (constBlocks_[i].constBlock) {
                        constBlocks_[i].constBlock->DecRef();
                    }
                    constBlocks_[i].constBlock = cb;
                    added = hTrue;
                }
            }
            if (!added && cb) {
                hBoundConstBlock bcb;
                bcb.paramid = id;
                bcb.constBlock = cb;
                cb->AddRef();
                constBlocks_.PushBack(bcb);
            }
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialInstance::bindSampler(hShaderParameterID id, hSamplerState* samplerState)
    {
        hBool succ = true;
        for (hUint32 tech = 0, nTech = techniques_.GetSize(); tech < nTech; ++tech) {
            for (hUint32 passIdx = 0, nPasses = techniques_[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                hMaterialTechniquePass* pass = techniques_[tech].GetPass(passIdx);
                succ &= pass->setSamplerInput(id, samplerState);
            }
        }

        if (succ) {
            hBool added = hFalse;
            for (hUint32 i = 0, c = boundSamplers_.GetSize(); i < c; ++i) {
                if (boundSamplers_[i].paramid == id) {
                    if (samplerState) {
                        samplerState->AddRef();
                    }
                    if (boundSamplers_[i].state) {
                        boundSamplers_[i].state->DecRef();
                    }
                    boundSamplers_[i].state = samplerState;
                    added = hTrue;
                }
            }
            if (!added) {
                hBoundSampler bt;
                bt.paramid = id;
                bt.state = samplerState;
                boundSamplers_.PushBack(bt);
            }
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialInstance::bindResource(hShaderParameterID id, hShaderResourceView* view) {
        hBool succ = true;
        for (hUint32 tech = 0, nTech = techniques_.GetSize(); tech < nTech; ++tech) {
            for (hUint32 passIdx = 0, nPasses = techniques_[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                hMaterialTechniquePass* pass = techniques_[tech].GetPass(passIdx);
                succ &= pass->setResourceView(id, view);
            }
        }

        if (succ) {
            hBool added = hFalse;
            for (hUint32 i = 0, c = boundResources_.GetSize(); i < c; ++i) {
                if (boundResources_[i].paramid == id) {
                    if (view) {
                        view->AddRef();
                    }
                    if (boundResources_[i].srv) {
                        boundResources_[i].srv->DecRef();
                    }
                    boundResources_[i].srv = view;
                    added = hTrue;
                }
            }
            if (!added) {
                hBoundResource bt;
                bt.paramid = id;
                bt.srv = view;
                view->AddRef();
                boundResources_.PushBack(bt);
            }
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialInstance::bindInputStreams(PrimitiveType type, hIndexBuffer* idx, hVertexBuffer** vtxs, hUint streamCnt) {
#pragma message ("TODO- Fix hMaterialInstance::bindInputStreams to take a reference to streams")
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
#pragma message ("TODO- Fix hMaterialInstance::bindVertexStream to take a reference to streams")
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

    hParameterConstantBlock* hMaterialInstance::GetParameterConstBlock(hShaderParameterID cbid)
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
