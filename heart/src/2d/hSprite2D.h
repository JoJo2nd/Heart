/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "core/hProtobuf.h"
#include "math/hVec2.h"
#include "components/hObjectFactory.h"
#include "components/hEntity.h"
#include "2d/hDynamicTileSet2D.h"
#include "resource_2d.pb.h"
#include <unordered_map>

namespace Heart {
class hTextureAtlasResource;

class hSprite2D : public hEntityComponent {
    struct LoadLinkInfo {
        hStringID tileSetResource;
    };

    hSize_t entry = 0;
    hSize_t layer = 0;
    std::unique_ptr<LoadLinkInfo> linkInfo;

    struct SpriteBucket {
        hUint32 layer;
        std::vector<hVec2> position;
        std::vector<hVec2> size;
        std::vector<hUint32> renderOrder;
        std::vector<hTextureAtlasResource*> tileSet;
        std::vector<hUint32> spriteTile;
    };

    static std::vector<SpriteBucket> layers;

    static void deepSwap(SpriteBucket* l, hSize_t a_entry, hSize_t b_entry) {
        if (a_entry == b_entry) return;
        std::swap(l->position[a_entry], l->position[b_entry]);
        std::swap(l->size[a_entry], l->size[b_entry]);
        std::swap(l->spriteTile[a_entry], l->spriteTile[b_entry]);
        std::swap(l->tileSet[a_entry], l->tileSet[b_entry]);
    }

    static hSize_t getLayerIndex(hUint32 render_order) {
        for (hSize_t i=0, n=layers.size(); i<n; ++i) {
            if (render_order == layers[i].layer) {
                return i;
            }
            if (render_order > layers[i].layer) {
                SpriteBucket new_bucket;
                new_bucket.layer = render_order;
                layers.insert(layers.begin()+i, std::move(new_bucket));
                return i;
            }
        }
        SpriteBucket new_bucket;
        new_bucket.layer = render_order;
        layers.push_back(std::move(new_bucket));
        return layers.size()-1;
    }

    friend class hSprite2DIterator;

public:
    hObjectType(hSprite2D, Heart::proto::RenderSprite2D);
    
    ~hSprite2D() {
        auto& l = layers[layer];
        hSize_t last_sprite_entry;
        last_sprite_entry = l.position.size()-1;
        deepSwap(&l, entry, last_sprite_entry);
        entry = last_sprite_entry;
        hcAssert(l.position.size()-1 == entry);
        l.position.resize(entry);
        l.size.resize(entry);
        l.spriteTile.resize(entry);
        l.tileSet.resize(entry);
    }
    
    hSprite2D(const hSprite2D& rhs) = delete;
    hSprite2D& operator = (const hSprite2D& rhs) = delete;
    
    hSize_t getEntry() const { return entry; }
    hSize_t getLayer() const { return layer; }
    hVec2 getSize() const { return layers[layer].size[entry]; }
    void setSize(hVec2 in_size) { layers[layer].size[entry] = in_size; }
    hVec2 getPosition() const { return layers[layer].position[entry]; }
    void setPosition(hVec2 in_pos) { layers[layer].position[entry] = in_pos; }
    hTextureAtlasResource* getTileSet() const { return layers[layer].tileSet[entry]; }
    void setTileSet(hTextureAtlasResource* in_tileset) { layers[layer].tileSet[entry] = in_tileset; }
    hDynamicTileSet2D::TileHandle getTileHandle() const { return layers[layer].spriteTile[entry]; }
    void setTileHandle(hUint32 in_tile) { layers[layer].spriteTile[entry] = in_tile; }

};

}