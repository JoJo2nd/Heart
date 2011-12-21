/********************************************************************
	created:	2010/09/05
	created:	5:9:2010   9:38
	filename: 	FontBuilder.h	
	author:		James
	
	purpose:	
*********************************************************************/

#ifndef FONTBUILDER_H__
#define FONTBUILDER_H__

#include <stdio.h>
#include <vector>
#include "GameDatabaseLib.h"
#include <ft2build.h>  
#include FT_FREETYPE_H
#include "png.h"

class FontBuilder : public GameData::gdResourceBuilderBase
{
public:

    FontBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo );
    ~FontBuilder();
    void    BuildResource();
    void    CleanUpFromBuild();

	static const hFloat					TO_PIXELS;//     ( 1.0f / 64.0f )
	static const hUint32				PNG_WIDTH = 1024;
	static const hChar*     			FREETYPE_FILE_PARAM;
	static const hChar*     			FONT_SIZE_PARAM;
	static const hChar*     			FONT_CHAR_ARRAY;

private:

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	struct CharData
	{
		hUint32     X_;
		hUint32     Y_;
		hUint32     Width_;
		hUint32     Height_;
		hUint32		XAdvan_;
		hUint32		YAdvan_;
		hInt32     	BaselineOffset_;
		hFloat     	U1_;
		hFloat     	U2_;
		hFloat		V1_;
		hFloat		V2_;
		hUint16     Char_;

		bool operator < ( const CharData& b )
		{
			return Char_ < b.Char_;
		}
	};

	typedef std::vector< CharData >		CharacterVector;

	void								GetCharacterData();
	void								GenerateFontBitmap();
	void								CheckParameters();
	hUint32								Power2( hUint32 i )
	{
		hUint32 j = 1;
		for (; ( j < i ); j <<= 1 );
		return j;
	}
	static void							pngWrite( png_structp pngptr, png_bytep pSrc, png_size_t len );
	static void							pngFlush( png_structp pngptr );

	FT_Library							library_;
	FT_Face								face_;

	std::string							ftFile_;
	std::string							characters_;//< Encoded in UTF-8
	hUint32								fontSize_;

	hUint32								maxX_;
	hUint32								maxY_;
    hUint32                             finalWidth_;
    hUint32                             finalHeight_;
    hUint32                             finalSize_;
	CharacterVector						fontCharacterData_;
	hInt32								baselineOffset_;
	hUint32								space_;
	hUint32*							fontBitmapData_;
	hUint32**							fontBitmapRows_;

	std::string							pngOutputName_;
	hUint32                             texPage1ID_;
};

#endif // FONTBUILDER_H__