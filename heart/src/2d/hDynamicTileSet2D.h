/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "core/hProtobuf.h"
#include "components/hObjectFactory.h"
#include "components/hEntity.h"
#include "render/hTextureFormat.h"
#include "render/hRendererTypeHelpers.h"
#include "utils/hTextureAtlas.h"
#include "resource_2d.pb.h"

namespace Heart {
class hTextureResource;

class hDynamicTileSet2D : public hEntityComponent {
public:
    hUint32 tileSetWidth = 0;
    hUint32 tileSetHeight = 0;
    hUint32 minXSize = 0;
    hUint32 minYSize = 0;
    hTextureFormat textureFormat;
    hRenderer::hTexture2DUniquePtr tileSet;
    Util::hTextureAtlasBuilder textureAtlas;
    std::vector<const Util::hAtlasNode*> tiles;
    hBool dirty = false;

public:
    hObjectType(hDynamicTileSet2D, Heart::proto::DynamicTileSet2D);

    typedef hUint32 TileHandle;
    enum { InvalidTileHandle = ~0U };

    TileHandle addTextureToTileSet(hTextureResource* texture);
    const Util::hAtlasNode* getTileInfo(TileHandle handle) { return tiles[handle]; }
    hUint32 getWidth() const { return tileSetWidth; }
    hUint32 getHeight() const { return tileSetHeight; }
    hRenderer::hTexture2D* getTexture2D() { return tileSet.get(); }
    hBool flushNewTextureData(hRenderer::hCmdList* cl);
    hBool isDirty() const { return dirty; }
};

}