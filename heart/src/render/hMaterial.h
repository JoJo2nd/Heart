/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hStringID.h"
#include "components/hObjectFactory.h"
#include "render/hRenderer.h"
#include "render/hPipelineStateDesc.h"
#include "resource_material_fx.pb.h"

namespace Heart {
class hMaterial {
    struct LinkInfo {
        std::vector<hStringID> waitingResources;
        Heart::proto::MaterialResource resource_pb;
    };

    struct Pass {
        hRenderer::hPipelineStateDesc pipelineStateDesc;
        hRenderer::hInputStateDesc inputStateDsec;
    };

    struct Technique {
        hStringID technqiueName;
        std::vector<Pass> passes;
    };

    std::unique_ptr<LinkInfo> linkingInfo;
    std::vector<hRenderer::hUniformBufferUniquePtr> embeddedUniformBuffers;
    std::vector<Technique> techinques;
    std::vector<hStringID> overrideTextureNames;
    std::vector<hStringID> overrideBufferNames;
public:
    hObjectType(hMaterial, Heart::proto::MaterialResource);

    static const hUint32 InvalidTechniqueIndex = ~0ul;

    hUint getTechniqueCount() const { return (hUint)techinques.size(); }
    hStringID getTechniqueName(hUint i) const { return techinques[i].technqiueName; }
    hUint getTechniqueByName(hStringID in_name) {
        for (hSize_t i=0, n=techinques.size(); i<n; ++i) {
            if (techinques[i].technqiueName == in_name) {
                return (hUint)i;
            }
        }
        return InvalidTechniqueIndex;
    }
    hUint getTechniquePassCount(hUint i) const { return (hUint)techinques[i].passes.size(); }
    hUint getOverrideBufferCount() const { return (hUint)overrideBufferNames.size(); }
    hStringID getOverrideBufferName(hUint i) const { return overrideBufferNames[i]; }
    hUint getOverrideTextureCount() const { return (hUint)overrideTextureNames.size(); }
    hStringID getOverrideTextureName(hUint i) const { return overrideTextureNames[i]; }
    const hRenderer::hPipelineStateDesc& getTechniquePassPipelineStateDesc(hUint t, hUint p) const { return techinques[t].passes[p].pipelineStateDesc; }
    const hRenderer::hInputStateDesc& getTechniquePassInputStateDesc(hUint t, hUint p) const { return techinques[t].passes[p].inputStateDsec; }
};
}