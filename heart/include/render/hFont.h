/********************************************************************

    filename: 	hFont.h	
    
    Copyright (c) 31:3:2012 James Moran
    
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    
    3. This notice may not be removed or altered from any source
    distribution.

*********************************************************************/
#ifndef __HRFONT_H__
#define __HRFONT_H__

class FontBuilder;

namespace Heart
{
    class hTexture;
    class hRenderSubmissionCtx;
    class hFont;

    struct hFontStyle
    {
        hUint32			Alignment_;
        hColour			Colour_;
        hUint32			Order_;
        hFloat          scale_;
        hFont*          font_;
    };

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

namespace Private
{
    struct hFontLine
    {
        hUint32					LineX_;
        hUint32					LineY_;
        hFloat					Width_;
        const hChar*			pStart_;
        const hChar*			pEnd_;
    };
}

#pragma pack(push, 1)

    // explicitly pad to 32 bytes
    // would be nice to get this under 32 bytes, might be possible if
    // units are pixel based, then could pack into elements into bytes.
    // I doubt many characters are more than 256 units wide or hight
    // in which case the possible min size is 28 bytes + page ID which could
    // be packed into ~3 bits.
    struct hFontCharacter
    {
        hUint32					page_;      //4;
        hUint32					unicode_;   //8;Decoded from UTF-8, where -1 is invalid
        hFloat                  x_;         //12;
        hFloat                  y_;         //16;
        hFloat					height_;    //20;
        hFloat					width_;     //24;
        hFloat                  xOffset_;   //28;
        hFloat                  yOffset_;   //32;
        hFloat					xAdvan_;    //36;
        hCPUVec2				UV1_;       //(8)44;
        hCPUVec2				UV2_;       //(8)52;
        hByte                   pad_[12];   //(12)64;
    };

#pragma pack(pop)

    typedef hUint64 hFontLookup;

    struct hFontVex
    {
        hCPUVec3    pos_;
        hColour     colour_;
        hCPUVec2    uv_;
    };

    class HEART_DLLEXPORT hFont : public hResourceClassBase
    {
    public:

        hFont(hMemoryHeapBase* heap) 
            : heap_(heap)
            , nTexturePages_(0)
            , texturePageResID_(0)
            , texturePages_(0)
            , fontMaterial_(NULL)
            , fontMaterialInstance_(NULL)
            , fontWidth_(0.f)
            , fontHeight_(0.f)
            , nMaxFontCharacters_(0)
            , nFontCharacters_(0)
            , fontCharacters_(NULL)
            , fontLookupSteps_(0)
            , fontLookup_(NULL)
        {}
        virtual	~hFont();

        void                SetFontHeight(hUint32 val) { fontHeight_ = val; }
        hUint32				GetFontHeight() const { return fontHeight_; }
        void                SetFontWidth(hUint32 val) { fontWidth_ = val; }
        hUint32				GetFontWidth() const { return fontWidth_; }
        void                SetPageCount(hUint32 val) { nTexturePages_ = val; }
        void                SetPageResourceID(hResourceID val) { texturePageResID_ = val; }
        void                SetMaterialResourceID(hResourceID val) { fontMaterialID_ = val; }
        void                SetFontCharacterLimit(hUint32 nChars);
        void                AddFontCharacter(const hFontCharacter* fchar);
        hMaterialInstance*  GetMaterialInstance() const { return fontMaterialInstance_; }
        void                SortCharacters();
        /*static hUint32      RenderString( const hFontStyle& sytle, 
                                          void* vBuffer,
                                          const hCPUVec2& topleft, 
                                          const hCPUVec2& bottomright, 
                                          hITextIterator* str);
        static hUint32      RenderStringSingleLine( const hFontStyle& style, 
                                                    void* vBuffer, 
                                                    const hCPUVec2& topleft, 
                                                    hITextIterator* str,
                                                    hUint32* bytesWritten = NULL,
                                                    hFloat widthLimit = FLT_MAX);
        hCPUVec2            CalcRenderSize(hITextIterator* str);*/
        hBool               Link(hResourceManager* resManager);
        const hFontCharacter*       GetFontCharacter( hUint32 charcode ) const;

    private:

        friend class ::FontBuilder;

        HEART_ALLOW_SERIALISE_FRIEND();

        hBool                       FitLine( Private::hFontLine& line, hFloat wid, const hChar* pStr );

        hMemoryHeapBase*            heap_;
        hUint32						nTexturePages_;
        hResourceID                 texturePageResID_;
        hTexture*				    texturePages_;
        hResourceID                 fontMaterialID_;
        hMaterial*                  fontMaterial_;
        hMaterialInstance*          fontMaterialInstance_;
        hUint32						fontWidth_;
        hUint32						fontHeight_;
        hUint32                     nMaxFontCharacters_;
        hUint32						nFontCharacters_;
        hFontCharacter*	            fontCharacters_;
        hUint32                     fontLookupSteps_;
        hFontLookup*                fontLookup_;
    };

}

#endif //__HRFONT_H__