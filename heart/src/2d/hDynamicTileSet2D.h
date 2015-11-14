/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "core/hProtobuf.h"
#include "components/hObjectFactory.h"
#include "components/hEntity.h"
#include "utils/hTextureAtlas.h"
#include "resource_2d.pb.h"

namespace Heart {
namespace hRenderer {
class hTextureResource;
}

class hDynamicTileSet2D : public hEntityComponent {
    hUint32 tileSetWidth;
    hUint32 tileSetHeight;
    hUint32 minXSize;
    hUint32 minYSize;
    hRenderer::hTextureResource* tileSet;
    Util::hTextureAtlasBuilder textureAtlas;
public:
    hObjectType(hDynamicTileSet2D, Heart::proto::DynamicTileSet2D);

    typedef hUint32 TileHandle;

    TileHandle addTextureToTileSet(hRenderer::hTextureResource* texture);
    void removeTextureFromTileSet(TileHandle tile_handle);
};

}