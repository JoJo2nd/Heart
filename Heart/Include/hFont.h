/********************************************************************
	created:	2009/02/26

	filename: 	hrFont.h

	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef __HRFONT_H__
#define __HRFONT_H__

#include "hRenderer.h"
#include "hResource.h"

namespace Heart
{

	class hResourceDependencyList;
	class hTexture;
	class hCommandBufferList;

	struct hFontStyle
	{
		hUint32			Alignment_;
		hUint32			Colour_;
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
		hVec2				    UV1_;
	 	hVec2				    UV2_;
	};
}

	class hFont : public hResourceClassBase
	{
	public:

		hFont() {}
		virtual	~hFont() {}

		const hFontStyle&	GetFontStyle() const { return style_; }
		void				SetFontStyle( const hFontStyle& fontStyle ) { style_ = fontStyle; }
		hUint32				FontHeight() const { return fontHeight_; };
		hUint32				FontWidth() const { return fontWidth_; };
		hUint32				RenderString( hIndexBuffer& iBuffer, 
										  hVertexBuffer& vBuffer, 
										  const Heart::hVec2& topleft, 
										  const Heart::hVec2& bottomright, 
										  const hChar* str, 
										  hCommandBufferList* pCmdList );

		static hResourceClassBase*		OnFontLoad( const hChar* ext, hSerialiserFileStream* dataStream, hResourceManager* resManager );
		static hUint32		            OnFontUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager );

	private:

		friend class FontBuilder;

        HEART_ALLOW_SERIALISE_FRIEND();

		hBool						FitLine( Private::hFontLine& line, hFloat wid, const hChar* pStr );
		void						RenderLine( hIndexBuffer& iBuffer, hVertexBuffer& vBuffer, hUint16& iOffset, hUint32& vOffset, Private::hFontLine& line, hFloat cury, const hVec2& topleft, const hVec2& bottomright, hFloat w, hUint32& charsWritten );
		Private::hFontCharacter*	GetFontCharacter( hUint32 charcode );

	 	hFontStyle					style_;
	 	hUint32						nTexturePages_;
	 	hTexture*				    texturePages_;
        hMaterial*                  fontMaterial_;
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
        SERIALISE_ELEMENT_PTR_AS_INT( data.fontMaterial_ );
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
        DESERIALISE_ELEMENT_INT_AS_PTR( data.fontMaterial_ );
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