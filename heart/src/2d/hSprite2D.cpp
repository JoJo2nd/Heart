/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "hSprite2D.h"
#include "core/hResourceManager.h"
#include "render/hTextureResource.h"

namespace Heart {

hRegisterObjectType(hSprite2D, Heart::hSprite2D, Heart::proto::RenderSprite2D);
std::vector<hSprite2D::SpriteBucket> hSprite2D::layers;

hSprite2D::hSprite2D(Heart::proto::RenderSprite2D* obj) {
    hUint32 render_order = obj->renderorder();
    layer = getLayerIndex(render_order);
    auto& l = layers[layer];
    entry = l.position.size();
    hSize_t new_size = entry + 1;
    l.position.resize(new_size);
    l.size.resize(new_size);
    l.spriteTile.resize(new_size);
    l.tileSet.resize(new_size);
    //
    hVec2 p(obj->x(), obj->y(), 0.f);
    hVec2 s(obj->width(), obj->height(), 0.f);
    setPosition(p);
    setSize(s);
    setTileHandle(hTextureAtlasResource::InvalidTileHandle);
    if (obj->tileid() >= 0) setTileHandle(obj->tileid());
    if (obj->has_tilesetresource()) {
        linkInfo.reset(new LoadLinkInfo());
        linkInfo->tileSetResource = hStringID(obj->tilesetresource().c_str());
    }
}

hBool hSprite2D::serialiseObject(Heart::proto::RenderSprite2D* obj) const {
    return hTrue;
}

hBool hSprite2D::linkObject() {
    if (linkInfo) {
        if (auto* ts_res = hResourceManager::weakResource<hTextureAtlasResource>(linkInfo->tileSetResource)) {
            setTileSet(ts_res);
            linkInfo.reset();
        }
    }
    return !linkInfo;
}

}