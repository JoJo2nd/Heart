/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hStringID.h"
#include "components/hObjectFactory.h"
#include "render/hRenderer.h"
#include "resource_renderpipeline.pb.h"

namespace Heart {
class hTextureResource;

class hRenderingPipeline {
public:
    struct StageOutput {
        hTextureResource* textureResource = nullptr;
        hRenderer::hRenderTarget* target = nullptr;
        hStringID outputResource;
    };
    struct Stage {
        hStringID name;
        hStringID viewName;
        hUint8 stencilValue = 0;
        hBool clearColour = false;
        hBool clearDepth = false;
        hBool clearStencil = false;
        hColour colorValue;
        hFloat depthValue = 1.f;
        hUint nOutputs = 0;
        hUint nTechniques = 0;
        hUint maxDrawCalls = 0;
        StageOutput outputs[8];
        hStringID techniques[8];
    };

    hObjectType(hRenderingPipeline, Heart::proto::renderpipeline::Pipeline);

    hUint getStageCount() const { return (hUint)pipelineStages.size(); }
    const Stage& getStage(hUint i) const { return pipelineStages[i]; }

private:
    std::vector<Stage> pipelineStages;
};
}