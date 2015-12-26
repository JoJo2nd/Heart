/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "hRenderPipeline.h"
#include "core/hSystem.h"
#include "core/hResourceManager.h"
#include "render/hMipDesc.h"
#include "render/hTextureFlags.h"

namespace Heart {

hRegisterObjectType(hRenderingPass, Heart::hRenderingPass, Heart::proto::renderpipeline::Pass);
hRegisterObjectType(hRenderingPipeline, Heart::hRenderingPipeline, Heart::proto::renderpipeline::Pipeline);

hRenderingPass::hRenderingPass(Heart::proto::renderpipeline::Pass* obj) {
    techniqueName.set(obj->techniquename().c_str());
    inputs.reserve(obj->inputs_size());
    outputs.reserve(obj->outputs_size());
    for (hUint32 i=0,n=obj->inputs_size(); i<n; ++i) {
        const auto& input = obj->inputs(i);
        inputs.emplace_back(hStringID(input.name().c_str()), input.type());
    }
    for (hUint i=0, n=obj->outputs_size(); i<n; ++i) {
        const auto& output = obj->outputs(i);
        outputs.emplace_back(hStringID(output.name().c_str()), output.index(), convertOutputTypeToFormat(output.type()));
    }
}

hBool hRenderingPass::serialiseObject(Heart::proto::renderpipeline::Pass* obj, const hSerialisedEntitiesParameters& params) const {
    return hTrue;
}

hBool hRenderingPass::linkObject() {
    return hTrue;
}

Heart::hTextureFormat hRenderingPass::convertOutputTypeToFormat(OutputType a) {
    namespace hprp = Heart::proto::renderpipeline;
    switch (a) {
    case hprp::RGBA8: return hTextureFormat::RGBA8_sRGB_unorm;
    case hprp::RGBA16: return hTextureFormat::RGBA16_float;
    case hprp::RG16: return hTextureFormat::RG16_float;
    case hprp::RGB10A2: return hTextureFormat::RGB10A2_unorm;
    case hprp::Depth: return hTextureFormat::D24S8_float; // INTZ for D3D9?
    }
    return hTextureFormat::Unknown;
}

hBool hRenderingPass::setInput(hStringID in_name, hRenderer::hTexture2D* t) {
    for (auto& i : inputs) {
        if (i.name == in_name && i.type == proto::renderpipeline::Texture) {
            i.texture = t;
            return hTrue;
        }
    }
    return hFalse;
}

hBool hRenderingPass::setInput(hStringID in_name, hRenderer::hUniformBuffer* ub) {
    for (auto& i : inputs) {
        if (i.name == in_name && i.type == proto::renderpipeline::UniformBuffer) {
            i.uniformBuffer = ub;
            return hTrue;
        }
    }
    return hFalse;
}

hRenderer::hTexture2D* hRenderingPass::getOutput(hStringID out_name) {
    for (auto& i : outputs) {
        if (i.name == out_name) {
            return i.surface.get();
        }
    }
    return nullptr;
}

hRenderingPipeline::hRenderingPipeline(Heart::proto::renderpipeline::Pipeline* obj) {
    passes.resize(obj->stages_size());
    linkInfo.reset(new LinkingInfo());
    linkInfo->stages.resize(obj->stages_size());
    bindings.reserve(obj->totalbindings());
    for (hUint i=0, n=obj->stages_size(); i<n; ++i) {
        auto& st = obj->stages(i);
        linkInfo->stages[st.stageid()].set(st.passresource().c_str());
        for (hUint b = 0, bn = st.inputbindings_size(); b < bn; ++b) {
            const auto& bind = st.inputbindings(b);
            ParamBinding binding;
            binding.from = bind.fromstateid();
            binding.fromName.set(bind.from().c_str());
            binding.to = i;
            binding.toName.set(bind.to().c_str());
            bindings.push_back(binding);
        }
    }
}

hBool hRenderingPipeline::serialiseObject(Heart::proto::renderpipeline::Pipeline* obj, const Heart::hSerialisedEntitiesParameters& params) const {
    return hTrue;
}

hBool hRenderingPipeline::linkObject() {
    if (!linkInfo) {
        return hTrue;
    }
    for (hSize_t i=0, n=linkInfo->stages.size(); i<n; ++i) {
        auto* r = hResourceManager::weakResource<hRenderingPass>(linkInfo->stages[i]);
        if (!r) return hFalse;
        passes[i] = r;
    }
    // all passes set, binding internal inputs->outputs
    for (const auto& i : bindings) {
        auto* rt = passes[i.from]->getOutput(i.fromName);
        hcAssertMsg(rt, "Rendering pass '%s' is missing output '%s'", linkInfo->stages[i.from].c_str(), i.fromName.c_str());
        passes[i.to]->setInput(i.toName, rt);
    }
    return hTrue;
}

hRenderingPassOutput::hRenderingPassOutput(hStringID in_name, hUint32 in_slot, hTextureFormat in_fmt) 
    : name(in_name)
    , slot(in_slot)
    , format(in_fmt)
{
    hRenderer::hMipDesc top_mip = {hSystem::getWindowWidth(), hSystem::getWindowHeight(), nullptr, 0};
    surface.reset(hRenderer::createTexture2D(1, &top_mip, in_fmt, (hUint32)hRenderer::TextureFlags::RenderTarget));
    target.reset(hRenderer::createRenderTarget(surface.get(), 0));
}

}
