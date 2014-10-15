/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef __HRFONT_H__
#define __HRFONT_H__

#include "base/hTypes.h"
#include "base/hProtobuf.h"
#include "base/hStringID.h"
#include "base/hRendererConstants.h"
#include "components/hObjectFactory.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include <memory>
#include <unordered_map>

struct FT_LibraryRec_;

namespace Heart {
#if 0
    enum 
    {
        FONT_ALIGN_VCENTRE	= 1,//	along y axis
        FONT_ALIGN_LEFT		= 1 << 1,
        FONT_ALIGN_RIGHT	= 1 << 2,
        FONT_ALIGN_TOP		= 1 << 3,
        FONT_ALIGN_BOTTOM	= 1 << 4,
        FONT_ALIGN_HCENTRE	= 1 << 5, // alogn x axis
        FONT_ALIGN_FLIP	    = 1 << 6, // alogn x axis
    };
#endif
    
    void        hInitFreetype2();
    FT_Library* hGetFreetype2Library();
    void        hDestroyFreetype2();

    typedef FT_Glyph_Metrics hFontGlyphMetrics;
    typedef FT_GlyphSlot hFontGlyph;
    typedef hUint64 hGlyphHash;

    struct hGlyphBMNode {
        hGlyphBMNode() {
            child[0]=nullptr;
            child[1]=nullptr;
        }
        hGlyphBMNode*       child[2]; //left, right children
        hUint16             x, y, w, h;  // image quad
    };

    struct hCachedGlyph {
        hFontGlyphMetrics   metrics_;
        hGlyphBMNode*       atlas_;
        hFloat              uv_[4]; //top, left, bottom, right
    };

    class  hTTFFontFace {
    public:
        hObjectType(Heart::hTTFFontFace, Heart::proto::TTFResource);

        hTTFFontFace()
            : TTFSize_(0){

        }
        ~hTTFFontFace();

        void setPixelSize(hUint size);
        // The returned hFontGlyph is only until the next call to getGlyph
        const hFontGlyph getGlyph(hUint32 charcode);
        hGlyphHash getGlyphHash(hUint32 charcode);

    private:

        hSize_t                  TTFSize_;
        std::unique_ptr<hByte[]> TTFData_;
        hUint32                  dataHash_;
        hGlyphHash               hashBase_;
        FT_Face                  face_;
        hUint                    fontSize_;
    };

    class hFontRenderCache {
        typedef std::unordered_map<hGlyphHash, hCachedGlyph> GlyphHashTable;

        std::unique_ptr<char>       textureCache_;
        std::vector<hGlyphBMNode>   atlasNodes_;
        GlyphHashTable              cachedGlyphs_;
        hGlyphBMNode*               root_;
        hUint                       textureDim_;
        hUint                       pad_;
        hUint                       nodeReserve_;
        hUint                       allocated_;
        hUint16                     freelist_;

        hGlyphBMNode* cacheInsert(hGlyphBMNode* node, hUint16 width, hUint16 height);
        hGlyphBMNode* allocateNode() {
            if (allocated_ >= nodeReserve_) {
                return nullptr;
            }
            atlasNodes_.push_back(hGlyphBMNode());
            return atlasNodes_.data() + allocated_++;
        }

    public:
        hFontRenderCache()
            : root_(nullptr){

        }

        void  initialise();
        // return true when the texture needs to be flushed (that is, it's out of space)
        const hCachedGlyph* getCachedGlyphBitmap(hTTFFontFace* face, hUint32 charcode);
        const char* getTextureData(hUint* hRestrict outwidth, hUint* hRestrict outheight);
        void  flush();
    };
}

#endif //__HRFONT_H__