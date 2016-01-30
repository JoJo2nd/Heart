/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "hRenderPipeline.h"
#include "core/hSystem.h"
#include "core/hResourceManager.h"
#include "render/hMipDesc.h"
#include "render/hTextureFlags.h"
#include "render/hTextureResource.h"

namespace Heart {

hRegisterObjectType(hRenderingPipeline, Heart::hRenderingPipeline, Heart::proto::renderpipeline::Pipeline);

hRenderingPipeline::hRenderingPipeline(Heart::proto::renderpipeline::Pipeline* obj) {
    pipelineStages.resize(obj->stages_size());
    for (hUint i=0, n=obj->stages_size(); i<n; ++i) {
        const proto::renderpipeline::PipelineStage& stage = obj->stages(i);
        pipelineStages[i].name.set(stage.name().c_str());
        pipelineStages[i].clearColour = stage.clearcolour();
        pipelineStages[i].clearDepth = stage.cleardepth();
        pipelineStages[i].clearStencil = stage.clearstencil();
        pipelineStages[i].colorValue = hColour(stage.clearcolourvalue().red(), stage.clearcolourvalue().green(), stage.clearcolourvalue().blue(), stage.clearcolourvalue().alpha());
        pipelineStages[i].depthValue = stage.cleardepthvalue();
        pipelineStages[i].stencilValue = (hUint8)(stage.clearstencilvalue() & 0xFF);
        pipelineStages[i].viewName.set(stage.viewname().c_str());
        pipelineStages[i].maxDrawCalls = stage.maxdrawcalls();
        pipelineStages[i].nTechniques = stage.techniques_size();
        for (hUint tech_i=0, tech_n=stage.techniques_size(); tech_i<tech_n; ++tech_i) {
            pipelineStages[i].techniques[tech_i].set(stage.techniques(tech_i).c_str());
        }
        pipelineStages[i].nOutputs = stage.outputs_size();
        for (hUint output_i = 0, output_n = pipelineStages[i].nOutputs; output_i < output_n; ++output_i) {
            pipelineStages[i].outputs[output_i].outputResource.set(stage.outputs(output_i).c_str());
        }
    }
}

hBool hRenderingPipeline::serialiseObject(Heart::proto::renderpipeline::Pipeline* obj, const Heart::hSerialisedEntitiesParameters& params) const {
    return hTrue;
}

hBool hRenderingPipeline::linkObject() {
    // all stages check for resources
    for (auto& i : pipelineStages) {
        for (hUint output_i=0, output_n=i.nOutputs; output_i<output_n; ++output_i) {
            if (!i.outputs[output_i].target) {
                i.outputs[output_i].textureResource = hResourceManager::weakResource<hTextureResource>(i.outputs[output_i].outputResource);
                if (!i.outputs[output_i].textureResource) return hFalse;
                hcAssert(i.outputs[output_i].textureResource->isRenderTarget());
                i.outputs[output_i].target = i.outputs[output_i].textureResource->getRenderTarget();
            }
        }
    }
    return hTrue;
}

}
