/********************************************************************
	created:	2008/06/14
	created:	14:6:2008   23:51
	filename: 	TextureBuilder.h
	author:		James Moran
	
	purpose:	
*********************************************************************/

#ifndef TextureBuilder_h__
#define TextureBuilder_h__

#include <stdio.h>
#include <vector>
#include "png.h"
#include "GameDatabaseLib.h"

class TextureBuilder : public GameData::gdResourceBuilderBase
{
public:
    TextureBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo );
    ~TextureBuilder();
    void    BuildResource();
    void    CleanUpFromBuild();

    static const gdChar*                ParameterName_Compression;
    static const gdChar*                ParameterName_GenerateMips;

private:

    enum ImageColourType
    {
        COLOURTYPE_GREYSCALE = 1,
        COLOURTYPE_GREYSCALEALPHA = 2,
        COLOURTYPE_RGB = 3,
        COLOURTYPE_RGBA = 4
    };

	static const int					MAX_MIPMAPS = 14;

    hBool                               ReadFileData();
	hBool								ReadPNGFileData();
    hBool                               ReadTGAFileData();
	void								GenerateMipMaps();
	void								GenerateMipMapLevel( hByte* parentlevel, hUint32 level, hUint32 parentwidth, hUint32 parentheight );
	void								CompressTextureMipLevels();
	void								BuildTextureResourceData();
	void								SwizzleColours();
	void								ConvertToOutputFormat();

	void AverageColourElement( hByte* &ppixel1, hByte* &ppixel2, hByte* &ppixel3, hByte* &ppixel4, hByte &C ) 
	{
		hUint32 tmp = 0;
		hUint32 c = 0;
		if ( ppixel1 ) {tmp += *ppixel1;++c;}
		if ( ppixel2 ) {tmp += *ppixel2;++c;}
		if ( ppixel3 ) {tmp += *ppixel3;++c;}
		if ( ppixel4 ) {tmp += *ppixel4;++c;}
		C = (hByte)(tmp / c);
		if ( ppixel1 ) { ++ppixel1; }
		if ( ppixel2 ) { ++ppixel2; }
		if ( ppixel3 ) { ++ppixel3; }
		if ( ppixel4 ) { ++ppixel4; }
	}

	static void							pngRead( png_structp pngPtr, png_bytep pDst, png_size_t len );

	typedef std::vector< hByte* >		MipVector;

	hByte*								SourcePixelData_;

	hUint32								ImageHeight_;
	hUint32								ImageWidth_;
	int									ImageBitsPerPixel_;
	int									ImageColourType_;

	hUint32								nLevels_;
	std::vector< hUint32 >				LevelSizes_;
	MipVector							MipMaps_;
    std::vector< hUint32 >				CompressedLevelSizes_;
    MipVector							CompressedMipMaps_;
	hUint32								maxMipMaps_;


};

#endif // TextureBuilder_h__