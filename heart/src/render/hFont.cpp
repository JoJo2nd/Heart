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

hBool hTTFFontFace::serialiseObject(Heart::proto::TTFResource* obj, const hSerialisedEntitiesParameters& params) const {
    return hTrue;
}

hTTFFontFace::hTTFFontFace(Heart::proto::TTFResource* obj) {
    hcAssertMsg(obj->has_ttfdata(), "invalid font face data!");
    TTFSize_ = obj->ttfdata().size();
    TTFData_.reset(new hByte[TTFSize_]);
    hMemCpy(TTFData_.get(), obj->ttfdata().c_str(), TTFSize_);
    auto* ft2l = hGetFreetype2Library();
    auto err = FT_New_Memory_Face(*ft2l, TTFData_.get(), (FT_Long)TTFSize_, 0, &face_);
    hcAssertMsg(!err, "Failed to create memory face");

    hCRC32::StartCRC32(&dataHash_, (const hChar*)TTFData_.get(), (FT_Long)TTFSize_);

    //hashBase_ = hCRC32::FullCRC((const hChar*)TTFData_.get(), (FT_Long)TTFSize_);
    //hashBase_ <<= 32;
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
    textureDim = hConfigurationVariables::getCVarUint("fontcache.cachesize", 1024);
    hUint minsize = hConfigurationVariables::getCVarUint("fontcache.minsize", 8);
    pad = hConfigurationVariables::getCVarUint("fontcache.glyphpadding", 2);
    atlasBuilder.initialise(textureDim, textureDim, 1, minsize, minsize);
    flush();
}

const hCachedGlyph* hFontRenderCache::getCachedGlyphBitmap(hTTFFontFace* face, hUint32 charcode) {
    auto gh = face->getGlyphHash(charcode);
    auto it = cachedGlyphs_.find(gh);
    if (it == cachedGlyphs_.end()) {
        auto ttfglyph = face->getGlyph(charcode);
        if (!ttfglyph) return nullptr;
        auto* entry = atlasBuilder.insert(ttfglyph->bitmap.width+(pad*2), ttfglyph->bitmap.rows+(pad*2), nullptr);
        if (!entry) return nullptr;// does flush need to be called?
        auto* tex_dst = (hByte*)atlasBuilder.getTextureDataPtrMutable();
        auto pitch = ttfglyph->bitmap.pitch;
        auto* srcbase = ttfglyph->bitmap.buffer; 
        for (auto y=0, yn=ttfglyph->bitmap.rows; y<yn; ++y) {
            auto* dst = tex_dst+(((entry->y+pad+y)*textureDim)+pad+entry->x);
            auto* src = srcbase+((pitch*y));
            for (auto x=0, xn=ttfglyph->bitmap.width; x<xn; ++x, ++dst, ++src) {
                *dst = *src;
            }
        }
        hCachedGlyph cg;
        //cg.metrics_ = ttfglyph->metrics;
        cg.width = (hFloat)ttfglyph->bitmap.width;
        cg.height = (hFloat)ttfglyph->bitmap.rows;
        cg.atlas_ = entry;
        cg.left = (hFloat)ttfglyph->bitmap_left;
        cg.top = (hFloat)ttfglyph->bitmap_top;
        cg.advanceX = (hFloat)(ttfglyph->advance.x >> 6);
        cg.advanceY = (hFloat)(ttfglyph->advance.y >> 6);
        cg.uv_[0] = (entry->x+pad)/(hFloat)textureDim;
        cg.uv_[1] = (entry->y+pad)/(hFloat)textureDim;
        cg.uv_[2] = (entry->x+pad+(entry->w-(pad*2))) / (hFloat)textureDim;
        cg.uv_[3] = (entry->y+pad+(entry->h-(pad*2))) / (hFloat)textureDim;
        cachedGlyphs_.insert(GlyphHashTable::value_type(gh, cg));
        it = cachedGlyphs_.find(gh);
    }

    return &it->second;
}

void  hFontRenderCache::flush() {
    atlasBuilder.clear();
}

const char* hFontRenderCache::getTextureData(hUint* hRestrict outwidth, hUint* hRestrict outheight) const {
    *outwidth = atlasBuilder.getTextureWidth();
    *outheight = atlasBuilder.getTextureHeight();
    return (const char*)atlasBuilder.getTextureDataPtr();
}

}
