/********************************************************************
	created:	2008/06/15
	created:	15:6:2008   1:06
	filename: 	TextureBuilder.cpp
	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "TextureBuilder.h"
#include "png.h"
#include "squish.h"
#include "Heart.h"

const gdChar* TextureBuilder::ParameterName_Compression = "Texture Compression";
const gdChar* TextureBuilder::ParameterName_GenerateMips = "Generate Mip Levels";


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TextureBuilder::TextureBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo )
    : gdResourceBuilderBase( resBuilderInfo )
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TextureBuilder::~TextureBuilder()
{
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureBuilder::BuildResource()
{
    Heart::hTexture texture(NULL);

    ReadPNGFileData();

    GenerateMipMaps();

    if ( GetParameter( ParameterName_Compression ).GetAsEnumValue().value_ != 0 )
    {
        CompressTextureMipLevels();

        hUint32 format = GetParameter( ParameterName_Compression ).GetAsEnumValue().value_;
        if ( format & squish::kDxt1 )
        {
            texture.format_ = Heart::TFORMAT_DXT1;
        }
        if ( format & squish::kDxt3 )
        {
            texture.format_ = Heart::TFORMAT_DXT3;
        }
        if ( format & squish::kDxt5 )
        {
            texture.format_ = Heart::TFORMAT_DXT5;
        }
    }
    else
    {
        texture.format_ = Heart::TFORMAT_ARGB8;
    }

    texture.nLevels_ = MipMaps_.size();
    texture.levelDescs_ = hNEW_ARRAY(hGeneralHeap, Heart::hTexture::LevelDesc, texture.nLevels_);

    hBool isCompressed = GetParameter( ParameterName_Compression ).GetAsEnumValue().value_ != 0;
    hUint32 w = ImageWidth_;
    hUint32 h = ImageHeight_;
    std::vector< hUint32 >& sizeArray = isCompressed ? CompressedLevelSizes_ : LevelSizes_;
    MipVector& mipData = isCompressed ? CompressedMipMaps_ : MipMaps_;
    hUint32 offset = 0;
    texture.totalDataSize_ = 0;
    for ( hUint32 i = 0; i < texture.nLevels_; ++i )
    {
        texture.totalDataSize_ += sizeArray[i];
    }

    texture.textureData_ = hNEW_ARRAY(hGeneralHeap, hByte,texture.totalDataSize_);
    for ( hUint32 i = 0; i < texture.nLevels_; ++i )
    {
        texture.levelDescs_[i].width_ = w;
        texture.levelDescs_[i].height_ = h;
        texture.levelDescs_[i].mipdata_ = (void*)offset;
        memcpy( texture.textureData_+offset, mipData[i], sizeArray[i] );

        w >>= 1;
        h >>= 1;
        offset += sizeArray[i];
    }
    
    GetSerilaiseObject()->Serialise( GetOutputFile(), texture );

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureBuilder::CleanUpFromBuild()
{
    delete SourcePixelData_;
}

void TextureBuilder::GenerateMipMaps()
{
	GenerateMipMapLevel( SourcePixelData_, 0, ImageWidth_, ImageHeight_ );
}

hBool TextureBuilder::ReadPNGFileData()
{
	// Open the PNG file.
    char  sig[8];
    png_structp pngStruct;
    png_infop   pngInfo_;
    GameData::gdFileHandle* textureFile;
	textureFile = GetInputFile();

	// Check the 8-byte signature
	textureFile->Read( sig, 8 );//fread( sig, 1, 8, TextureFile_ );
	if ( !png_check_sig(( unsigned char * ) sig, 8 ) )
	{
		ThrowFatalError( "Input File is not a valid png" );
	}

	pngStruct = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	if ( !pngStruct )
	{
		ThrowFatalError( "Failed to create png read struct" );
	}

	pngInfo_ = png_create_info_struct( pngStruct );
	if ( !pngInfo_ )
	{
		png_destroy_read_struct( &pngStruct, &pngInfo_, NULL );
		ThrowFatalError( "Failed to create png info struct" );
		return hFalse;
	}

	//block to handle libpng errors, tho windows dont like setjump, may have to work round this [8/4/2008 James]
	if ( setjmp( png_jmpbuf( pngStruct ) ) )
	{
		png_destroy_read_struct( &pngStruct, &pngInfo_, NULL );
		png_destroy_info_struct( pngStruct, &pngInfo_ );
		ThrowFatalError( "failed to set png error handle" );
	}

	// Set the custom file reader
	png_set_read_fn( pngStruct, (png_voidp)textureFile, &TextureBuilder::pngRead );

	// lets libpng know that we already checked the 8 byte sig
	png_set_sig_bytes( pngStruct, 8 );

	// so far so good, read it in [8/4/2008 James]

	unsigned int  rowbytes =0;         // raw bytes at row n in image

	png_read_info( pngStruct, pngInfo_ );

	png_get_IHDR( pngStruct, 
		pngInfo_, 
		static_cast< png_uint_32* >( &ImageWidth_ ), 
		static_cast< png_uint_32* >( &ImageHeight_ ), 
		&ImageBitsPerPixel_, 
		&ImageColourType_, 
		NULL, NULL, NULL );


	// check the image is RGB/RGBA [8/7/2008 James]
	if ( ImageBitsPerPixel_ != 8 )
	{
		png_destroy_read_struct( &pngStruct, &pngInfo_, NULL );
		png_destroy_info_struct( pngStruct, &pngInfo_ );
		ThrowFatalError( "PNG image is not 8 bit per colour" );
		return hFalse;
	}
	// check the image is power of 2 and small enough [8/7/2008 James]
	if ( ( ImageWidth_ & ( ImageWidth_ - 1 ) ) || ( ImageHeight_ & ( ImageHeight_ - 1 ) ) && 
		ImageHeight_ <= 1024 && ImageWidth_ <= 1024 )
	{
		if ( GetParameter( ParameterName_GenerateMips ).GetAsBool() )
		{
			png_destroy_read_struct( &pngStruct, &pngInfo_, NULL );
			png_destroy_info_struct( pngStruct, &pngInfo_ );
			ThrowFatalError( "PNG image size ( %d x %d ) is not a power of 2 or a size of 1024 x 1024 or less and has mipmaps", ImageWidth_, ImageHeight_ );
			return hFalse;
		}
	}

	// Update the png info struct.
	png_read_update_info( pngStruct, pngInfo_ );

	// Rowsize in bytes.
	rowbytes = png_get_rowbytes( pngStruct, pngInfo_ );

	// Allocate the image_data buffer.
	SourcePixelData_ = new hByte [ rowbytes * ImageHeight_ ];

	png_bytepp pngRows = new png_byte* [ ImageHeight_ ];

	// set the individual row_pointers to point at the correct offsets
	for ( hUint32 i = 0; i < ImageHeight_; ++i )
	{
		pngRows[ i ] = ( png_byte* )( SourcePixelData_ ) + ( i * rowbytes );
	}

	// read in the entire image
	png_read_image( pngStruct, pngRows );

	// Clean up.

	png_destroy_read_struct( &pngStruct, &pngInfo_, NULL );
	png_destroy_info_struct( pngStruct, &pngInfo_ );
	delete[] pngRows;

	return hTrue;
}

inline hByte* getPixel( hByte* texture, hUint32 width, hUint32 x, hUint32 y, hUint32 bbp )
{
	return &texture[ ( ( y * width ) * bbp ) + ( x * bbp ) ];
}

void TextureBuilder::GenerateMipMapLevel( hByte* parentlevel, hUint32 level, hUint32 width, hUint32 height )
{
	if ( width == 1 || height == 1 )
	{
		// we've done as much as we can or need to [8/7/2008 James]
		return;
	}
	
	nLevels_ = level + 1;

	hUint32 parentwidth = width * 2;
	hUint32 parentheight = height * 2;
	hUint32 pixelWidth = ImageColourType_ == PNG_COLOR_TYPE_RGB ? 3 : 4;
	hUint32 destPixelWidth = 4;
	const bool useAlpha = ImageColourType_ == PNG_COLOR_TYPE_RGBA;

	//only the base pixel level will ever be 3 bytes wide
	if ( level > 0 )
	{
		pixelWidth = 4;
	}

	LevelSizes_.push_back( width * height * destPixelWidth );
	MipMaps_.push_back( new hByte[ LevelSizes_[ level ] ] );// 4 for RGBA [8/7/2008 James]
	hByte* myptr = MipMaps_[ level ];

	for ( hUint32 i = 0; i < height; ++i )
	{
		for ( hUint32 j = 0; j < width; ++j )
		{
			hUint32 pi = i * 2;
			hUint32 pj = j * 2;
			hByte* ppixel1 = NULL;
			hByte* ppixel2 = NULL;
			hByte* ppixel3 = NULL;
			hByte* ppixel4 = NULL;
			hByte R,G,B,A;

			if ( level == 0 )
			{
				ppixel1 = getPixel( parentlevel, width, j, i, pixelWidth );
			}
			else
			{
				ppixel1 = getPixel( parentlevel, parentwidth, pj,   pi  , pixelWidth );
				ppixel2 = getPixel( parentlevel, parentwidth, pj+1, pi  , pixelWidth );
				ppixel3 = getPixel( parentlevel, parentwidth, pj,   pi+1, pixelWidth );
				ppixel4 = getPixel( parentlevel, parentwidth, pj+1, pi+1, pixelWidth );
			}

			// assuming R,G,B,A order [8/7/2008 James]
			// average the colours for the out put pixel [8/7/2008 James]
			AverageColourElement(ppixel1, ppixel2, ppixel3, ppixel4, R);
			AverageColourElement(ppixel1, ppixel2, ppixel3, ppixel4, G);
			AverageColourElement(ppixel1, ppixel2, ppixel3, ppixel4, B);
			if ( ImageColourType_ == PNG_COLOR_TYPE_RGBA )
			{
				AverageColourElement(ppixel1, ppixel2, ppixel3, ppixel4, A);
			}

			hByte* mypixel = getPixel( myptr, width, j, i, destPixelWidth );
			//Swizzeled pixel format
			*mypixel = R; ++mypixel;
			*mypixel = G; ++mypixel;
			*mypixel = B; ++mypixel;
			if ( useAlpha )
			{
				*mypixel = A; ++mypixel;
			}
			else 
			{
				*mypixel = 255; ++mypixel;
			}

		}
	}

	if ( GetParameter( ParameterName_GenerateMips ).GetAsBool() )
	{
		//don't generate mipmaps if not wanted!
		GenerateMipMapLevel( myptr, level + 1, width / 2, height / 2 );
	}

}

void TextureBuilder::CompressTextureMipLevels()
{
    // do the main image [8/7/2008 James]
    hUint32 w = ImageWidth_;
    hUint32 h = ImageHeight_;
    for ( hUint32 i = 0; i < MipMaps_.size(); ++i, w /= 2, h /= 2 )
    {
        hUint32 size = squish::GetStorageRequirements( w, h, GetParameter( ParameterName_Compression ).GetAsEnumValue().value_ );
        CompressedLevelSizes_.push_back( size );
        CompressedMipMaps_.push_back( new hByte[ size ] );
        squish::CompressImage( MipMaps_[ i ], w, h, CompressedMipMaps_[ i ], GetParameter( ParameterName_Compression ).GetAsEnumValue().value_ );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureBuilder::pngRead( png_structp pngPtr, png_bytep pDst, png_size_t len )
{
    GameData::gdFileHandle* pFile = (GameData::gdFileHandle*)png_get_io_ptr(pngPtr);
	pFile->Read( pDst, len );
}
