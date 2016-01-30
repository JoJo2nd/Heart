/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "hMaterial.h"
#include "base/hRendererConstants.h"
#include "base/hColour.h"
#include "core/hResourceManager.h"
#include "render/hRenderShaderProgram.h"
#include "render/hProgramReflectionInfo.h"
#include "render/hUniformBufferFlags.h"
#include "render/hTextureResource.h"
#include "render/hUniformBufferResource.h"
#include "render/hTechniques.h"

namespace Heart {

    hRegisterObjectType(hMaterial, Heart::hMaterial, Heart::proto::MaterialResource);

    hMaterial::hMaterial(Heart::proto::MaterialResource* obj) {
        linkingInfo.reset(new LinkInfo());
        linkingInfo->resource_pb = *obj;
        linkingInfo->waitingResources.reserve(64);
        for (hUint i = 0, n = obj->parameters_size(); i < n; ++i) {
            const proto::MaterialParameter& param_pb = obj->parameters(i);
            if (param_pb.has_resourceid()) {
                hStringID name(param_pb.resourceid().c_str());
                linkingInfo->waitingResources.push_back(name);
            }
        }

        for (hUint i = 0, n = obj->uniformbuffers_size(); i < n; ++i) {
            const proto::MaterialUniformBuffer& ub_pb = obj->uniformbuffers(i);
            hStringID name(ub_pb.resource().c_str());
            linkingInfo->waitingResources.push_back(name);
        }

        for (hUint i = 0, n = obj->techniques_size(); i < n; ++i) {
            const proto::MaterialTechnique& tech_pb = obj->techniques(i);
            for (hUint p = 0, pn = tech_pb.passes_size(); p < pn; ++p) {
                const proto::MaterialPass& pass_pb = tech_pb.passes(p);
                if (pass_pb.has_vertex()) linkingInfo->waitingResources.push_back(hStringID(pass_pb.vertex().c_str()));
                if (pass_pb.has_pixel()) linkingInfo->waitingResources.push_back(hStringID(pass_pb.pixel().c_str()));
                if (pass_pb.has_geometry()) linkingInfo->waitingResources.push_back(hStringID(pass_pb.geometry().c_str()));
                if (pass_pb.has_hull()) linkingInfo->waitingResources.push_back(hStringID(pass_pb.hull().c_str()));
                if (pass_pb.has_domain()) linkingInfo->waitingResources.push_back(hStringID(pass_pb.domain().c_str()));
            }
        }
    }

    hBool hMaterial::serialiseObject(Heart::proto::MaterialResource* obj, const hSerialisedEntitiesParameters& params) const {
        return hTrue;
    }

    hBool hMaterial::linkObject() {
        for (const auto& i : linkingInfo->waitingResources) {
            if (!hResourceManager::weakResourceRef(i)) return hFalse;
        }

        // all the resources are ready!
        Heart::proto::MaterialResource* obj = &linkingInfo->resource_pb;
        hRenderer::hPipelineStateDesc psd;
        hRenderer::hInputStateDesc isd;
        for (hUint i = 0, n = obj->samplers_size(); i < n; ++i) {
            const auto& mat_sampler_pb = obj->samplers(i);
            const auto& sampler_pb = mat_sampler_pb.samplerstate();
            hRenderer::hPipelineStateDesc::hSamplerStateDesc ssd;
            ssd.filter_ = sampler_pb.filter();
            ssd.addressU_ = sampler_pb.addressu();
            ssd.addressV_ = sampler_pb.addressv();
            ssd.addressW_ = sampler_pb.addressw();
            const auto& bordercolour = sampler_pb.bordercolour();
            ssd.borderColour_ =
                hColour(bordercolour.red(), bordercolour.green(), bordercolour.blue(), bordercolour.alpha());
            ssd.maxAnisotropy_ = sampler_pb.maxanisotropy();
            ssd.minLOD_ = sampler_pb.minlod();
            ssd.maxLOD_ = sampler_pb.maxlod();
            ssd.mipLODBias_ = sampler_pb.miplodbias();
            psd.setSampler(hStringID(mat_sampler_pb.samplername().c_str()), ssd);
        }

        for (hUint i = 0, n = obj->parameters_size(); i < n; ++i) {
            const proto::MaterialParameter& param_pb = obj->parameters(i);
            if (param_pb.has_resourceid()) {
                hTextureResource* res = hResourceManager::weakResource<hTextureResource>(hStringID(param_pb.resourceid().c_str()));
                hStringID name(param_pb.paramname().c_str());
                isd.setTextureSlot(name, res->getTexture2D());
                if (param_pb.allowoverride()) {
                    overrideTextureNames.push_back(name);
                }
            }
        }

        for (hUint i = 0, n = obj->uniformbuffers_size(); i < n; ++i) {
            const proto::MaterialUniformBuffer& ub_pb = obj->uniformbuffers(i);
            hUniformBufferResource* ub_res = hResourceManager::weakResource<hUniformBufferResource>(hStringID(ub_pb.resource().c_str()));
            if (ub_pb.embed() || ub_pb.allowoverride()) {
                // Embedded buffers are static for the time being...
                embeddedUniformBuffers.emplace_back();
                embeddedUniformBuffers.back().reset(hRenderer::createUniformBuffer(nullptr/*TODO: Grab memory for this buffer*/, ub_res->getLayoutDesc(), ub_res->getLayoutDescSize(), ub_res->getByteSize(), 0));
                isd.setUniformBuffer(ub_res->getName(), embeddedUniformBuffers.back().get());

                if (ub_pb.allowoverride()) {
                    overrideBufferNames.push_back(ub_res->getName());
                }
            } else { // shared
                isd.setUniformBuffer(ub_res->getName(), ub_res->getSharedUniformBuffer());
            }
        }

        techinques.reserve(obj->techniques_size());
        for (hUint tech_i=0, tech_n=obj->techniques_size(); tech_i<tech_n; ++tech_i) {
            techinques.emplace_back();
            Technique& technique = techinques.back();
            const proto::MaterialTechnique& technique_pb = obj->techniques(tech_i);
            technique.technqiueName.set(technique_pb.techniquename().c_str());
            hTechniques::registerTechnique(technique.technqiueName);
            technique.techniqueFlag = hTechniques::getTechniqueFlag(technique.technqiueName);
            technique.passes.reserve(technique_pb.passes_size());
            for (hUint pass_i=0, pass_n=technique_pb.passes_size(); pass_i<pass_n; ++pass_i) {
                technique.passes.emplace_back();
                Pass& pass = technique.passes.back();
                const proto::MaterialPass& pass_pb = technique_pb.passes(pass_i);
                const proto::BlendState& blend_pb = pass_pb.blend();
                psd.blend_.blendEnable_ = blend_pb.blendenable();
                psd.blend_.srcBlend_ = blend_pb.srcblend();
                psd.blend_.srcBlendAlpha_ = blend_pb.srcblendalpha();
                psd.blend_.destBlend_ = blend_pb.destblend();
                psd.blend_.destBlendAlpha_ = blend_pb.destblendalpha();
                psd.blend_.blendOp_ = blend_pb.blendop();
                psd.blend_.blendOpAlpha_ = blend_pb.blendopalpha();
                psd.blend_.renderTargetWriteMask_ = blend_pb.rendertargetwritemask();
                const proto::DepthStencilState& depthstencil_pb = pass_pb.depthstencil();
                psd.depthStencil_.depthEnable_ = depthstencil_pb.depthenable();
                psd.depthStencil_.depthFunc_ = depthstencil_pb.depthfunc();
                psd.depthStencil_.depthWriteMask_ = depthstencil_pb.depthwritemask();
                psd.depthStencil_.stencilEnable_ = depthstencil_pb.stencilenable();
                psd.depthStencil_.stencilFunc_ = depthstencil_pb.stencilfunc();
                psd.depthStencil_.stencilPassOp_ = depthstencil_pb.stencilpassop();
                psd.depthStencil_.stencilFailOp_ = depthstencil_pb.stencilfailop();
                psd.depthStencil_.stencilRef_ = depthstencil_pb.stencilref();
                psd.depthStencil_.stencilReadMask_ = depthstencil_pb.stencilreadmask();
                psd.depthStencil_.stencilWriteMask_ = depthstencil_pb.stencilwritemask();
                const proto::RasterizerState& rasterizer_pb = pass_pb.rasterizer();
                psd.rasterizer_.cullMode_ = rasterizer_pb.cullmode();
                psd.rasterizer_.fillMode_ = rasterizer_pb.fillmode();
                psd.rasterizer_.scissorEnable_ = rasterizer_pb.scissorenable();
                psd.rasterizer_.depthBias_ = (hInt32)rasterizer_pb.depthbias();
                psd.rasterizer_.depthBiasClamp_ = rasterizer_pb.depthbiasclamp();
                psd.rasterizer_.depthClipEnable_ = rasterizer_pb.depthclipenable();
                psd.rasterizer_.slopeScaledDepthBias_ = rasterizer_pb.slopescaleddepthbias();

                psd.vertex_ = hResourceManager::weakResource<hShaderProgram>(hStringID(pass_pb.vertex().c_str()))->getShader(hRenderer::getActiveProfile(hShaderFrequency::Vertex));
                psd.fragment_ = hResourceManager::weakResource<hShaderProgram>(hStringID(pass_pb.pixel().c_str()))->getShader(hRenderer::getActiveProfile(hShaderFrequency::Pixel));

                pass.pipelineStateDesc = psd;
                pass.inputStateDsec = isd;
            }
        }

        linkingInfo.reset();
        return hTrue;
    }

}
