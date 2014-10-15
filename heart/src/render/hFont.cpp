/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "render/hFont.h"
#include "base/hProfiler.h"
#include "base/hCRC32.h"
#include "core/hConfigOptions.h"

namespace Heart {
namespace FT2 {
    FT_Library freetype2Library;
}

hRegisterObjectType(TTFFontFace, Heart::hTTFFontFace, Heart::proto::TTFResource);

void hInitFreetype2() {
    auto err = FT_Init_FreeType(&FT2::freetype2Library);
    hcAssertMsg(err == 0, "Error creating Freetype2 library instance");
}

FT_Library* hGetFreetype2Library() {
    return &FT2::freetype2Library;
}

void hDestroyFreetype2() {
    FT_Done_FreeType(FT2::freetype2Library);
}

hTTFFontFace::~hTTFFontFace() {
    if (TTFData_) {
        FT_Done_Face(face_);
    }
}

hBool hTTFFontFace::serialiseObject(Heart::proto::TTFResource* obj) const {
    return hTrue;
}

hBool hTTFFontFace::deserialiseObject(Heart::proto::TTFResource* obj) {
    hcAssertMsg(obj->has_ttfdata(), "invalid font face data!");
    TTFSize_ = obj->ttfdata().size();
    TTFData_.reset(new hByte[TTFSize_]);
    hMemCpy(TTFData_.get(), obj->ttfdata().c_str(), TTFSize_);
    auto* ft2l = hGetFreetype2Library();
    auto err = FT_New_Memory_Face(*ft2l, TTFData_.get(), (FT_Long)TTFSize_, 0, &face_);
    if (err)
        return hFalse;

    hCRC32::StartCRC32(&dataHash_, (const hChar*)TTFData_.get(), (FT_Long)TTFSize_);

    //hashBase_ = hCRC32::FullCRC((const hChar*)TTFData_.get(), (FT_Long)TTFSize_);
    //hashBase_ <<= 32;

    return hTrue;
}

hBool hTTFFontFace::linkObject() {
    return hTrue;
}

void hTTFFontFace::setPixelSize(hUint size) {
    hUint32 newDataHash = dataHash_;
    hCRC32::ContinueCRC32(&newDataHash, (const hChar*)&size, sizeof(size));
    hashBase_ = hCRC32::FinishCRC32(&newDataHash);
    hashBase_ <<= 32;
    FT_Set_Pixel_Sizes(face_, size, size);
}

hGlyphHash hTTFFontFace::getGlyphHash(hUint32 charcode) {
    return (hashBase_ | charcode);
}

const hFontGlyph hTTFFontFace::getGlyph(hUint32 charcode) {
    /* retrieve glyph index from character code */
    auto glyph_index = FT_Get_Char_Index(face_, charcode);

    /* load glyph image into the slot (erase previous one) */
    auto error = FT_Load_Glyph(face_, glyph_index, FT_LOAD_DEFAULT);
    if ( error )
        return nullptr;
    /* convert to an anti-aliased bitmap */
    error = FT_Render_Glyph(face_->glyph, FT_RENDER_MODE_NORMAL);
    if ( error )
        return nullptr;

    // Pull out the information needed by for render cache
    return face_->glyph;
    // /* now, draw to our target surface */
    // my_draw_bitmap( &slot->bitmap,
    //                 pen_x + slot->bitmap_left,
    //                 pen_y - slot->bitmap_top );

    // /* increment pen position */
    // pen_x += slot->advance.x >> 6;
    // pen_y += slot->advance.y >> 6; /* not useful for now */
}


void  hFontRenderCache::initialise() {
    textureDim_ = hConfigurationVariables::getCVarUint("fontcache.cachesize", 1024);
    hUint minsize = hConfigurationVariables::getCVarUint("fontcache.minsize", 8);
    pad_ = hConfigurationVariables::getCVarUint("fontcache.glyphpadding", 2);
    nodeReserve_ = (textureDim_/minsize);
    nodeReserve_ *= nodeReserve_;
    textureCache_.reset(new char[textureDim_*textureDim_]);
    flush();
}

hGlyphBMNode* hFontRenderCache::cacheInsert(hGlyphBMNode* pnode, hUint16 width, hUint16 height) {
    if (!pnode)
        return nullptr;
    hGlyphBMNode* newnode = nullptr;
    if (pnode->child[0] || pnode->child[1]) {
        //this pnode is occupied, try inserting to a child
        newnode = cacheInsert(pnode->child[0], width, height);
        if (!newnode)
            newnode = cacheInsert(pnode->child[1], width, height);
        return newnode;
    } else {
        // bail if we don't fit
        if (width > pnode->w || height > pnode->h)
            return nullptr;

        if (width == pnode->w && height == pnode->h) {// just copy
            return pnode;
        } else {// merge-split case
            //split case
            pnode->child[0] = allocateNode();
            pnode->child[1] = allocateNode();

            if (!pnode->child[0] || !pnode->child[1])
                return nullptr; //no mem
            
            // decide which way to split
            hUint16 dw = pnode->w - (width);
            hUint16 dh = pnode->h - (height);
            
            if (dw > dh) { // split vertically
                pnode->child[0]->x = pnode->x; pnode->child[0]->w = width; 
                pnode->child[0]->y = pnode->y+height; pnode->child[0]->h = pnode->h-height; 

                pnode->child[1]->x = pnode->x+width; pnode->child[1]->w = pnode->w - width; 
                pnode->child[1]->y = pnode->y; pnode->child[1]->h = pnode->h; 
            } else { // split horizontally 
                pnode->child[0]->x = pnode->x+width; pnode->child[0]->w = pnode->w - width; 
                pnode->child[0]->y = pnode->y; pnode->child[0]->h = height; 

                pnode->child[1]->x = pnode->x; pnode->child[1]->w = pnode->w; 
                pnode->child[1]->y = pnode->y+height; pnode->child[1]->h = pnode->h - height; 
            }
            
            //insert into this node
            pnode->w = width;
            pnode->h = height;
            return pnode;
        }
    }
}

const hCachedGlyph* hFontRenderCache::getCachedGlyphBitmap(hTTFFontFace* face, hUint32 charcode) {
    auto gh = face->getGlyphHash(charcode);
    auto it = cachedGlyphs_.find(gh);
    if (it == cachedGlyphs_.end()) {
        auto ttfglyph = face->getGlyph(charcode);
        if (!ttfglyph)
            return nullptr;
        auto* entry = cacheInsert(root_, ttfglyph->bitmap.width+(pad_*2), ttfglyph->bitmap.rows+(pad_*2));
        if (!entry)// does flush need to be called?
            return nullptr;
        auto pitch = ttfglyph->bitmap.pitch;
        auto* srcbase = ttfglyph->bitmap.buffer; 
        for (auto y=0, yn=ttfglyph->bitmap.rows; y<yn; ++y) {
            auto* dst = textureCache_.get()+(((entry->y+pad_+y)*textureDim_)+pad_+entry->x);
            auto* src = srcbase+((pitch*y));
            for (auto x=0, xn=ttfglyph->bitmap.width; x<xn; ++x, ++dst, ++src) {
                *dst = *src;
            }
        }
        hCachedGlyph cg;
        cg.metrics_ = ttfglyph->metrics;
        cg.atlas_ = entry;
        // !!JM TODO: store UVs
        cachedGlyphs_.insert(GlyphHashTable::value_type(gh, cg));
        it = cachedGlyphs_.find(gh);
    }

    return &it->second;
}

void  hFontRenderCache::flush() {
    atlasNodes_.clear();
    atlasNodes_.reserve(nodeReserve_);
    allocated_ = 0;
    root_ = allocateNode();
    root_->x=0;
    root_->y=0;
    root_->w=textureDim_;
    root_->h=textureDim_;
    hZeroMem(textureCache_.get(), textureDim_*textureDim_);
}

const char* hFontRenderCache::getTextureData(hUint* hRestrict outwidth, hUint* hRestrict outheight) {
    *outwidth = *outheight = textureDim_;
    return textureCache_.get();
}

}
