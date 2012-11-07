/********************************************************************

	filename: 	textureloader.cpp	
	
	Copyright (c) 21:7:2012 James Moran
	
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

#include "textureloader.h"
#include "png.h"
#include "libtga/tga.h"
#include "nvtt/nvtt.h"

using namespace Heart;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define TEXTURE_MAGIC_NUM              hMAKE_FOURCC('h','T','E','X')
#define TEXTURE_STRING_MAX_LEN         (32)
#define TEXTURE_MAJOR_VERSION          (((hUint16)1))
#define TEXTURE_MINOR_VERSION          (((hUint16)0))
#define TEXTURE_VERSION                ((TEXTURE_MAJOR_VERSION << 16)|TEXTURE_MINOR_VERSION)

#pragma pack(push, 1)

struct TextureHeader
{
    Heart::hResourceBinHeader   resHeader;
    hUint32                     version;
    hUint32                     width;
    hUint32                     height;
    hUint32                     depth;
    hUint32                     mipCount;
    Heart::hTextureFormat       format;
    Heart::ResourceFlags        flags;
};

#pragma pack(pop)

struct RawTextureData
{
    void*                   data_;      //Assumes RGBA or L8 format based on bytesPerPixel
    hUint32                 width_;
    hUint32                 pitch_;
    hUint32                 height_;
    hUint32                 mips_;
    hUint32                 bytesPerPixel_;
    hBool                   compressed_;
    Heart::hTextureFormat   format_;
};

#define DDSD_CAPS	        (0x1)       //Required in every .dds file.	0x1
#define DDSD_HEIGHT	        (0x2)       //Required in every .dds file.	0x2
#define DDSD_WIDTH	        (0x4)       //Required in every .dds file.	0x4
#define DDSD_PITCH	        (0x8)       //Required when pitch is provided for an uncompressed texture.	0x8
#define DDSD_PIXELFORMAT	(0x1000)    //Required in every .dds file.	0x1000
#define DDSD_MIPMAPCOUNT	(0x20000)   //Required in a mipmapped texture.	0x20000
#define DDSD_LINEARSIZE	    (0x80000)   //Required when pitch is provided for a compressed texture.	0x80000
#define DDSD_DEPTH	        (0x800000)  //Required in a depth texture.	0x800000

#define DDPF_ALPHAPIXELS	(0x1)       //Texture contains alpha data; dwRGBAlphaBitMask contains valid data.	0x1
#define DDPF_ALPHA	        (0x2)       //Used in some older DDS files for alpha channel only uncompressed data (dwRGBBitCount contains the alpha channel bitcount; dwABitMask contains valid data)	0x2
#define DDPF_FOURCC	        (0x4)       //Texture contains compressed RGB data; dwFourCC contains valid data.	0x4
#define DDPF_RGB	        (0x40)      //Texture contains uncompressed RGB data; dwRGBBitCount and the RGB masks (dwRBitMask, dwRBitMask, dwRBitMask) contain valid data.	0x40
#define DDPF_YUV	        (0x200)     //Used in some older DDS files for YUV uncompressed data (dwRGBBitCount contains the YUV bit count; dwRBitMask contains the Y mask, dwGBitMask contains the U mask, dwBBitMask contains the V mask)	0x200
#define DDPF_LUMINANCE	    (0x20000)   //Used in some older DDS files for single channel color uncompressed data (dwRGBBitCount contains the luminance channel bit count; dwRBitMask contains the channel mask). Can be combined with DDPF_ALPHAPIXELS for a two channel DDS file.	0x20000

struct TexFormatEnumName
{
    const hChar*            name_;
    nvtt::Format            fmt_;
    Heart::hTextureFormat   hFmt_;
};

TexFormatEnumName g_formatNames[] =
{
    // No compression.
    {"RGBA", nvtt::Format_RGBA, Heart::TFORMAT_ARGB8},// = Format_RGB,

    // DX9 formats.
    {"DXT1" , nvtt::Format_DXT1 , Heart::TFORMAT_DXT1},
    {"DXT1a", nvtt::Format_DXT1a, Heart::TFORMAT_DXT1},   // DXT1 with binary alpha.
    {"DXT3" , nvtt::Format_DXT3 , Heart::TFORMAT_DXT3},
    {"DXT5" , nvtt::Format_DXT5 , Heart::TFORMAT_DXT5},
    {"DXT5n", nvtt::Format_DXT5n, Heart::TFORMAT_DXT5},   // Compressed HILO: R=1, G=y, B=0, A=x

    // DX10 formats.
    {"BC1" , nvtt::Format_BC1 , Heart::TFORMAT_DXT1},
    {"BC1a", nvtt::Format_BC1a, Heart::TFORMAT_DXT1},
    {"BC2" , nvtt::Format_BC2 , Heart::TFORMAT_DXT3},
    {"BC3" , nvtt::Format_BC3 , Heart::TFORMAT_DXT5},
    {"BC3n", nvtt::Format_BC3n, Heart::TFORMAT_DXT5},
    {"BC4" , nvtt::Format_BC4 , Heart::TFORMAT_DXT5},     // ATI1       - NOTE: WRONG!
    {"BC5" , nvtt::Format_BC5 , Heart::TFORMAT_DXT5},     // 3DC, ATI2  - NOTE: WRONG!
};

#define GetPitchFromWidth(w,bitsPerPixel) (( w * bitsPerPixel + 7 ) / 8)

#pragma pack ( push, 1 )

struct DDSPixelFormat 
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    DWORD dwRGBBitCount;
    DWORD dwRBitMask;
    DWORD dwGBitMask;
    DWORD dwBBitMask;
    DWORD dwABitMask;
};

struct DDSHeader
{
    DWORD           dwMagic;
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwHeight;
    DWORD           dwWidth;
    DWORD           dwPitchOrLinearSize;
    DWORD           dwDepth;
    DWORD           dwMipMapCount;
    DWORD           dwReserved1[11];
    DDSPixelFormat  ddspf;
    DWORD           dwCaps;
    DWORD           dwCaps2;
    DWORD           dwCaps3;
    DWORD           dwCaps4;
    DWORD           dwReserved2;
};

#pragma pack ( pop )

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class HeartOutputHandler : public nvtt::OutputHandler
{
public:
    struct MipLevel
    {
        hUint32 size_;
        hUint32 width_;
        hUint32 height_;
        void*   data_;
    };

    HeartOutputHandler() 
        : doneHeader_(hFalse)
        , mipdata_(NULL)
        , mips_(0)
    {}

    ~HeartOutputHandler()
    {
        hDELETE_ARRAY_SAFE(GetGlobalHeap(), mipdata_);
    }

    /// Indicate the start of a new compressed image that's part of the final texture.
    void beginImage(int size, int width, int height, int depth, int face, int miplevel)
    {
        hcPrintf(__FUNCTION__"(size:%d,width:%d,height:%d,depth:%d,face:%d,miplevel:%d)",
            size,width,height,depth,face,miplevel);
        currentMip_ = miplevel;
        mipdata_[currentMip_].size_ = size;
        mipdata_[currentMip_].width_ = width;
        mipdata_[currentMip_].height_ = height;
        mipdata_[currentMip_].data_ = hNEW_ARRAY(GetGlobalHeap(), hByte, size);
        currentPtr_ = (hByte*)mipdata_[currentMip_].data_;
        currentEnd_ = currentPtr_ + size;

    }
    /// Output data. Compressed data is output as soon as it's generated to minimize memory allocations.
    bool writeData(const void * data, int size)
    {
        if (!doneHeader_)
        {
            hMemCpy(&header_, data, size);
            mips_ = (header_.dwFlags & DDSD_MIPMAPCOUNT) ? header_.dwMipMapCount : 1;
            mipdata_ = hNEW_ARRAY(GetGlobalHeap(), MipLevel, mips_);
            doneHeader_ = hTrue;
            currentMip_ = 0;
        }
        else
        {
            hMemCpy(currentPtr_, data, size);
            currentPtr_ += size;
            hcAssertMsg(currentPtr_ <= currentEnd_, "Buffer overwrite");
        }

        return hTrue;
    }

    DDSHeader   header_;
    hUint32     mips_;
    hBool       doneHeader_;
    MipLevel*   mipdata_;
    hUint32     currentMip_;
    hByte*      currentPtr_;
    hByte*      currentEnd_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void pngRead(png_structp, png_bytep, png_size_t);
hBool ReadTGAFileData(hIDataCacheFile* inFile, RawTextureData* outData, hResourceMemAlloc* memalloc);
hBool ReadDDSFileData(hIDataCacheFile* inFile, RawTextureData* outData, hResourceMemAlloc* memalloc);
hBool ReadPNGFileData(hIDataCacheFile* inFile, RawTextureData* outData, hResourceMemAlloc* memalloc);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool ReadPNGFileData(hIDataCacheFile* inFile, RawTextureData* outData, hResourceMemAlloc* memalloc)
{
	// Open the PNG file.
    char        sig[8];
    int         bitsPerPixel;
    int         imageType;
    png_structp pngStruct;
    png_infop   pngInfo;


	// Check the 8-byte signature
	inFile->Read( sig, 8 );
	if ( !png_check_sig((unsigned char*) sig, 8 ) )
	{
		hcAssertFailMsg( "Input File is not a valid png" );
        return hFalse;
	}

	pngStruct = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	if ( !pngStruct )
	{
		hcAssertFailMsg( "Failed to create png read struct" );
        return hFalse;
	}

	pngInfo = png_create_info_struct( pngStruct );
	if ( !pngInfo )
	{
		png_destroy_read_struct( &pngStruct, &pngInfo, NULL );
		hcAssertFailMsg( "Failed to create png info struct" );
		return hFalse;
	}

	//block to handle libpng errors, tho windows dont like setjump, may have to work round this [8/4/2008 James]
	if ( setjmp( png_jmpbuf( pngStruct ) ) )
	{
		png_destroy_read_struct( &pngStruct, &pngInfo, NULL );
		png_destroy_info_struct( pngStruct, &pngInfo );
		hcAssertFailMsg( "failed to set png error handle" );
        return hFalse;
	}

	// Set the custom file reader
	png_set_read_fn( pngStruct, (png_voidp)inFile, &pngRead );

	// lets libpng know that we already checked the 8 byte sig
	png_set_sig_bytes( pngStruct, 8 );

	// so far so good, read it in [8/4/2008 James]

	unsigned int  rowbytes =0;         // raw bytes at row n in image

	png_read_info( pngStruct, pngInfo );

	png_get_IHDR( pngStruct, 
		pngInfo, 
		static_cast< png_uint_32* >( &outData->width_ ), 
		static_cast< png_uint_32* >( &outData->height_ ), 
		&bitsPerPixel, 
		&imageType, 
		NULL, NULL, NULL );

    outData->pitch_ = outData->width_;

    if ( imageType == PNG_COLOR_TYPE_RGBA)
        outData->bytesPerPixel_ = 4;
    else if (imageType == PNG_COLOR_TYPE_RGB)
        outData->bytesPerPixel_ = 3;
    else if (imageType == PNG_COLOR_TYPE_GRAY)
        outData->bytesPerPixel_ = 1;
    else
    {
        hcAssertFailMsg("Invalid format");
        return hFalse;
    }

	// check the image is RGB/RGBA [8/7/2008 James]
	if ( bitsPerPixel != 8 )
	{
		png_destroy_read_struct( &pngStruct, &pngInfo, NULL );
		png_destroy_info_struct( pngStruct, &pngInfo );
		hcAssertFailMsg( "PNG image is not 8 bit per colour" );
		return hFalse;
	}

	// Update the png info struct.
	png_read_update_info( pngStruct, pngInfo );

	// Rowsize in bytes.
	rowbytes = png_get_rowbytes( pngStruct, pngInfo );

	// Allocate the image_data buffer.
	outData->data_ = hNEW_ARRAY(memalloc->tempHeap_, hByte, rowbytes*outData->height_);

	png_bytepp pngRows = hNEW_ARRAY(memalloc->tempHeap_, png_byte*, outData->height_ );

	// set the individual row_pointers to point at the correct offsets
	for ( hUint32 i = 0; i < outData->height_; ++i )
	{
		pngRows[ i ] = ( png_byte* )( outData->data_ ) + ( i * rowbytes );
	}

	// read in the entire image
	png_read_image( pngStruct, pngRows );

	// Clean up.

	png_destroy_read_struct( &pngStruct, &pngInfo, NULL );
	png_destroy_info_struct( pngStruct, &pngInfo );
	hDELETE_ARRAY_SAFE(memalloc->tempHeap_, pngRows);

	return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void pngRead(png_structp pngPtr, png_bytep pDst, png_size_t len)
{
    hIDataCacheFile* pFile = (hIDataCacheFile*)png_get_io_ptr(pngPtr);
	pFile->Read( pDst, len );
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
Heart::hResourceClassBase* HEART_API HeartBinLoader(Heart::hISerialiseStream* infile, Heart::hIDataParameterSet* params, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine)
{
    Heart::hTexture* texutre;
    Heart::hRenderer* renderer = engine->GetRenderer();
    Heart::hMipDesc* mips = NULL;
    hUint32 totalTextureSize = 0;
    hByte* textureData = NULL;
    TextureHeader header = {0};

    infile->Read(&header, sizeof(header));

    mips = (Heart::hMipDesc*)hAlloca(header.mipCount*sizeof(Heart::hMipDesc));

    //Read mip info
    infile->Read(mips, header.mipCount*sizeof(Heart::hMipDesc));

    //Add up the size need for textures
    for (hUint32 i = 0; i < header.mipCount; ++i)
    {
        mips[i].data = (hByte*)totalTextureSize;
        totalTextureSize += mips[i].size;
    }

    textureData = (hByte*)hHeapMalloc(memalloc->resourcePakHeap_, totalTextureSize);

    for (hUint32 i = 0; i < header.mipCount; ++i)
    {
        mips[i].data = textureData + (hUint32)(mips[i].data);
    }

    //Read Texture data
    infile->Read(textureData, totalTextureSize);

    renderer->CreateTexture(header.width, header.height, header.mipCount, mips, header.format, header.flags, memalloc->resourcePakHeap_, &texutre);

    return texutre;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartDataCompiler( Heart::hIDataCacheFile* inFile, Heart::hIBuiltDataCache* fileCache, Heart::hIDataParameterSet* params, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine, Heart::hISerialiseStream* binoutput )
{

    RawTextureData textureData;
    hUint32 len = Heart::hStrLen(params->GetInputFilePath());
    hBool gammaCorrect = hFalse;
    hBool buildMips = hFalse;
    hBool keepcpu = hFalse;
    hBool ddsInput = hFalse;
    HeartOutputHandler outputHandler;

    if (Heart::hStrICmp(&params->GetInputFilePath()[len-4], ".tga") == 0)
    {
        ReadTGAFileData(inFile, &textureData, memalloc);
    }
    else if (Heart::hStrICmp(&params->GetInputFilePath()[len-4], ".png") == 0)
    {
        ReadPNGFileData(inFile, &textureData, memalloc);
    }
    else if (Heart::hStrICmp(&params->GetInputFilePath()[len-4], ".dds") == 0)
    {
        ReadDDSFileData(inFile, &textureData, memalloc);
        ddsInput = hTrue;
    }
    else
    {
        hcAssertFailMsg("Unknown/Unsupported texture format");
    }

    if (Heart::hStrICmp(params->GetBuildParameter("sRGB", "true"),"true") == 0)
    {
        gammaCorrect = hTrue;
    }

    if (Heart::hStrICmp(params->GetBuildParameter("MIPMAPS", "true"),"true") == 0)
    {
        buildMips = hTrue;
    }

    if (Heart::hStrICmp(params->GetBuildParameter("KEEPCPU", "false"),"true") == 0)
    {
        keepcpu = hTrue;
    }

    Heart::hMipDesc* mips;
    hUint32 mipCount;
    if (!ddsInput)
    {
        nvtt::Format outFmt;
        const hChar* fmtString = params->GetBuildParameter("FORMAT", "DXT1");
        for (hUint32 i = 0; i < hStaticArraySize(g_formatNames); ++i)
        {
            if (Heart::hStrICmp(g_formatNames[i].name_, fmtString) == 0)
            {
                outFmt = g_formatNames[i].fmt_;
                textureData.format_ = g_formatNames[i].hFmt_;
                break;
            }
        }

        nvtt::InputOptions inputOptions;
        inputOptions.setTextureLayout(nvtt::TextureType_2D, textureData.width_, textureData.height_);
        inputOptions.setMipmapFilter(nvtt::MipmapFilter_Kaiser);
        inputOptions.setMipmapGeneration(buildMips);
        gammaCorrect ? inputOptions.setGamma(2.2f,2.2f) : inputOptions.setGamma(1.0f,1.0f);
        inputOptions.setMipmapData(textureData.data_, textureData.width_, textureData.height_);

        nvtt::OutputOptions outputOptions;
        outputOptions.setOutputHandler(&outputHandler);

        nvtt::CompressionOptions compresserOptions;
        compresserOptions.setFormat(outFmt);
        compresserOptions.setQuality(nvtt::Quality_Normal);

        nvtt::Compressor compressor;
        compressor.process(inputOptions,compresserOptions, outputOptions);

        /*
        * Create texture need the w/h sizes of each mip
        */

        mipCount = outputHandler.mips_;
        mips = (Heart::hMipDesc*)hAlloca(sizeof(Heart::hMipDesc)*mipCount);
        for (hUint32 i = 0; i < outputHandler.mips_; ++i)
        {
            mips[i].width = outputHandler.mipdata_[i].width_;
            mips[i].height = outputHandler.mipdata_[i].height_;
            mips[i].data = (hByte*)outputHandler.mipdata_[i].data_;
            mips[i].size = outputHandler.mipdata_[i].size_;
        }
    }
    else
    {
        //DDS input file
        mipCount = textureData.mips_;
        hUint32 bitsPerPixel = textureData.bytesPerPixel_*8;
        hUint32 w = textureData.width_;
        hUint32 h = textureData.height_;
        hUint32 size = textureData.compressed_ ? Heart::hTexture::GetDXTTextureSize(textureData.format_ == Heart::TFORMAT_DXT1, w,h) : GetPitchFromWidth(w,bitsPerPixel)*h;
        hByte* ptr = (hByte*)textureData.data_;

        mips = (Heart::hMipDesc*)hAlloca(sizeof(Heart::hMipDesc)*textureData.mips_);
        for (hUint32 i = 0; i < textureData.mips_; ++i)
        {
            mips[i].width = w;
            mips[i].height = h;
            mips[i].data = hNEW_ARRAY(memalloc->tempHeap_, hByte, size);
            mips[i].size = size;

            Heart::hMemCpy(mips[i].data, ptr, size);

            if (gammaCorrect && textureData.bytesPerPixel_ >=3 )
            {
                //If this texture is going ot be read as a sRGB, Red and Blue need to be swapped
                hByte* srgb = mips[i].data;
                for(hUint32 i = 0; i < size; i += textureData.bytesPerPixel_, srgb += textureData.bytesPerPixel_)
                {
                    hByte r = srgb[0];
                    srgb[0] = srgb[2];
                    srgb[2] = r;
                }
            }

            ptr += mips[i].size;
            w = hMax(w >> 1, 1);
            h = hMax(h >> 1, 1);
            size = textureData.compressed_ ? Heart::hTexture::GetDXTTextureSize(textureData.format_ == Heart::TFORMAT_DXT1, w,h) : GetPitchFromWidth(w,bitsPerPixel)*h;
        }

        hHeapFreeSafe(memalloc->tempHeap_, textureData.data_);
    }

    if (gammaCorrect)
    {
        textureData.format_ = (Heart::hTextureFormat)(textureData.format_ | Heart::TFORMAT_GAMMA_sRGB);
    }

    //Write Header
    TextureHeader header = {0};
    header.resHeader.resourceType = TEXTURE_MAGIC_NUM;
    header.version = TEXTURE_VERSION;
    header.width = textureData.width_;
    header.height = textureData.height_;
    header.depth = 0;
    header.mipCount = textureData.mips_;
    header.format = textureData.format_;
    header.flags = keepcpu ? Heart::RESOURCEFLAG_KEEPCPUDATA : (Heart::ResourceFlags)0;

    binoutput->Write(&header, sizeof(header));

    //Write mip info
    for (hUint32 i = 0; i < header.mipCount; ++i)
    {
        Heart::hMipDesc mdesc;
        mdesc = mips[i];
        mdesc.data = 0;
        binoutput->Write(&mdesc, sizeof(mdesc));
    }

    //Write Texture data
    for (hUint32 i = 0; i < header.mipCount; ++i)
    {
        binoutput->Write(mips[i].data, mips[i].size);
        //Release Data
        hDELETE_ARRAY_SAFE(memalloc->tempHeap_, mips[i].data);
    }

    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
hBool HEART_API HeartPackageLink( Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine )
{
    //Nothing to do
    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
void HEART_API HeartPackageUnlink( Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine )
{
    //Nothing to do
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DLL_EXPORT
void HEART_API HeartPackageUnload( Heart::hResourceClassBase* resource, Heart::hResourceMemAlloc* memalloc, Heart::hHeartEngine* engine )
{
    engine->GetRenderer()->DestroyTexture(static_cast<hTexture*>(resource));
}

//////////////////////////////////////////////////////////////////////////
// TGA helpers ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
size_t LTGA_CALLBACK TGAReadBytes(void* dst, size_t read, void* user)
{
    hIDataCacheFile* f = static_cast<hIDataCacheFile*>(user);
    return f->Read(dst, read);
}

size_t LTGA_CALLBACK TGASeekBytes(size_t offset, int from, void* user)
{
    hIDataCacheFile* f = static_cast<hIDataCacheFile*>(user);

    if (from == LTGA_SEEK_CUR)
        return f->Seek(offset, hIDataCacheFile::CURRENT);
    else if (from == LTGA_SEEK_END)
        return f->Seek(offset, hIDataCacheFile::END);
    else
        return f->Seek(offset, hIDataCacheFile::BEGIN);
}

void* LTGA_CALLBACK TGAmalloc(size_t size, void* user)
{
    Heart::hResourceMemAlloc* memalloc = (Heart::hResourceMemAlloc*)user;
    return hHeapMalloc(memalloc->tempHeap_, size);
}

void LTGA_CALLBACK TGAfree(void* ptr, void* user)
{
    Heart::hResourceMemAlloc* memalloc = (Heart::hResourceMemAlloc*)user;
    hHeapFreeSafe(memalloc->tempHeap_, ptr);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool ReadTGAFileData(hIDataCacheFile* inFile, RawTextureData* outData, hResourceMemAlloc* memalloc )
{
    TGADataReaderFuncs_t funcs;
    TGAImage_t* tgaImage;
    TGAImageInfo_t info;

    funcs.mallocFunc_ = TGAmalloc;
    funcs.freeFunc_ = TGAfree;
    funcs.mallocUser_ = memalloc;
    funcs.reader_ = TGAReadBytes;
    funcs.seek_ = TGASeekBytes;
    funcs.writer_ = NULL;

    tgaImage = TGACreateImage(funcs, (void*)inFile);

    if (!tgaImage)
        return hFalse;

    TGAReadImage(tgaImage);
    TGAGetImageInfo(tgaImage, &info);

    hUint32 size            = info.colourWidth_*info.width_*info.height_;
    outData->data_          = hHeapMalloc(memalloc->tempHeap_, size);
    outData->width_         = info.width_;
    outData->pitch_         = outData->width_;
    outData->height_        = info.height_;
    outData->bytesPerPixel_ = info.bbp_;
    outData->mips_          = 1;

    Heart::hMemCpy(outData->data_, TGAGetImageData(tgaImage), size);

    TGADestroyImage(tgaImage);

    return hTrue;
}

hBool ReadDDSFileData(hIDataCacheFile* inFile, RawTextureData* outData, hResourceMemAlloc* memalloc)
{
    DDSHeader header;
    hUint32 textureSize = inFile->Lenght() - sizeof(header);
    inFile->Read(&header, sizeof(header));

    if (header.dwMagic != 0x20534444)
    {
        hcAssertFailMsg("Incorrect magic number for DDS file");
        return hFalse;
    }

    outData->height_        = header.dwHeight;
    outData->width_         = header.dwWidth;
    outData->pitch_         = (header.dwFlags & DDSD_PITCH) ? header.dwPitchOrLinearSize : header.dwWidth;
    outData->mips_          = (header.dwFlags & DDSD_MIPMAPCOUNT) ? header.dwMipMapCount : 1;
    outData->compressed_    = hFalse;
    if ((header.ddspf.dwFlags & DDPF_FOURCC) == 0)
    {
        outData->bytesPerPixel_ = header.ddspf.dwRGBBitCount / 8;
        if (outData->bytesPerPixel_ == 1)
        {
            outData->format_        = Heart::TFORMAT_L8;
        }
        else if (outData->bytesPerPixel_ = 4)
        {
            outData->format_ = (header.ddspf.dwFlags & DDPF_ALPHA) ? Heart::TFORMAT_ARGB8 : Heart::TFORMAT_XRGB8;
        }
        else
        {
            hcAssertFailMsg("Incorrect format");
        }
    }
    else
    {
        outData->bytesPerPixel_ = 0;
        outData->compressed_    = hTrue;
        if ((MAKEFOURCC('D','X','T','1') == header.ddspf.dwFourCC) ||
            (MAKEFOURCC('D','X','T','2') == header.ddspf.dwFourCC))
        {
            outData->format_ = Heart::TFORMAT_DXT1;
        }
        else if ((MAKEFOURCC('D','X','T','3') == header.ddspf.dwFourCC) ||
                 (MAKEFOURCC('D','X','T','4') == header.ddspf.dwFourCC))
        {
            outData->format_ = Heart::TFORMAT_DXT3;
        }
        else if ((MAKEFOURCC('D','X','T','5') == header.ddspf.dwFourCC))
        {
            outData->format_ = Heart::TFORMAT_DXT5;
        }
        else
        {
            hcAssertFailMsg("Incorrect Compressed format");
        }
    }


    outData->data_ = hHeapMalloc(memalloc->tempHeap_, textureSize);
    inFile->Read(outData->data_, textureSize);

    return hTrue;
}
