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

	struct hFontStyle
	{
		hUint32			Alignment_;
		hColour			Colour_;
		hUint32			Order_;
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

		const hFontStyle&	GetFontStyle() const { return style_; }
		void				SetFontStyle( const hFontStyle& fontStyle ) { style_ = fontStyle; }
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
        hUint32				RenderString( void* iBuffer, 
                                          void* vBuffer, 
                                          const hCPUVec2& topleft, 
                                          const hCPUVec2& bottomright, 
                                          const hChar* str );
        hUint32             RenderStringSingleLine( void* vBuffer, 
                                                    const hCPUVec2& topleft, 
                                                    const hChar* str );
        hCPUVec2            CalcRenderSize(const hChar* str);
        hBool               Link(hResourceManager* resManager);

	private:

        friend class ::FontBuilder;

        HEART_ALLOW_SERIALISE_FRIEND();

		hBool						FitLine( Private::hFontLine& line, hFloat wid, const hChar* pStr );
		void						RenderLine( hUint16** iBuffer, void** vBuffer, hUint16& vOffset, Private::hFontLine& line, hFloat cury, const hCPUVec2& topleft, const hCPUVec2& bottomright, hFloat w, hUint32& charsWritten );
		hFontCharacter*	            GetFontCharacter( hUint32 charcode );

        hMemoryHeapBase*            heap_;
	 	hFontStyle					style_;
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

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hFont >( Heart::hSerialiser* ser, const Heart::hFont& data )
    {
        SERIALISE_ELEMENT( data.nTexturePages_ );
        SERIALISE_ELEMENT( data.texturePageResID_ );
        SERIALISE_ELEMENT( data.fontMaterialID_ );
        SERIALISE_ELEMENT( data.fontWidth_ );
        SERIALISE_ELEMENT( data.fontHeight_ );
        SERIALISE_ELEMENT( data.nFontCharacters_ );
        SERIALISE_ELEMENT_COUNT( data.fontCharacters_, data.nFontCharacters_ );
        SERIALISE_ELEMENT( data.fontLookupSteps_ );
        SERIALISE_ELEMENT_COUNT( data.fontLookup_, data.nFontCharacters_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hFont >( Heart::hSerialiser* ser, Heart::hFont& data )
    {
        DESERIALISE_ELEMENT( data.nTexturePages_ );
        DESERIALISE_ELEMENT( data.texturePageResID_ );
        DESERIALISE_ELEMENT( data.fontMaterialID_ );
        DESERIALISE_ELEMENT( data.fontWidth_ );
        DESERIALISE_ELEMENT( data.fontHeight_ );
        DESERIALISE_ELEMENT( data.nFontCharacters_ );
        DESERIALISE_ELEMENT( data.fontCharacters_ );
        DESERIALISE_ELEMENT( data.fontLookupSteps_ );
        DESERIALISE_ELEMENT( data.fontLookup_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
   
    template<>
    inline void SerialiseMethod< Heart::hFontCharacter >( Heart::hSerialiser* ser, const Heart::hFontCharacter& data )
    {
        SERIALISE_ELEMENT( data.page_ );
        SERIALISE_ELEMENT( data.unicode_ );
        SERIALISE_ELEMENT( data.x_ );
        SERIALISE_ELEMENT( data.y_ );
        SERIALISE_ELEMENT( data.height_ );
        SERIALISE_ELEMENT( data.width_ );
        SERIALISE_ELEMENT( data.xOffset_ );
        SERIALISE_ELEMENT( data.yOffset_ );
        SERIALISE_ELEMENT( data.xAdvan_ );
        SERIALISE_ELEMENT( data.UV1_ );
        SERIALISE_ELEMENT( data.UV2_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hFontCharacter >( Heart::hSerialiser* ser, Heart::hFontCharacter& data )
    {
        DESERIALISE_ELEMENT( data.page_ );
        DESERIALISE_ELEMENT( data.unicode_ );
        DESERIALISE_ELEMENT( data.x_ );
        DESERIALISE_ELEMENT( data.y_ );
        DESERIALISE_ELEMENT( data.height_ );
        DESERIALISE_ELEMENT( data.width_ );
        DESERIALISE_ELEMENT( data.xOffset_ );
        DESERIALISE_ELEMENT( data.yOffset_ );
        DESERIALISE_ELEMENT( data.xAdvan_ );
        DESERIALISE_ELEMENT( data.UV1_ );
        DESERIALISE_ELEMENT( data.UV2_ );
    }
}

#endif //__HRFONT_H__