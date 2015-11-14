/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "utils/hTextureAtlas.h"
#include "base/hMemoryUtil.h"

namespace Heart {
namespace Util {

hAtlasNode* hTextureAtlasBuilder::atlasInsert(hAtlasNode* pnode, hUint16 width, hUint16 height) {
    if (!pnode) return nullptr;
    if (pnode->child[0] || pnode->child[1]) {
        //this pnode is occupied, try inserting to a child
        hAtlasNode* newnode = atlasInsert(pnode->child[0], width, height);
        if (!newnode) newnode = atlasInsert(pnode->child[1], width, height);
        return newnode;
    }
    else {
        // bail if we don't fit
        if (width > pnode->w || height > pnode->h) return nullptr;

        if (width == pnode->w && height == pnode->h) return pnode;// just copy
        else {// merge-split case
                //split case
            pnode->child[0] = allocateNode();
            pnode->child[1] = allocateNode();

            if (!pnode->child[0] || !pnode->child[1]) return nullptr; //no mem

            // decide which way to split
            hUint16 dw = pnode->w - (width);
            hUint16 dh = pnode->h - (height);

            if (dw > dh) { // split vertically
                pnode->child[0]->x = pnode->x; pnode->child[0]->w = width;
                pnode->child[0]->y = pnode->y + height; pnode->child[0]->h = pnode->h - height;

                pnode->child[1]->x = pnode->x + width; pnode->child[1]->w = pnode->w - width;
                pnode->child[1]->y = pnode->y; pnode->child[1]->h = pnode->h;
            } else { // split horizontally 
                pnode->child[0]->x = pnode->x + width; pnode->child[0]->w = pnode->w - width;
                pnode->child[0]->y = pnode->y; pnode->child[0]->h = height;

                pnode->child[1]->x = pnode->x; pnode->child[1]->w = pnode->w;
                pnode->child[1]->y = pnode->y + height; pnode->child[1]->h = pnode->h - height;
            }

            //insert into this node
            pnode->w = width;
            pnode->h = height;
            return pnode;
        }
    }
}

void hTextureAtlasBuilder::initialise(hUint in_textureWidth, hUint in_textureHeight, hUint in_bytesPerPixel, hUint in_minXSize, hUint in_minYSize) {
    if (textureHeight != in_textureHeight || textureWidth != in_textureWidth || bytesPerPixel != in_bytesPerPixel) {
        textureWidth = in_textureWidth;
        textureHeight = in_textureHeight;
        textureData.reset(new hByte[textureWidth*textureHeight*bytesPerPixel]);
    }
    hUint x_blocks = textureWidth / in_minXSize;
    hUint y_blocks = textureHeight / in_minYSize;
    if (x_blocks*y_blocks < nodeReserve) {
        nodeReserve = x_blocks*y_blocks;
        atlasNodes.reserve(nodeReserve);
    }
    clear();
}

const hAtlasNode* hTextureAtlasBuilder::insert(hUint width, hUint height, const void* data_ptr) {
    auto* node = atlasInsert(root, width, height);
    if (!node) return nullptr;
    const hByte* src = (const hByte*)data_ptr;
    if (!src) return node;
    for (hUint y = 0; y < height; ++y) {
        hMemCpy(getDataLocation(node->x, node->y + y), src, width*bytesPerPixel);
        src += width*bytesPerPixel;
    }
    return node;
}

void hTextureAtlasBuilder::clear() {
    root = allocateNode();
    root->x = 0;
    root->y = 0;
    root->w = textureWidth;
    root->h = textureHeight;
}

}
}