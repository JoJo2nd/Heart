/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "render/hProgramReflectionInfo.h"

namespace Heart {
namespace hTileRenderer2D {

struct Vert2D {
    struct { float x, y; } pos;
    struct { hHalfFloat x, y; } uv;
};

} // namespace hTileRenderer2D
} // namespace Heart
