/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hRendererConstants.h"
#include "core/hProtobuf.h"
#include "math/hVec3.h"
#include "components/hObjectFactory.h"
#include "components/hEntity.h"
#include "render/hRendererTypeHelpers.h"
#include "resource_2d.pb.h"

namespace Heart {
class hShaderProgram;
class hDynamicTileSet2D;
class hTextureAtlasResource;

class hRenderPlane2D : public hEntityComponent {
    struct LoadLinkInfo {
        hStringID tileSetResource;
        hUuid_t tileSetEntity;
        hStringID shaderResource;
    };

    std::unique_ptr<LoadLinkInfo> linkInfo;
    hStringID name;
    std::vector<hUint> tiles;
    hRenderer::hVertexBufferUniquePtr gridVertexBuffer;
    hRenderer::hPipelineStateUniquePtr pipelineState;
    hRenderer::hUniformBufferUniquePtr planeUniformBuffer;
    hRenderer::hInputStateUniquePtr inputState;
    hShaderProgram* shaderProgram = nullptr;
    hDynamicTileSet2D* tileSet = nullptr;
    hTextureAtlasResource* staticTileSet = nullptr;
    hUint renderSectorSize = 0;
    hUint gridSizeX = 0;
    hUint gridSizeY = 0;
    hUint tileSizeX = 0;
    hUint tileSizeY = 0;
    hUint renderOrder = 0;
    hUint offsetX = 0;
    hUint offsetY = 0;
    hUint currentFence = 0;
    hInt viewSizeX = 0;
    hInt viewSizeY = 0;
    union {
        struct {
            hBool xWrapAround : 1;
            hBool yWrapAround : 1;
            hBool dynamic : 1;
            hBool dirty : 1;
        };
        hUint32 flags = 0;
    };

public:
    hObjectType(hRenderPlane2D, Heart::proto::RenderPlane2D);

    hUint getPlaneLayer() const { return renderOrder; }
    void updateTileData(hRect region, hUint* data);
    void updateView(hVec3 topLeft, hVec3 bottomRight);
    void updateSectorVertexBuffer(hRenderer::hCmdList* cl, hUint current_fence);
    void submitPlane(hRenderer::hCmdList* cl, hUint current_fence);
};

}