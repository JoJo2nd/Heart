/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

namespace Heart {
namespace hRenderer {
    struct hVertexBuffer;
    hUint getStride(const hVertexBuffer* vb);
    hUint getVertexCount(const hVertexBuffer* vb);
}
}