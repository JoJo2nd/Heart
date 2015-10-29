/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hMaterial.h"
#include "render/hRenderer.h"
#include "render/hRenderShaderProgram.h"
#include "render/hProgramReflectionInfo.h"
#include "render/hTextureResource.h"
#include "core/hResourceManager.h"

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
                    hRenderer::hRenderCallDesc& rcd = *(rcDescs_.get()+currentpass);
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
                    for (auto i=0; i<maxShaderCount; ++i) programIDs[i] = hStringID();
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

                    hPassDesc pd;
                    pd.rcd = &rcd;
                    pd.progIDs = programIDs;
                    passTable_.insert(hPassHashTable::value_type(key, pd));
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
        hBool fullylinked = hTrue;
        for (auto& i : passTable_) {
            auto& p = i.second;
            if (!p.reflection) {
                hUint linkedprogs = 0;
                if (!p.progIDs[0].is_default() && !p.rcd->vertex_) {
                    auto res = hResourceManager::weakResource<hShaderProgram>(p.progIDs[0]);
                    p.rcd->vertex_ = res->getShader(p.progProfiles[0]);
                    linkedprogs |= (p.rcd->vertex_ != nullptr);
                } else 
                    linkedprogs |= 1;
                if (!p.progIDs[1].is_default() && !p.rcd->fragment_) {
                    auto res = hResourceManager::weakResource<hShaderProgram>(p.progIDs[1]);
                    p.rcd->fragment_ = res->getShader(p.progProfiles[1]);
                    linkedprogs |= (p.rcd->fragment_ != nullptr) << 1;
                }
                /*if (!p.progIDs[0].is_default() && !p.rcd->vertex_) {
                    p.rcd->vertex = hResourceManager::weakResource<hShaderStage*>(p.progIDs[0]);
                    linkedprogs |= p.rcd->vertex != nullptr;
                }
                if (!p.progIDs[0].is_default() && !p.rcd->vertex_) {
                    p.rcd->vertex = hResourceManager::weakResource<hShaderStage*>(p.progIDs[0]);
                    linkedprogs |= p.rcd->vertex != nullptr;
                }
                if (!p.progIDs[0].is_default() && !p.rcd->vertex_) {
                    p.rcd->vertex = hResourceManager::weakResource<hShaderStage*>(p.progIDs[0]);
                    linkedprogs |= p.rcd->vertex != nullptr;
                }*/
                if (linkedprogs == 0x3) {
                    p.reflection = hRenderer::createProgramReflectionInfo(
                        p.rcd->vertex_, p.rcd->fragment_, nullptr, nullptr, nullptr);
                    fullylinked = p.reflection != nullptr;
                } else {
                    fullylinked = hFalse;
                }
            }
        }
        if (fullylinked) {
            for (auto& i : parameters_) {
                if (i.type() == hMaterialParamType::Resource) {
                    // !!JM TODO: Textures
                    //hResourceManager::weakResource(i.getResourceID())
                }
            }
        }
        return fullylinked;
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

}//Heart
