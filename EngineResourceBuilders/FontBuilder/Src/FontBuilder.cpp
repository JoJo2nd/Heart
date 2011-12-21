/********************************************************************
	created:	2010/09/05
	created:	5:9:2010   9:38
	filename: 	FontBuilder.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "FontBuilder.h"
#include "Heart.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
const hFloat			FontBuilder::TO_PIXELS				= ( 1.0f / 64.0f );
const hChar*    		FontBuilder::FREETYPE_FILE_PARAM	= "Input";
const hChar*            FontBuilder::FONT_SIZE_PARAM		= "Font Size";
const hChar*            FontBuilder::FONT_CHAR_ARRAY		= "Character List";

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

FontBuilder::FontBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo ) 
    : gdResourceBuilderBase( resBuilderInfo )
    , maxX_( 0 )
	, maxY_( 0 )
	, space_( 0 )
	, baselineOffset_( 0 )
    , fontBitmapData_(NULL)
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

FontBuilder::~FontBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void FontBuilder::BuildResource()
{
	if ( FT_Init_FreeType( &library_ ) )
	{
		ThrowFatalError( "Unable to create FreeType Library" );
	}

	CheckParameters();

	if ( FT_New_Face( library_, GetInputFile()->GetPath(), 0, &face_ ) )
	{
		ThrowFatalError( "Failed to create FreeType face from file: %s", GetInputFile()->GetPath() );
	}

	face_->style_flags = FT_STYLE_FLAG_BOLD;

	if ( !FT_HAS_HORIZONTAL( face_ ) )
	{
		ThrowFatalError( "Font is not horizontal" );
	}

	if ( FT_Set_Pixel_Sizes( face_, 0, fontSize_ ) )
	{
		ThrowFatalError( "Failed to set font size %d. make sure font size is not set in font file?" );
	}

	GetCharacterData();

	GenerateFontBitmap();

    Heart::hFont font;
    
    font.nTexturePages_ = 1;
    font.spaceWidth_ = space_;
    font.baseLine_ = baselineOffset_;
    font.fontWidth_ = maxX_;
    font.fontHeight_ = maxY_;
    font.nFontCharacters_ = fontCharacterData_.size();
    font.fontSourceWidth_ = finalWidth_;
    font.fontSourceHeight_ = finalHeight_;
    font.fontSourceSize_ = finalSize_;
    font.fontSourceData_ = (hByte*)fontBitmapData_;
    font.fontCharacters_ = new Heart::Private::hFontCharacter[font.nFontCharacters_];

    for ( hUint32 i = 0; i < font.nFontCharacters_; ++i )
    {
		font.fontCharacters_[i].BaseLine_ = fontCharacterData_[ i ].BaselineOffset_;
 	    font.fontCharacters_[i].Page_ = 0;
		font.fontCharacters_[i].Width_ = fontCharacterData_[ i ].Width_;
		font.fontCharacters_[i].Height_ = fontCharacterData_[ i ].Height_;
 	    font.fontCharacters_[i].xAdvan_ = fontCharacterData_[ i ].XAdvan_;
		font.fontCharacters_[i].UV1_.x = fontCharacterData_[ i ].U1_;
		font.fontCharacters_[i].UV1_.y = -fontCharacterData_[ i ].V1_;
		font.fontCharacters_[i].UV2_.x = fontCharacterData_[ i ].U2_;
		font.fontCharacters_[i].UV2_.y = -fontCharacterData_[ i ].V2_;
		font.fontCharacters_[i].CharCode_ = fontCharacterData_[ i ].Char_;
	}

    GetSerilaiseObject()->Serialise( GetOutputFile(), font );

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void FontBuilder::CleanUpFromBuild()
{
    delete fontBitmapData_;
    FT_Done_FreeType( library_ );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void FontBuilder::GetCharacterData()
{
	maxX_ = 0;
	maxY_ = 0;
	hUint32 averagew = 0;
	const char* c = characters_.c_str();
	const char* cend = characters_.c_str() + characters_.length();

	//////////////////////////////////////////////////////////////////////////
	// Read through the character string decoding each element into a ////////
	// unicode character /////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	while ( c < cend )
	{
		hUint16 unicode;
		CharData charData;
        c += Heart::hUTF8::DecodeToUnicode( c, unicode );

		int index = FT_Get_Char_Index( face_, unicode );

		if ( !index )
		{
			ThrowFatalError( "Character \"%c\"(%d) not found in TTF file", unicode, unicode );
		}

		if ( FT_Load_Glyph( face_, index , FT_LOAD_DEFAULT ) )
		{
			ThrowFatalError( "Failed to load glyph for index %d, character %c ", index, unicode );
		}

		if ( FT_Load_Char( face_, unicode, FT_LOAD_RENDER ) )
		{
			ThrowFatalError( "Failed to render glyph for character %c(%d)", unicode, unicode );
		}

		charData.Char_ = unicode;
		charData.Width_ = (hUint32)(face_->glyph->metrics.width * TO_PIXELS);
		charData.Height_ = (hUint32)(face_->glyph->metrics.height * TO_PIXELS);
		charData.XAdvan_ = (hUint32)(face_->glyph->metrics.horiAdvance * TO_PIXELS);
		charData.YAdvan_ = (hUint32)(face_->glyph->metrics.vertAdvance * TO_PIXELS);
		charData.BaselineOffset_ = (hUint32)(face_->glyph->metrics.horiBearingY * TO_PIXELS);
		charData.BaselineOffset_ = (hInt32)(charData.BaselineOffset_ - charData.Height_);

		averagew += charData.Width_;

		if ( charData.Width_ > maxX_ )
		{
			maxX_ = charData.Width_;
		}

		if ( charData.YAdvan_ > maxY_ )
		{
			maxY_ = charData.YAdvan_;
		}

		if ( baselineOffset_ > charData.BaselineOffset_ )
		{
			baselineOffset_ = charData.BaselineOffset_;
		}

		fontCharacterData_.push_back( charData );
	}

	space_ = averagew / characters_.size();
	baselineOffset_ *= -1;

	//sort in to character order
	std::sort( fontCharacterData_.begin(), fontCharacterData_.end()	);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void FontBuilder::CheckParameters()
{
	fontSize_	= GetParameter( FONT_SIZE_PARAM ).GetAsInt();
	characters_ = GetParameter( FONT_CHAR_ARRAY ).GetAsString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void FontBuilder::GenerateFontBitmap()
{
	// added 1 pixel buffers to each side of characters to cope with filtering
	maxX_ += 2;
	maxY_ += 2;
	//  calc needed params for texture gen
	hUint32 charsperline = PNG_WIDTH / maxX_;
	hUint32 lines = ( fontCharacterData_.size() / charsperline ) + 1;
	hUint32 neededtexsize = lines * maxY_;
	hUint32 power2texsize = Power2( neededtexsize );
	const hUint32 pixeldepth = 4;
	const hUint32 defaultpix = 0x00000000;// pixel with no alpha

	float halftexelx = ( 1.0f / PNG_WIDTH ) / 2.0f;
	float halftexely = ( 1.0f / power2texsize ) / 2.0f;

	fontBitmapData_ = new hUint32[ PNG_WIDTH * power2texsize ];
	fontBitmapRows_ = new hUint32*[ power2texsize ];
	hUint32*	tmp = fontBitmapData_;

    finalWidth_  = PNG_WIDTH;
    finalHeight_ = power2texsize;
    finalSize_   = PNG_WIDTH * power2texsize * sizeof(hUint32);

	for ( hUint32 i = 0; i < power2texsize; ++i )
	{
		fontBitmapRows_[ i ] = tmp;
		for ( int j = 0; j < PNG_WIDTH; ++j )
		{
			fontBitmapRows_[ i ][ j ] = defaultpix;
			++tmp;
		}
	}

	//  for each character, copy data into texture
	CharacterVector::const_iterator iend = fontCharacterData_.end();
	hUint32 cnt = 0;
	for ( CharacterVector::iterator i = fontCharacterData_.begin(); i < iend; ++i, ++cnt )
	{
		if ( FT_Load_Char( face_, i->Char_, FT_LOAD_RENDER ) )
		{
			ThrowFatalError( "Failed to render glyph for character %c(%d)", i->Char_, i->Char_ );
		}

		hUint32 curline = ( cnt / charsperline ) * maxY_;
		hUint32 curcol = ( cnt % charsperline ) * maxX_;
		FT_Bitmap* bm = &face_->glyph->bitmap;

		i->X_ = curcol;
		i->Y_ = curline + ( maxY_ - bm->rows );
		for ( hUint32 y = 0; y < maxY_; ++y )
		{
			for ( hUint32 x = 0; x < maxX_; ++x )
			{
				if ( ( y >= ( maxY_ - (hUint32)bm->rows ) ) && ( x < (hUint32)bm->width ) )
				{
					hUint32 py = curline + ( y );
					hUint32 px = x + curcol;
					hUint32 by = ( y - ( maxY_ - bm->rows ) ) * bm->width;
					hUint32 bx = x;
					unsigned char val = bm->buffer[ by + bx ];
					fontBitmapRows_[ py ][ px ] = ( val << 24 ) | ( val << 16 ) | ( val << 8 ) | ( val );

				}
			}
		}

		//Calculate UVs for each character
		i->U1_ = (static_cast< hFloat >( i->X_ )									/ static_cast< float >( PNG_WIDTH ))/* + halftexelx*/;
		i->U2_ = (static_cast< hFloat >( i->X_ + i->Width_ )						/ static_cast< float >( PNG_WIDTH ))/* - halftexelx*/; 
		i->V1_ = (static_cast< hFloat >( power2texsize - i->Y_ )					/ static_cast< float >( power2texsize ))/* + halftexely*/;
		i->V2_ = (static_cast< hFloat >( power2texsize - ( i->Y_ + i->Height_ ) )	/ static_cast< float >( power2texsize ))/* - halftexely*/;
	}
/*
	pngOutputName_ = GetResourceDependenciesPath() + "\\page0.png";

 	ResourceFile* fp = pResourceFileSystem()->OpenFile( pngOutputName_.c_str(), FILEMODE_WRITE );
	if ( !fp ) 
	{
		ThrowFatalError( "failed to open output file: %s", pngOutputName_ );
	}  

	png_structp png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );

	if ( !png_ptr )
	{
		ThrowFatalError( "failed to png write struct" );
	}

	png_infop info_ptr = png_create_info_struct( png_ptr );
	if ( !info_ptr )
	{
		png_destroy_write_struct( &png_ptr, ( png_infopp ) NULL );
		ThrowFatalError( "failed to create png info struct");
	}

	if ( setjmp( png_jmpbuf( png_ptr ) ) )
	{
		png_destroy_write_struct( &png_ptr, &info_ptr );
		pResourceFileSystem()->CloseFile( fp );
		ThrowFatalError( "failed to set longjmp" );
	}

	//  tell libpng how to write to our file
	png_set_write_fn( png_ptr, fp, &FontBuilder::pngWrite, &FontBuilder::pngFlush );

	png_set_compression_level( png_ptr, Z_BEST_COMPRESSION );

	png_set_IHDR( png_ptr, info_ptr, PNG_WIDTH, power2texsize,
		8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );

	png_set_rows( png_ptr, info_ptr, reinterpret_cast< png_bytepp >( fontBitmapRows_ ) );

	png_write_png( png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL ); 

	png_destroy_write_struct( &png_ptr, &info_ptr );

	delete[] fontBitmapData_;
	delete[] fontBitmapRows_;

	pResourceFileSystem()->CloseFile( fp );

	// Add dependency
	GameResource* pRes = AddDependency( "page0", pngOutputName_, "texture", texPage1ID_ );
	pRes->paramMap_[ "maxmipmaps" ] = "1";
*/
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void FontBuilder::pngWrite( png_structp pngptr, png_bytep pSrc, png_size_t len )
{
// 	ResourceFile* pFile = (ResourceFile*)png_get_io_ptr( pngptr );
// 
// 	pFile->Write( pSrc, len );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void FontBuilder::pngFlush( png_structp pngptr )
{
}