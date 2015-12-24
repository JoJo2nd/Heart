/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "hDynamicTileSet2D.h"
#include "render\hTextureResource.h"
#include "render\hTextureFlags.h"
#include "render\hMipDesc.h"
#include "render\hRenderer.h"

namespace Heart {

hRegisterObjectType(hDynamicTileSet2D, Heart::hDynamicTileSet2D, Heart::proto::DynamicTileSet2D);

hDynamicTileSet2D::hDynamicTileSet2D(Heart::proto::DynamicTileSet2D* obj) {
    tileSetWidth = obj->cachewidth();
    tileSetHeight = obj->cacheheight();
    minXSize = obj->mintilex();
    minYSize = obj->mintiley();
    textureFormat = hRenderer::convertTextureFormat(obj->textureformat(), true);
}

hBool hDynamicTileSet2D::serialiseObject(Heart::proto::DynamicTileSet2D* obj, const hSerialisedEntitiesParameters& params) const {
    bool srgb = true;
    obj->set_cachewidth(tileSetWidth);
    obj->set_cacheheight(tileSetHeight);
    obj->set_mintilex(minXSize);
    obj->set_mintiley(minYSize);
    obj->set_textureformat(hRenderer::convertTextureFormat(textureFormat, &srgb));
    return hTrue;
}

hBool hDynamicTileSet2D::linkObject() {
    if (!tileSet) {
        hRenderer::hMipDesc mip = { tileSetWidth, tileSetHeight, nullptr, 0 };
        tileSet.reset(hRenderer::createTexture2D(1, &mip, textureFormat, 0));
        textureAtlas.initialise(tileSetWidth, tileSetHeight, hRenderer::getTextureFormatBytesPerPixel(textureFormat), minXSize, minYSize);
    }
    return hTrue;
}

hDynamicTileSet2D::TileHandle hDynamicTileSet2D::addTextureToTileSet(hTextureResource* texture) {
    hcAssertMsg(texture->getTextureFormat() == textureFormat, "Incoming texture format and texture atlas format much match");
    auto* mip = texture->getCPUMipDesc(0);
    hcAssertMsg(mip, "Texture has no CPU data to copy");
    auto* atlas_node = textureAtlas.insert(mip->width, mip->height, mip->data);
    if (!atlas_node) return InvalidTileHandle;

    tiles.push_back(atlas_node);
    dirty = true;
    return (TileHandle)(tiles.size()-1);
}

hBool hDynamicTileSet2D::flushNewTextureData(hRenderer::hCmdList* cl) {
    if (!dirty) return hFalse;

    void* dst = hRenderer::getTexture2DMappingPtr(tileSet.get(), 0);
    if (!dst) return hFalse;
    hUint data_size = textureAtlas.getTextureDataSize();
    hMemCpy(dst, textureAtlas.getTextureDataPtr(), textureAtlas.getTextureDataSize());
    hRenderer::flushTexture2DMemoryRange(cl, tileSet.get(), 0, 0, data_size);
    dirty = false;
    return hTrue;
}

}