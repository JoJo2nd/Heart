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

    void tick();
    void renderTilePlanes(hRenderer::hCmdList* cl);
    hBool registerComponents();
}
}