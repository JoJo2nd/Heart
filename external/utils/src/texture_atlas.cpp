/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "texture_atlas.h"

namespace utils {

AtlasNode* TextureAtlasBuilder::atlasInsert(AtlasNode* pnode, uint16_t width, uint16_t height) {
    if (!pnode) return nullptr;
    if (pnode->child[0] || pnode->child[1]) {
        //this pnode is occupied, try inserting to a child
        AtlasNode* newnode = atlasInsert(pnode->child[0], width, height);
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
            uint16_t dw = pnode->w - (width);
            uint16_t dh = pnode->h - (height);

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

void TextureAtlasBuilder::initialise(uint32_t in_textureWidth, uint32_t in_textureHeight, uint32_t in_bytesPerPixel, uint32_t in_minXSize, uint32_t in_minYSize) {
    if (textureHeight != in_textureHeight || textureWidth != in_textureWidth || bytesPerPixel != in_bytesPerPixel) {
        textureWidth = in_textureWidth;
        textureHeight = in_textureHeight;
        bytesPerPixel = in_bytesPerPixel;
        textureData.reset(new uint8_t[textureWidth*textureHeight*bytesPerPixel]);
        memset(textureData.get(), 0, textureWidth*textureHeight*bytesPerPixel);
    }
    uint32_t x_blocks = textureWidth / in_minXSize;
    uint32_t y_blocks = textureHeight / in_minYSize;
    if (x_blocks*y_blocks > nodeReserve) {
        nodeReserve = x_blocks*y_blocks;
        atlasNodes.reserve(nodeReserve);
    }
    clear();
}

const AtlasNode* TextureAtlasBuilder::insert(uint32_t width, uint32_t height, const void* data_ptr) {
    auto* node = atlasInsert(root, width, height);
    if (!node) return nullptr;
    const uint8_t* src = (const uint8_t*)data_ptr;
    if (!src) return node;
    for (uint32_t y = 0; y < height; ++y) {
        memcpy(getDataLocation(node->x, node->y + y), src, width*bytesPerPixel);
        src += width*bytesPerPixel;
    }
    return node;
}

void TextureAtlasBuilder::clear() {
    root = allocateNode();
    root->x = 0;
    root->y = 0;
    root->w = textureWidth;
    root->h = textureHeight;
}

}
