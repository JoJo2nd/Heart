/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hMaterial.h"
#include "render/hRenderer.h"
#include "hRenderShaderProgram.h"
#include "hTexture.h"

#include <algorithm>

namespace Heart {


    hRegisterObjectType(material, Heart::hMaterial, Heart::proto::MaterialResource);

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    hBool hMaterial::serialiseObject(Heart::proto::MaterialResource* obj) const {
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    hBool hMaterial::deserialiseObject(Heart::proto::MaterialResource* obj) {
        totalPasses_=obj->totalpasses();
        rcDescs_.reset(new hRenderer::hRenderCallDesc[totalPasses_]);
        passProgIDs_.reset(new hStringID[totalPasses_*maxShaderCount]);
        groupNames_.reserve(obj->totalgroups());
        techniqueNames_.reserve(obj->totaltechniques());
        samplerStates_.resize(obj->totalsamplers());

        auto currentpass = 0u;
        for (hUint gi=0, gn=obj->groups_size(); gi<gn; ++gi) {
            const proto::MaterialGroup& groupdef=obj->groups(gi);
            auto groupname = hStringID(groupdef.groupname().c_str());
            groupNames_.push_back(groupname);
            for (hUint ti=0, tn=groupdef.technique_size(); ti<tn; ++ti, ++currentpass) {
                const proto::MaterialTechnique& techdef=groupdef.technique(ti);
                auto techname = hStringID(techdef.techniquename().c_str());
                if (std::find(techniqueNames_.begin(), techniqueNames_.end(), techname) == techniqueNames_.end()) {
                    techniqueNames_.push_back(techname);
                }
                for (hUint pi=0, pn=techdef.passes_size(); pi<pn; ++pi){
                    auto key = hPassKey(groupname, techname, pi);
                    const proto::MaterialPass& passdef=techdef.passes(pi);
                    hRenderer::hRenderCallDesc rcd;
                    if (passdef.has_blend()) {
                        const proto::BlendState& blenddef=passdef.blend();
                        if (blenddef.has_blendenable()) {
                            rcd.blend_.blendEnable_=blenddef.blendenable();
                        }
                        if (blenddef.has_srcblend()) {
                            rcd.blend_.srcBlend_=blenddef.srcblend();
                        }
                        if (blenddef.has_destblend()) {
                            rcd.blend_.destBlend_=blenddef.destblend();
                        }
                        if (blenddef.has_blendop()) {
                            rcd.blend_.blendOp_=blenddef.blendop();
                        }
                        if (blenddef.has_srcblendalpha()) {
                            rcd.blend_.srcBlendAlpha_=blenddef.srcblendalpha();
                        }
                        if (blenddef.has_destblendalpha()) {
                            rcd.blend_.destBlendAlpha_=blenddef.destblendalpha();
                        }
                        if (blenddef.has_blendopalpha()) {
                            rcd.blend_.blendOpAlpha_=blenddef.blendopalpha();
                        }
                        if (blenddef.has_rendertargetwritemask()) {
                            rcd.blend_.renderTargetWriteMask_=blenddef.rendertargetwritemask();
                        }
                    }
                    if (passdef.has_depthstencil()) {
                        const proto::DepthStencilState& depthdef=passdef.depthstencil();
                        if (depthdef.has_depthenable()) {
                            rcd.depthStencil_.depthEnable_ = depthdef.depthenable();
                        }
                        if (depthdef.has_depthwritemask()) {
                            rcd.depthStencil_.depthWriteMask_=depthdef.depthwritemask();
                        }
                        if (depthdef.has_depthfunc()) {
                            rcd.depthStencil_.depthFunc_=depthdef.depthfunc();
                        }
                        if (depthdef.has_stencilenable()) {
                            rcd.depthStencil_.stencilEnable_=depthdef.stencilenable();
                        }
                        if (depthdef.has_stencilreadmask()) {
                            rcd.depthStencil_.stencilReadMask_=depthdef.stencilreadmask();
                        }
                        if (depthdef.has_stencilwritemask()) {
                            rcd.depthStencil_.stencilWriteMask_=depthdef.stencilwritemask();
                        }
                        if (depthdef.has_stencilfailop()) {
                            rcd.depthStencil_.stencilFailOp_=depthdef.stencilfailop();
                        }
                        if (depthdef.has_stencildepthfailop()) {
                            rcd.depthStencil_.stencilDepthFailOp_=depthdef.stencildepthfailop();
                        }
                        if (depthdef.has_stencilpassop()) {
                            rcd.depthStencil_.stencilPassOp_=depthdef.stencilpassop();
                        }
                        if (depthdef.has_stencilfunc()) {
                            rcd.depthStencil_.stencilFunc_=depthdef.stencilfunc();
                        }
                        if (depthdef.has_stencilref()) {
                            rcd.depthStencil_.stencilRef_=depthdef.stencilref();
                        }
                    }
                    if (passdef.has_rasterizer()) {
                        const proto::RasterizerState& rasterdef=passdef.rasterizer();
                        if (rasterdef.has_fillmode()) {
                            rcd.rasterizer_.fillMode_=rasterdef.fillmode();
                        }
                        if (rasterdef.has_cullmode()) {
                            rcd.rasterizer_.cullMode_=rasterdef.cullmode();
                        }
                        if (rasterdef.has_frontcounterclockwise()) {
                            rcd.rasterizer_.frontCounterClockwise_=rasterdef.frontcounterclockwise();
                        }
                        if (rasterdef.has_depthbias()) {
                            rcd.rasterizer_.depthBias_=rasterdef.depthbias();
                        }
                        if (rasterdef.has_depthbiasclamp()) {
                            rcd.rasterizer_.depthBiasClamp_=rasterdef.depthbiasclamp();
                        }
                        if (rasterdef.has_slopescaleddepthbias()) {
                            rcd.rasterizer_.slopeScaledDepthBias_=rasterdef.slopescaleddepthbias();
                        }
                        if (rasterdef.has_depthclipenable()) {
                            rcd.rasterizer_.depthClipEnable_=rasterdef.depthclipenable();
                        }
                        if (rasterdef.has_scissorenable()) {
                            rcd.rasterizer_.scissorEnable_=rasterdef.scissorenable();
                        }
                    }

                    hStringID* programIDs = passProgIDs_.get() + (maxShaderCount*currentpass);
                    if (passdef.has_vertex()) {
                        programIDs[0] = hStringID(passdef.vertex().c_str());
                    }
                    if (passdef.has_pixel()) {
                        programIDs[1] = hStringID(passdef.pixel().c_str());
                    }
                    if (passdef.has_geometry()) {
                        programIDs[2] = hStringID(passdef.geometry().c_str());
                    }
                    if (passdef.has_hull()) {
                        programIDs[3] = hStringID(passdef.hull().c_str());
                    }
                    if (passdef.has_domain()) {
                        programIDs[4] = hStringID(passdef.domain().c_str());
                    }
                }
            }
        }
        //Read samplers
        for (hUint32 is=0, ns=obj->samplers_size(); is<ns; ++is) {
            auto& sd = samplerStates_[is];
            const proto::MaterialSampler& matsamplerdef=obj->samplers(is);

            if (matsamplerdef.has_samplerstate()) {
                const proto::SamplerState& samplerdef=matsamplerdef.samplerstate();
                if (samplerdef.has_filter()) {
                    sd.filter_=samplerdef.filter();
                }
                if (samplerdef.has_addressu()) {
                    sd.addressU_=samplerdef.addressu();
                }
                if (samplerdef.has_addressv()) {
                    sd.addressV_=samplerdef.addressv();
                }
                if (samplerdef.has_addressw()) {
                    sd.addressW_=samplerdef.addressw();
                }
                if (samplerdef.has_miplodbias()) {
                    sd.mipLODBias_=samplerdef.miplodbias();
                }
                if (samplerdef.has_maxanisotropy()) {
                    sd.maxAnisotropy_=samplerdef.maxanisotropy();
                }
                if (samplerdef.has_bordercolour()) {
                    const proto::Colour& colour=samplerdef.bordercolour();
                    hFloat alpha=colour.has_alpha() ? (colour.alpha()/255.f) : 1.f;
                    sd.borderColour_=hColour((colour.red()/255.f), (colour.green()/255.f), (colour.blue()/255.f), alpha);
                }
                if (samplerdef.has_minlod()) {
                    sd.minLOD_=samplerdef.minlod();
                }
                if (samplerdef.has_maxlod()) {
                    sd.maxLOD_=samplerdef.maxlod();
                }
            }
        }
        //Read parameters
        hUint paramdatasize=0;
        for (hUint pi=0, pn=obj->parameters_size(); pi<pn; ++pi) {
            const proto::MaterialParameter& paramdef=obj->parameters(pi);
            if (paramdef.floatvalues_size()) paramdatasize += sizeof(hFloat)*paramdef.floatvalues_size();
            else if (paramdef.intvalues_size()) paramdatasize += sizeof(hInt)*paramdef.intvalues_size();
        }
        if (!paramdatasize) 
            return hTrue;

        parameters_.reserve(obj->parameters_size());
        defaultParamData_.reset(new hByte[paramdatasize]);
        auto* dptr = defaultParamData_.get();
        for (hUint pi=0, pn=obj->parameters_size(); pi<pn; ++pi) {
            const proto::MaterialParameter& paramdef=obj->parameters(pi);
            auto floatcount = paramdef.floatvalues_size();
            auto intcount = paramdef.intvalues_size();
            if (paramdef.has_resourceid()) {
                parameters_.emplace_back(hStringID(paramdef.resourceid().c_str()));
            } else if (floatcount) {
                auto bytesize = sizeof(hFloat)*floatcount;
                hMemCpy(dptr, paramdef.floatvalues().data(), bytesize);
                parameters_.emplace_back((hFloat*)dptr, floatcount);
                dptr+=bytesize;
            } else if (intcount) {
                auto bytesize = sizeof(hInt)*intcount;
                hMemCpy(dptr, paramdef.intvalues().data(), bytesize);
                parameters_.emplace_back((hInt*)dptr, intcount);
                dptr+=bytesize;
            }
        }
        return hTrue;
    }

    hBool hMaterial::linkObject() {
        // !!JM TODO:
        // foreach pass in all passes
        //   ri = hRenderer::createReflectionInfo(all shaders) 
        //   query(ri)
        // buildDefaultUniformBlocks()
        return hTrue;
    }
    
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
#if 0
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterial::hMaterial() 
        : renderer_(nullptr)
        , manager_(nullptr)
        , defaultDataSize_(0)
        , defaultData_(nullptr)
        , selectorCount_(0)
        , groupCmds_(nullptr)
        , techCmds_(nullptr)
        , passCmds_(nullptr)
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterial::hMaterial(hRenderer* renderer) 
        : renderer_(renderer)
        , manager_(renderer->GetMaterialManager())
        , defaultDataSize_(0)
        , defaultData_(nullptr)
        , selectorCount_(0)
        , groupCmds_(nullptr)
        , techCmds_(nullptr)
        , passCmds_(nullptr)
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterial::~hMaterial()
    {
        stopListeningToResourceEvents();
        unbind();
        cleanup();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::addSamplerParameter(const hSamplerParameter& samp)
    {
        defaultSamplers_.push_back(samp);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialGroup* hMaterial::getGroupByName(const hChar* name) {
        for (hUint i=0,n=(hUint)groups_.size(); i<n; ++i) {
            if (hStrICmp(name, groups_[i].name_.c_str())==0) {
                return &groups_[i];
            }
        }
        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::bindConstanstBuffer(hShaderParameterID id, hRenderBuffer* cb)
    {
        hBool succ = true;
        for (hUint group=0, ngroups=(hUint)groups_.size(); group<ngroups; ++group) {
            for (hUint32 tech=0, nTech=(hUint)groups_[group].techniques_.size(); tech < nTech; ++tech) {
                for (hUint32 passIdx = 0, nPasses = groups_[group].techniques_[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                    hMaterialTechniquePass* pass = groups_[group].techniques_[tech].GetPass(passIdx);
                    succ &= pass->setConstantBuffer(id, cb);
                }
            }
        }

        if (succ) {
            hBool added = hFalse;
            for (hUint32 i = 0, c = constBlocks_.size(); i < c; ++i) {
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
                constBlocks_.push_back(bcb);
            }

            updateRenderCommands();
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderBuffer* hMaterial::GetParameterConstBlock( hShaderParameterID cbid )
    {
        for (hUint32 i = 0, c = constBlocks_.size(); i < c; ++i) {
            if (constBlocks_[i].paramid == cbid) {
                return constBlocks_[i].constBlock;
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::bind() {
#if 0 // !!JM
        // Grab and Bind const block info
        struct {
            hUint32 hash;
            hChar name[hMAX_PARAMETER_NAME_LEN];
            hUint32 size;
        }cbHashes[HEART_MAX_CONSTANT_BLOCKS] = {0};
        hUint32 cbcount = 0;
        totalTechniqueCount_=0;
        totalPassCount_=0;
        for (hUint32 group = 0; group < groups_.size(); ++group) {
            for (hUint32 tech = 0; tech < groups_[group].techniques_.size(); ++tech) {
                ++totalTechniqueCount_;
                for (hUint32 pass = 0; pass < groups_[group].techniques_[tech].passes_.size(); ++pass) {
                    hMaterialTechniquePass* passptr = &groups_[group].getTech(tech)->passes_[pass];
                    ++totalPassCount_;
                    for (hUint32 progidx = 0; progidx < passptr->GetProgramCount(); ++progidx) {
                        hShaderProgram* prog = passptr->getProgram(progidx);
                        //Create slots in the shaders
                        for (hUint si=0; prog && si<prog->getInputCount(); ++si) {
                            hShaderInput param;
                            static const hChar* typestr[] = {
                                "eShaderInputType_Buffer",
                                "eShaderInputType_Resource",
                                "eShaderInputType_Sampler",
                                "eShaderInputType_UAV",
                            };
                            prog->getInput(si, &param);
                            hShaderParameterID paramID = hCRC32::StringCRC(param.name_);
                            hcPrintf("Found Shader program input %s (Type: %s)", param.name_, typestr[param.type_]);
                            switch (param.type_) {
                            case eShaderInputType_Buffer: bindConstanstBuffer(paramID, hNullptr); break;
                            case eShaderInputType_Resource: bindResource(paramID, hNullptr); break;
                            case eShaderInputType_Sampler: bindSampler(paramID, hNullptr); break;
                            //case eShaderInputType_UAV: bindUAV(paramID, hNullptr); break;
                            }
                        }
                        //Create & Bind constant blocks
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
                            hRenderBuffer* globalCB = manager_->GetGlobalConstantBlockByAlias(desc.name_);
                            hRenderBuffer* tmp=NULL;
                            hBool alreadyAdded = hFalse;
                            if (!globalCB) {
                                for (hUint mcb=0, mcbc=constBlocks_.size(); mcb < mcbc; ++mcb) {
                                    if (constBlocks_[mcb].paramid == cbID && constBlocks_[mcb].constBlock != hNullptr) {
                                        alreadyAdded = hTrue;
                                    }
                                }
                                if (!alreadyAdded) {
                                    hByte* defaultCBData=(hByte*)hAlloca(desc.size_);
                                    initConstBlockBufferData(desc, prog, defaultCBData);
                                    renderer_->createBuffer(desc.size_, defaultCBData, eResourceFlag_ConstantBuffer, 0, &tmp);
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

        // Bind the default textures to the programs
        for (hUint dv=0, ndv=defaultValues_.size(); dv<ndv; ++dv) {
            if (defaultValues_[dv].type==ePTTexture) {
                hShaderResourceView* srv;
                hShaderResourceViewDesc srvdesc;
                hTexture* texture=defaultValues_[dv].resourcePtr.weakPtr<hTexture>();
                hZeroMem(&srvdesc, sizeof(srvdesc));
                srvdesc.resourceType_=texture->getRenderType();
                srvdesc.format_=texture->getTextureFormat();
                if (srvdesc.resourceType_==eRenderResourceType_Tex1D) {
                    srvdesc.tex1D_.topMip_=0;
                    srvdesc.tex1D_.mipLevels_=texture->getMipCount();
                } else if (srvdesc.resourceType_==eRenderResourceType_Tex1DArray) {
                    srvdesc.tex1DArray_.topMip_=0;
                    srvdesc.tex1DArray_.mipLevels_=texture->getMipCount();;
                    srvdesc.tex1DArray_.arrayStart_=0;
                    srvdesc.tex1DArray_.arraySize_=texture->getArraySize();
                } else if (srvdesc.resourceType_==eRenderResourceType_Tex2D) {
                    srvdesc.tex2D_.topMip_=0;
                    srvdesc.tex2D_.mipLevels_=texture->getMipCount();
                } else if (srvdesc.resourceType_==eRenderResourceType_Tex2DArray) {
                    srvdesc.tex2DArray_.topMip_=0;
                    srvdesc.tex2DArray_.mipLevels_=texture->getMipCount();
                    srvdesc.tex2DArray_.arrayStart_=0;
                    srvdesc.tex2DArray_.arraySize_=texture->getArraySize();
                }
                renderer_->createShaderResourceView(texture, srvdesc, &srv);
                bindResource(defaultValues_[dv].paramid, srv);
                srv->DecRef();
            }
        }
        // Bind the default samplers to the programs
        for (hUint32 si = 0; si < defaultSamplers_.size(); ++si) {
            hUint32 paramid=hCRC32::StringCRC(defaultSamplers_[si].name_);
            bindSampler(paramid, defaultSamplers_[si].samplerState_);
        }

        generateRenderCommands();
#endif
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::bindResource(hShaderParameterID id, hShaderResourceView* srv)
    {
        hBool succ = hFalse;
        for (hUint group=0; group < groups_.size(); ++group) {
            for (hUint32 tech = 0, nTech=(hUint)groups_[group].techniques_.size(); tech < nTech; ++tech) {
                for (hUint32 passIdx = 0, nPasses = groups_[group].techniques_[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                    hMaterialTechniquePass* pass = groups_[group].techniques_[tech].GetPass(passIdx);
                    succ |= pass->setResourceView(id, srv);
                }
            }
        }

        if (succ) {
            hBool added = hFalse;
            for (hUint32 i = 0, c = boundResources_.size(); i < c; ++i) {
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
            if (!added && srv) {
                hBoundResource bt;
                bt.paramid = id;
                bt.srv = srv;
                srv->AddRef();
                boundResources_.push_back(bt);
            }

            updateRenderCommands();
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::bindSampler(hShaderParameterID id, hSamplerState* samplerState)
    {
        hBool succ = true;
        for (hUint group=0; group < groups_.size(); ++group) {
            for (hUint32 tech = 0, nTech=(hUint)groups_[group].techniques_.size(); tech < nTech; ++tech) {
                for (hUint32 passIdx = 0, nPasses = groups_[group].techniques_[tech].GetPassCount(); passIdx < nPasses; ++passIdx) {
                    hMaterialTechniquePass* pass = groups_[group].techniques_[tech].GetPass(passIdx);
                    succ |= pass->setSamplerInput(id, samplerState);
                }
            }
        }

        if (succ) {
            hBool added = hFalse;
            for (hUint32 i = 0, c = boundSamplers_.size(); i < c; ++i) {
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
            if (!added && samplerState) {
                hBoundSampler bt;
                bt.paramid = id;
                bt.state = samplerState;
                samplerState->AddRef();
                boundSamplers_.push_back(bt);
            }

            updateRenderCommands();
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::addDefaultParameterValue(const hChar* name, hStringID res_id) {
        hDefaultParameterValue defVal;
        defVal.type=ePTTexture;
        defVal.paramid=hCRC32::StringCRC(name);
        defVal.resourcePtr=hResourceHandle(res_id);
        defVal.count=0;
        defaultValues_.push_back(defVal);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::addDefaultParameterValue(const hChar* name, const hInt32* data, hUint count) {
        hDefaultParameterValue defVal;
        hUint bytecount=count*sizeof(hInt32);
        defVal.type=ePTInt;
        defVal.paramid=hCRC32::StringCRC(name);
        defVal.count=count;
        defVal.dataOffset=defaultDataSize_;
        defaultData_=(hUint8*)hRealloc(defaultData_, defaultDataSize_+bytecount);
        hMemCpy(((hUint8*)defaultData_)+defVal.dataOffset, data, bytecount);
        defaultValues_.push_back(defVal);
        defaultDataSize_=bytecount;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::addDefaultParameterValue(const hChar* name, const hFloat* data, hUint count) {
        hDefaultParameterValue defVal;
        hUint bytecount=count*sizeof(hFloat);
        defVal.type=ePTFloat;
        defVal.paramid=hCRC32::StringCRC(name);
        defVal.count=count;
        defVal.dataOffset=defaultDataSize_;
        defaultData_=(hUint8*)hRealloc(defaultData_, defaultDataSize_+bytecount);
        hMemCpy(((hUint8*)defaultData_)+defVal.dataOffset, data, bytecount);
        defaultValues_.push_back(defVal);
        defaultDataSize_=bytecount;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::addDefaultParameterValue(const hChar* name, const hColour& colour) {
        hDefaultParameterValue defVal;
        hUint bytecount=sizeof(hColour);
        defVal.type=ePTColour;
        defVal.paramid=hCRC32::StringCRC(name);
        defVal.count=1;
        defVal.dataOffset=defaultDataSize_;
        defaultData_=(hUint8*)hRealloc(defaultData_, defaultDataSize_+bytecount);
        hMemCpy(((hUint8*)defaultData_)+defVal.dataOffset, &colour, bytecount);
        defaultValues_.push_back(defVal);
        defaultDataSize_=bytecount;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::initConstBlockBufferData(const hConstantBlockDesc& desc, const hShaderProgram* prog, void* outdata) {
        hShaderParameter param;
        for (hUint i=0, n=desc.parameterCount_; i<n; ++i) {
            prog->getConstantBlockParameter(desc, i, &param);
            hShaderParameterID paramid=hCRC32::StringCRC(param.name_);
            for (hUint dv=0, ndv=defaultValues_.size(); dv<ndv; ++dv) {
                if (defaultValues_[dv].paramid==paramid) {
                    hMemCpy(((hByte*)outdata)+param.cReg_, defaultData_+defaultValues_[dv].dataOffset, hMin(param.size_, defaultValues_[dv].count));
                    break;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::unbind() {
        hcAssert(renderer_ && manager_);

        releaseRenderCommands();

        for (hUint32 i = 0, c = constBlocks_.size(); i < c; ++i) {
            if (constBlocks_[i].constBlock) {
                constBlocks_[i].constBlock->DecRef();
                constBlocks_[i].constBlock=NULL;
            }
        }
        constBlocks_.resize(0);
        for (hUint i=0,n=boundResources_.size(); i<n; ++i) {
            if (boundResources_[i].srv) {
                boundResources_[i].srv->DecRef();
                boundResources_[i].srv=NULL;
            }
        }
        boundResources_.resize(0);
        for (hUint i=0,n=boundSamplers_.size(); i<n; ++i) {
            if (boundSamplers_[i].state) {
                boundSamplers_[i].state->DecRef();
                boundSamplers_[i].state=NULL;
            }
        }
        boundSamplers_.resize(0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::generateRenderCommands() {
        releaseRenderCommands();
        hRenderCommandGenerator rcGen(&renderCmds_);
        selectorCount_=getGroupCount()+getTotalTechniqueCount()+getTotalPassCount();
        groupCmds_=new hUint[selectorCount_];
        techCmds_=groupCmds_+getGroupCount();
        passCmds_=techCmds_+getTotalTechniqueCount();
        hUint groups=getGroupCount();
        hUint groupsWritten=0;
        hUint techsWritten=0;
        hUint passesWritten=0;
        for (hUint group=0; group<groups; ++group) {
            groupCmds_[groupsWritten++]=techsWritten;
            hUint techs=getGroup(group)->getTechCount();
            for (hUint tech=0; tech<techs; ++tech) {
                techCmds_[techsWritten++]=passesWritten;
                hUint passes=getGroup(group)->getTech(tech)->GetPassCount();
                for (hUint pass=0; pass<passes; ++pass) {
                    passCmds_[passesWritten++]=rcGen.getRenderCommandsSize();
                    hMaterialTechniquePass* passptr=getGroup(group)->getTech(tech)->GetPass(pass);
                    rcGen.setRenderStates(passptr->GetBlendState(), passptr->GetRasterizerState(), passptr->GetDepthStencilState());
                    rcGen.setShader(passptr->GetVertexShader(), ShaderType_VERTEXPROG);
                    if (passptr->getProgram(ShaderType_VERTEXPROG)) {
                        rcGen.setVertexInputs(
                            passptr->getSamplers(ShaderType_VERTEXPROG),
                            passptr->getSamplerCount(ShaderType_VERTEXPROG),
                            passptr->getShaderResourceViews(ShaderType_VERTEXPROG),
                            passptr->getShaderResourceViewCount(ShaderType_VERTEXPROG),
                            passptr->getConstantBuffers(ShaderType_VERTEXPROG),
                            passptr->getConstantBufferCount(ShaderType_VERTEXPROG));
                    }
                    rcGen.setShader(passptr->getProgram(ShaderType_FRAGMENTPROG), ShaderType_FRAGMENTPROG);
                    if (passptr->getProgram(ShaderType_FRAGMENTPROG)) {
                        rcGen.setPixelInputs(
                            passptr->getSamplers(ShaderType_FRAGMENTPROG),
                            passptr->getSamplerCount(ShaderType_FRAGMENTPROG),
                            passptr->getShaderResourceViews(ShaderType_FRAGMENTPROG),
                            passptr->getShaderResourceViewCount(ShaderType_FRAGMENTPROG),
                            passptr->getConstantBuffers(ShaderType_FRAGMENTPROG),
                            passptr->getConstantBufferCount(ShaderType_FRAGMENTPROG));
                    }
                    rcGen.setReturn();
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::releaseRenderCommands() {
        renderCmds_.reset();
        selectorCount_=0;
        delete[] groupCmds_;
        groupCmds_ = nullptr;
        techCmds_=hNullptr;
        passCmds_=hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::updateRenderCommands() {
        if (renderCmds_.isEmpty()) {
            return;
        }
        hRenderCommandGenerator rcGen(&renderCmds_);
        for (hUint group=0, groups=getGroupCount(); group<groups; ++group) {
            hUint techs=getGroup(group)->getTechCount();
            for (hUint tech=0; tech<techs; ++tech) {
                hUint passes=getGroup(group)->getTech(tech)->GetPassCount();
                for (hUint pass=0; pass<passes; ++pass) {
                    hMaterialTechniquePass* passptr=getGroup(group)->getTech(tech)->GetPass(pass);
                    hRCmd* rc=getRenderCommandsBegin(group, tech, pass);
                    hRCmd* rcend=getRenderCommandsEnd(group, tech, pass);
                    for (; rc<rcend; rc=(hRCmd*)(((hByte*)rc)+rc->size_)) {
                        if (rc->opCode_==eRenderCmd_SetVertexInputs) {
                            rcGen.updateVertexInputs(rc,
                                passptr->getSamplers(ShaderType_VERTEXPROG),
                                passptr->getSamplerCount(ShaderType_VERTEXPROG),
                                passptr->getShaderResourceViews(ShaderType_VERTEXPROG),
                                passptr->getShaderResourceViewCount(ShaderType_VERTEXPROG),
                                passptr->getConstantBuffers(ShaderType_VERTEXPROG),
                                passptr->getConstantBufferCount(ShaderType_VERTEXPROG));
                        } else if (rc->opCode_==eRenderCmd_SetPixelInputs) {
                            rcGen.updatePixelInputs(rc,
                                passptr->getSamplers(ShaderType_FRAGMENTPROG),
                                passptr->getSamplerCount(ShaderType_FRAGMENTPROG),
                                passptr->getShaderResourceViews(ShaderType_FRAGMENTPROG),
                                passptr->getShaderResourceViewCount(ShaderType_FRAGMENTPROG),
                                passptr->getConstantBuffers(ShaderType_FRAGMENTPROG),
                                passptr->getConstantBufferCount(ShaderType_FRAGMENTPROG));
                        }
                    }
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialGroup* hMaterial::addGroup(const hChar* name) {
        for (auto i=groups_.begin(),n=groups_.end(); i!=n; ++i) {
            if (hStrCmp(i->getName(), name)==0) {
                return &(*i);
            }
        }
        groups_.push_back(name);
        return &(*groups_.rbegin());
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::resourceUpdate(hStringID resourceid, hResurceEvent event) {
        // if a resource is missing, remove ourselves from the resource database then unbind
#if 0
        if (event == hResourceEvent_DBRemove) {
            resManager->removeResource(getResourceID());
            unbind();
        } else if (event == hResourceEvent_DBInsert) {
            if (linkDependeeResources()) {
                bind();
                resManager->insertResource(getResourceID(), this);
            }
        }
#else
        hcPrintf("Stub "__FUNCTION__);
#endif
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::linkDependeeResources() {
#if 0
        // Grab default resources
        for (hUint i=0, n=defaultValues_.size(); i<n; ++i) {
            if (defaultValues_[i].type==ePTTexture) {
                defaultValues_[i].resourcePtr=hResourceHandle(defaultValues_[i].resourceID);
                if (!defaultValues_[i].resourcePtr.weakPtr()) {
                    return hFalse;
                }
            }
        }

        // Grab all the shader programs
        for (hUint32 group = 0; group < groups_.size(); ++group) {
            for (hUint32 tech = 0; tech < groups_[group].techniques_.size(); ++tech) {
                groups_[group].techniques_[tech].mask_ = manager_->AddRenderTechnique( groups_[group].techniques_[tech].name_.c_str() )->mask_;
                for (hUint32 pass = 0; pass < groups_[group].techniques_[tech].passes_.size(); ++pass) {
                    hMaterialTechniquePass* passptr = &(groups_[group].techniques_[tech].passes_[pass]);
                    for (hUint shader=0; shader<ShaderType_MAX; ++shader) {
                        passptr->programs_[shader]=hResourceHandle(passptr->getProgramID(shader));
                        if (passptr->programs_[shader].getIsValid() && !passptr->programs_[shader].weakPtr()) {
                            return hFalse;
                        }
                    }
                }
            }
        }
#else
        hcPrintf("Stub "__FUNCTION__);
#endif

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::listenToResourceEvents() {

#if 0
        // register for resource updates
        for (hUint i=0, n=defaultValues_.size(); i<n; ++i) {
            if (defaultValues_[i].type==ePTTexture) {
                hResourceHandle(defaultValues_[i].resourceID).registerForUpdates(hFUNCTOR_BINDMEMBER(hResourceEventProc, hMaterial, resourceUpdate, this));
            }
        }
        for (hUint32 group = 0; group < groups_.size(); ++group) {
            for (hUint32 tech = 0; tech < groups_[group].techniques_.size(); ++tech) {
                groups_[group].techniques_[tech].mask_ = manager_->AddRenderTechnique( groups_[group].techniques_[tech].name_.c_str() )->mask_;
                for (hUint32 pass = 0; pass < groups_[group].techniques_[tech].passes_.size(); ++pass) {
                    hMaterialTechniquePass* passptr = &(groups_[group].techniques_[tech].passes_[pass]);
                    for (hUint shader=0; shader<ShaderType_MAX; ++shader) {
                        if (passptr->programs_[shader].getIsValid()) {
                            passptr->programs_[shader].registerForUpdates(hFUNCTOR_BINDMEMBER(hResourceEventProc, hMaterial, resourceUpdate, this));
                        }
                    }
                }
            }
        }

        if (linkDependeeResources()) {
            bind();
            resmanager->insertResource(getResourceID(), this);
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::stopListeningToResourceEvents() {
#if 0
        for (hUint i=0, n=defaultValues_.size(); i<n; ++i) {
            if (defaultValues_[i].type==ePTTexture) {
                hResourceHandle(defaultValues_[i].resourceID).unregisterForUpdates(hFUNCTOR_BINDMEMBER(hResourceEventProc, hMaterial, resourceUpdate, this));
            }
        }
        for (hUint32 group = 0; group < groups_.size(); ++group) {
            for (hUint32 tech = 0; tech < groups_[group].techniques_.size(); ++tech) {
                groups_[group].techniques_[tech].mask_ = manager_->AddRenderTechnique( groups_[group].techniques_[tech].name_.c_str() )->mask_;
                for (hUint32 pass = 0; pass < groups_[group].techniques_[tech].passes_.size(); ++pass) {
                    hMaterialTechniquePass* passptr = &(groups_[group].techniques_[tech].passes_[pass]);
                    for (hUint shader=0; shader<ShaderType_MAX; ++shader) {
                        if (passptr->programs_[shader].getIsValid()) {
                            passptr->programs_[shader].unregisterForUpdates(hFUNCTOR_BINDMEMBER(hResourceEventProc, hMaterial, resourceUpdate, this));
                        }
                    }
                }
            }
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::cleanup() {
        for (hUint32 i = 0, c = defaultSamplers_.size(); i < c; ++i) {
            if (defaultSamplers_[i].samplerState_) {
                defaultSamplers_[i].samplerState_->DecRef();
            }
            defaultSamplers_[i].samplerState_=NULL;
        }
        defaultSamplers_.resize(0);

        defaultDataSize_=0;
        hFree(defaultData_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialCmdLookUpHelper::init(hMaterial* material) {
        count_=material->getGroupCount()+material->getTotalTechniqueCount()+material->getTotalPassCount();
        group_=new hUint[count_];
        tech_=group_+material->getGroupCount();
        pass_=tech_+material->getTotalTechniqueCount();
        hUint groupsWritten=0;
        hUint techsWritten=0;
        hUint passesWritten=0;
        hUint groups=material->getGroupCount();
        for (hUint group=0; group<groups; ++group) {
            group_[groupsWritten++]=techsWritten;
            hUint techs=material->getGroup(group)->getTechCount();
            for (hUint tech=0; tech<techs; ++tech) {
                tech_[techsWritten++]=passesWritten;
                hUint passes=material->getGroup(group)->getTech(tech)->GetPassCount();
                for (hUint pass=0; pass<passes; ++pass) {
                    pass_[passesWritten++]=hErrorCode;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialCmdLookUpHelper::destroy() {
        count_=0;
        delete[] group_;
        group_ = nullptr;
        tech_=hNullptr;
        pass_=hNullptr;
    }
#endif
}//Heart
