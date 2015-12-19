/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "hSprite2D.h"
#include "core/hResourceManager.h"
#include "render/hTextureResource.h"
#include "render/hRenderer.h"
#include "shaders/hTileRenderer2DConstants.h"

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

void hSprite2D::setTileSet(hTextureAtlasResource* in_tileset) {
    layers[layer].tileSet[entry] = in_tileset;
    if (layers[layer].tileSet[entry]) {
        auto tile_data = layers[layer].tileSet[entry]->getAtlasImage(layers[layer].spriteTile[entry]);
        layers[layer].uvCoords[entry] = hVec4(tile_data.u1(), tile_data.v1(), tile_data.u2(), tile_data.v2());
    }
}

void hSprite2D::setTileHandle(hUint32 in_tile) {
    layers[layer].spriteTile[entry] = in_tile;
    if (layers[layer].tileSet[entry]) {
        auto tile_data = layers[layer].tileSet[entry]->getAtlasImage(in_tile);
        layers[layer].uvCoords[entry] = hVec4(tile_data.u1(), tile_data.v1(), tile_data.u2(), tile_data.v2());
    }
}

void hSprite2D::submitSpriteLayers(hUint32 start_layer, hUint32 end_layer, hRenderer::hCmdList* cl, hRenderer::hPipelineState* pls, hRenderer::hInputState* is, hUint32 texture_slot, hUint32 vertex_start, hTileRenderer2D::Vert2D* vtx_buffer, hUint32 max_vtx_count) {
    hUint32 written = 0;
    hUint32 draw_prims = 0;
    for (const auto& i : layers) {
        if (i.layer < start_layer) continue;
        if (i.layer >= end_layer) break;
        if (written+6 > max_vtx_count) break;

        for (hSize_t s=0, sn=i.position.size(); s < sn; ++s) {
            hVec2 total = i.position[s] + i.size[s];
            vtx_buffer[written].pos.x = i.position[s].getX();
            vtx_buffer[written].pos.y = i.position[s].getY();
            vtx_buffer[written].uv.x = hFloatToHalf(i.uvCoords[s].getX());
            vtx_buffer[written].uv.y = hFloatToHalf(i.uvCoords[s].getY());
            ++written;
            vtx_buffer[written].pos.x = total.getX();
            vtx_buffer[written].pos.y = i.position[s].getY();
            vtx_buffer[written].uv.x = hFloatToHalf(i.uvCoords[s].getZ());
            vtx_buffer[written].uv.y = hFloatToHalf(i.uvCoords[s].getY());
            ++written;
            vtx_buffer[written].pos.x = total.getX();
            vtx_buffer[written].pos.y = total.getY();
            vtx_buffer[written].uv.x = hFloatToHalf(i.uvCoords[s].getZ());
            vtx_buffer[written].uv.y = hFloatToHalf(i.uvCoords[s].getW());
            ++written;
            vtx_buffer[written].pos.x = i.position[s].getX();
            vtx_buffer[written].pos.y = i.position[s].getY();
            vtx_buffer[written].uv.x = hFloatToHalf(i.uvCoords[s].getX());
            vtx_buffer[written].uv.y = hFloatToHalf(i.uvCoords[s].getY());
            ++written;
            vtx_buffer[written].pos.x = total.getX();
            vtx_buffer[written].pos.y = total.getY();
            vtx_buffer[written].uv.x = hFloatToHalf(i.uvCoords[s].getZ());
            vtx_buffer[written].uv.y = hFloatToHalf(i.uvCoords[s].getW());
            ++written;
            vtx_buffer[written].pos.x = i.position[s].getX();
            vtx_buffer[written].pos.y = total.getY();
            vtx_buffer[written].uv.x = hFloatToHalf(i.uvCoords[s].getX());
            vtx_buffer[written].uv.y = hFloatToHalf(i.uvCoords[s].getW());
            ++written;
            draw_prims += 2;
            if (s+1 >= sn|| i.tileSet[s] != i.tileSet[s+1]) {
                hRenderer::setTextureOverride(cl, is, texture_slot, i.tileSet[s]->getTextureResource()->getTexture2D());
                hRenderer::draw(cl, pls, is, hRenderer::Primative::Triangles, draw_prims, vertex_start+written);
                draw_prims = 0;
            }
        }
    }
}

}