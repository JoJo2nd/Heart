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
    struct StageOutput {
        hTextureResource* textureResource = nullptr;
        hRenderer::hRenderTarget* target = nullptr;
        hStringID outputResource;
    };
    struct Stage {
        hStringID name;
        hStringID viewName;
        hStringID technique;
        hUint nOutputs = 0;
        StageOutput outputs[8];
    };

    std::vector<Stage> pipelineStages;
public:
    hObjectType(hRenderingPipeline, Heart::proto::renderpipeline::Pipeline);
};
}