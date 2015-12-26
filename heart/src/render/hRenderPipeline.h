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

struct hRenderingPassInput {
    typedef proto::renderpipeline::InputType InputType;
    
    hRenderingPassInput(hStringID in_name, InputType in_type)
        : name(in_name)
        , type(in_type)
    {}

    hStringID name;
    InputType type = proto::renderpipeline::Texture;
    // the following are never owned, only weak pointers
    hRenderer::hTexture2D* texture;
    hRenderer::hUniformBuffer* uniformBuffer;
};

struct hRenderingPassOutput {
    hRenderingPassOutput(hStringID in_name, hUint32 in_slot, hTextureFormat in_fmt);
    hStringID name;
    hUint32 slot = 0;
    hTextureFormat format = hTextureFormat::RGBA8_sRGB_unorm;
    hRenderer::hTexture2DUniquePtr surface;
    hRenderer::hRenderTargetUniquePtr target;
};

class hRenderingPass {
    typedef proto::renderpipeline::OutputType OutputType;

    hStringID techniqueName;
    std::vector<hRenderingPassInput> inputs;
    std::vector<hRenderingPassOutput> outputs;

    static hTextureFormat convertOutputTypeToFormat(OutputType a);
public:
    hObjectType(hRenderingPass, Heart::proto::renderpipeline::Pass);

    hBool setInput(hStringID in_name, hRenderer::hTexture2D* t);
    hBool setInput(hStringID in_name, hRenderer::hUniformBuffer* ub);
    hRenderer::hTexture2D* getOutput(hStringID out_name);
};

class hRenderingPipeline {
    struct LinkingInfo {
        std::vector<hStringID> stages;
    };
    struct ParamBinding {
        hUint32 from;
        hStringID fromName;
        hUint32 to;
        hStringID toName;
    };

    std::unique_ptr<LinkingInfo> linkInfo;

    std::vector<ParamBinding> bindings;
    std::vector<hRenderingPass*> passes;
public:
    hObjectType(hRenderingPipeline, Heart::proto::renderpipeline::Pipeline);
};
}