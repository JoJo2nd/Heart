/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"

namespace Heart {
namespace hRenderer {
    struct hCmdList;
}
namespace hTileRenderer2D {

    hBool initialise();
    void initialiseResources();
    void setView(const hRendererCamera& camera);
    void transformPlanesToViewSpace();
    void updateDynamicRenderResources(hRenderer::hCmdList* cl);
    void renderTilePlanes(hRenderer::hCmdList* cl);
    hBool registerComponents();
    hRenderer::hUniformBuffer* getViewUniformBuffer();
    hRenderer::hUniformBuffer* getSharedUniformBuffer(); // temp
}
}