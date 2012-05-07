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

	struct hFontCharacter
	{
	 	hUint32					Page_;
	 	hUint32					CharCode_;
	 	hUint32					Height_;
	 	hUint32					Width_;
		hUint32					xAdvan_;
	 	hInt32					BaseLine_;
		hCPUVec2				UV1_;
	 	hCPUVec2				UV2_;
	};
}

	class hFont : public hResourceClassBase
	{
	public:

		hFont() 
            : fontSourceData_(NULL)
            , fontCharacters_(NULL)
        {}
		virtual	~hFont();

		const hFontStyle&	GetFontStyle() const { return style_; }
		void				SetFontStyle( const hFontStyle& fontStyle ) { style_ = fontStyle; }
		hUint32				FontHeight() const { return fontHeight_; };
        hUint32             FontBaseLine() const { return baseLine_; }
		hUint32				FontWidth() const { return fontWidth_; };
		hUint32				RenderString( hIndexBuffer& iBuffer, 
										  hVertexBuffer& vBuffer, 
										  const hCPUVec2& topleft, 
										  const hCPUVec2& bottomright, 
										  const hChar* str, 
										  hRenderSubmissionCtx* rnCtx );
        hUint32             RenderStringSingleLine(
            hIndexBuffer& iBuffer, 
            hVertexBuffer& vBuffer, 
            const hCPUVec2& topleft, 
            const hChar* str, 
            hRenderSubmissionCtx* rnCtx);
        hCPUVec2            CalcRenderSize(const hChar* str);

		static hResourceClassBase*		OnFontLoad( const hChar* ext, hUint32 resID, hSerialiserFileStream* dataStream, hResourceManager* resManager );
		static hUint32		            OnFontUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager );

	private:

        friend class ::FontBuilder;

        HEART_ALLOW_SERIALISE_FRIEND();

		hBool						FitLine( Private::hFontLine& line, hFloat wid, const hChar* pStr );
		void						RenderLine( hUint16** iBuffer, void** vBuffer, hUint16& vOffset, Private::hFontLine& line, hFloat cury, const hCPUVec2& topleft, const hCPUVec2& bottomright, hFloat w, hUint32& charsWritten );
		Private::hFontCharacter*	GetFontCharacter( hUint32 charcode );

	 	hFontStyle					style_;
	 	hUint32						nTexturePages_;
	 	hTexture*				    texturePages_;
        hMaterial*                  fontMaterial_;
        hMaterialInstance*          fontMaterialInstance_;
	 	hUint32						fontWidth_;
	 	hUint32						fontHeight_;
	 	hUint32						baseLine_;
	 	hUint32						spaceWidth_;
        hUint32                     fontSourceSize_;
        hUint32                     fontSourceWidth_;
        hUint32                     fontSourceHeight_;
        hByte*                      fontSourceData_;
	 	hUint32						nFontCharacters_;
		Private::hFontCharacter*	fontCharacters_;

	};

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hFont >( Heart::hSerialiser* ser, const Heart::hFont& data )
    {
        SERIALISE_ELEMENT( data.nTexturePages_ );
        SERIALISE_ELEMENT_RESOURCE_CRC( data.fontMaterial_ );
        SERIALISE_ELEMENT( data.fontWidth_ );
        SERIALISE_ELEMENT( data.fontHeight_ );
        SERIALISE_ELEMENT( data.baseLine_ );
        SERIALISE_ELEMENT( data.spaceWidth_ );
        SERIALISE_ELEMENT( data.fontSourceSize_ );
        SERIALISE_ELEMENT( data.fontSourceWidth_ );
        SERIALISE_ELEMENT( data.fontSourceHeight_ );
        SERIALISE_ELEMENT_COUNT( data.fontSourceData_, data.fontSourceSize_ );
        SERIALISE_ELEMENT( data.nFontCharacters_ );
        SERIALISE_ELEMENT_COUNT( data.fontCharacters_, data.nFontCharacters_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hFont >( Heart::hSerialiser* ser, Heart::hFont& data )
    {
        DESERIALISE_ELEMENT( data.nTexturePages_ );
        DESERIALISE_ELEMENT_RESOURCE_CRC( data.fontMaterial_ );
        DESERIALISE_ELEMENT( data.fontWidth_ );
        DESERIALISE_ELEMENT( data.fontHeight_ );
        DESERIALISE_ELEMENT( data.baseLine_ );
        DESERIALISE_ELEMENT( data.spaceWidth_ );
        DESERIALISE_ELEMENT( data.fontSourceSize_ );
        DESERIALISE_ELEMENT( data.fontSourceWidth_ );
        DESERIALISE_ELEMENT( data.fontSourceHeight_ );
        DESERIALISE_ELEMENT( data.fontSourceData_ );
        DESERIALISE_ELEMENT( data.nFontCharacters_ );
        DESERIALISE_ELEMENT( data.fontCharacters_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
   
    template<>
    inline void SerialiseMethod< Heart::Private::hFontCharacter >( Heart::hSerialiser* ser, const Heart::Private::hFontCharacter& data )
    {
        SERIALISE_ELEMENT( data.Page_ );
        SERIALISE_ELEMENT( data.CharCode_ );
        SERIALISE_ELEMENT( data.Height_ );
        SERIALISE_ELEMENT( data.Width_ );
        SERIALISE_ELEMENT( data.xAdvan_ );
        SERIALISE_ELEMENT( data.BaseLine_ );
        SERIALISE_ELEMENT( data.UV1_ );
        SERIALISE_ELEMENT( data.UV2_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::Private::hFontCharacter >( Heart::hSerialiser* ser, Heart::Private::hFontCharacter& data )
    {
        DESERIALISE_ELEMENT( data.Page_ );
        DESERIALISE_ELEMENT( data.CharCode_ );
        DESERIALISE_ELEMENT( data.Height_ );
        DESERIALISE_ELEMENT( data.Width_ );
        DESERIALISE_ELEMENT( data.xAdvan_ );
        DESERIALISE_ELEMENT( data.BaseLine_ );
        DESERIALISE_ELEMENT( data.UV1_ );
        DESERIALISE_ELEMENT( data.UV2_ );
    }
}

#endif //__HRFONT_H__