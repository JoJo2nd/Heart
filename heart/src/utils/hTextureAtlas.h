/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include <memory>
#include <vector>

namespace Heart {
namespace Util {

struct hAtlasNode {
    hAtlasNode() {
        child[0] = nullptr;
        child[1] = nullptr;
    }

    hAtlasNode*       child[2]; //left, right children.
    hUint16           x, y, w, h;  // image quad
};

class hTextureAtlasBuilder {
    std::unique_ptr<hByte> textureData;
    std::vector<hAtlasNode> atlasNodes;
    hAtlasNode* root;
    hUint textureWidth;
    hUint textureHeight;
    hUint bytesPerPixel;
    hUint nodeReserve;
    hUint allocated;

    hAtlasNode* atlasInsert(hAtlasNode* pnode, hUint16 width, hUint16 height);
    hAtlasNode* allocateNode() {
        if (allocated >= nodeReserve) {
            return nullptr;
        }
        atlasNodes.push_back(hAtlasNode());
        return atlasNodes.data() + allocated++;
    }
    void* getDataLocation(hUint x, hUint y) {
        return (void*)(textureData.get() + (x*textureHeight*bytesPerPixel) + (y*bytesPerPixel));
    }

public:
    hTextureAtlasBuilder() 
        : root(nullptr)
        , textureWidth(0)
        , textureHeight(0)
        , nodeReserve(0)
        , allocated(0)
    {}

    void initialise(hUint in_textureWidth, hUint in_textureHeight, hUint in_bytesPerPixel, hUint in_minXSize, hUint in_minYSize);
    const hAtlasNode* insert(hUint width, hUint height, const void* data_ptr);
    void* getAtlasNodeDataMutable(const hAtlasNode* node) { return getDataLocation(node->x, node->y); }
    const void* getTextureDataPtr() const { return textureData.get(); }
    void* getTextureDataPtrMutable() const { return textureData.get(); }
    hUint getTextureWidth() const { return textureWidth; }
    hUint getTextureHeight() const { return textureHeight; }
    void clear();
};

}
}