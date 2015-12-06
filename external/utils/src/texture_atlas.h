/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include <memory>
#include <vector>
#include <inttypes.h>

namespace utils {

struct AtlasNode {
    AtlasNode*       child[2] = {nullptr, nullptr}; //left, right children.
    uint16_t         x, y, w, h;  // image quad
};

class TextureAtlasBuilder {
    std::unique_ptr<uint8_t> textureData;
    std::vector<AtlasNode> atlasNodes;
    AtlasNode* root = nullptr;
    uint32_t textureWidth = 0;
    uint32_t textureHeight = 0;
    uint32_t bytesPerPixel = 0;
    uint32_t nodeReserve = 0;
    uint32_t allocated = 0;

    AtlasNode* atlasInsert(AtlasNode* pnode, uint16_t width, uint16_t height);
    AtlasNode* allocateNode() {
        if (allocated >= nodeReserve) {
            return nullptr;
        }
        atlasNodes.push_back(AtlasNode());
        return atlasNodes.data() + allocated++;
    }
    void* getDataLocation(uint32_t x, uint32_t y) {
        return (void*)(textureData.get() + (y*textureWidth*bytesPerPixel) + (x*bytesPerPixel));
    }

public:
    void initialise(uint32_t in_textureWidth, uint32_t in_textureHeight, uint32_t in_bytesPerPixel, uint32_t in_minXSize, uint32_t in_minYSize);
    const AtlasNode* insert(uint32_t width, uint32_t height, const void* data_ptr);
    void* getAtlasNodeDataMutable(const AtlasNode* node) { return getDataLocation(node->x, node->y); }
    const void* getTextureDataPtr() const { return textureData.get(); }
    void* getTextureDataPtrMutable() const { return textureData.get(); }
    void* getTextureLocationPtrMutable(uint32_t x, uint32_t y) { return getDataLocation(x, y); }
    uint32_t getTextureWidth() const { return textureWidth; }
    uint32_t getTextureHeight() const { return textureHeight; }
    uint32_t getTextureDataSize() const { return textureWidth*textureHeight*bytesPerPixel; }
    void clear();
};

}
