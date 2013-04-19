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
//#include "nvtt/nvtt.h"
#include <boost/filesystem.hpp>
#include <boost/smart_ptr.hpp>
#include <fstream>

using namespace Heart;

#ifndef MAKEFOURCC
#   define MAKEFOURCC(ch0, ch1, ch2, ch3)                           \
        ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |           \
        ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif /* defined(MAKEFOURCC) */

struct RawTextureData
{
    boost::shared_ptr<void> data_;      //Assumes RGBA or L8 format based on bytesPerPixel
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
#if 0
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
#endif

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

hBool ReadDDSFileData(const hChar* filepath, RawTextureData* outData);
#define getPitchFromWidth(w,bitsPerPixel) (( w * bitsPerPixel + 7 ) / 8)
hUint32 getDXTTextureSize(hBool dxt1, hUint32 width, hUint32 height) {
    // compute the storage requirements
    int blockcount = ( ( width + 3 )/4 ) * ( ( height + 3 )/4 );
    int blocksize = (dxt1) ? 8 : 16;
    return blockcount*blocksize;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int TB_API textureCompile(lua_State* L) {
    using namespace Heart;
    using namespace boost;
    /* Args from Lua (1: input files table, 2: dep files table, 3: parameter table, 4: outputpath)*/
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    luaL_checktype(L, 4, LUA_TSTRING);

    RawTextureData textureData;
    const hChar* filepath=hNullptr;
    hBool gammaCorrect = hFalse;
    hBool buildMips = hFalse;
    hBool keepcpu = hFalse;
    hBool ddsInput = hFalse;

    lua_getglobal(L, "buildpathresolve");
    lua_rawgeti(L, 1, 1);
    if(!lua_isstring(L, -1)) {
        luaL_error(L, "input path is not a string");
        return 0;
    }
    lua_call(L, 1, 1);
    filepath=lua_tostring(L, -1);
    lua_pop(L, 1);

    if (!ReadDDSFileData(filepath, &textureData)) {
        luaL_error(L, "Unable to read DDS file");
        return 0;
    }
    ddsInput = hTrue;

    lua_getfield(L, 3, "sRGB");
    if (!lua_isnil(L, -1) && lua_toboolean(L, -1)) {
        gammaCorrect = hTrue;
    }
    lua_pop(L, 1);

    lua_getfield(L, 3, "keepcpu");
    if (!lua_isnil(L, -1) && lua_toboolean(L, -1)) {
        keepcpu=hTrue;
    }
    lua_pop(L, 1);

    Heart::hMipDesc* mips;
    hUint32 mipCount;
    {
        //DDS input file
        mipCount = textureData.mips_;
        hUint32 bitsPerPixel = textureData.bytesPerPixel_*8;
        hUint32 w = textureData.width_;
        hUint32 h = textureData.height_;
        hUint32 size = textureData.compressed_ ? getDXTTextureSize(textureData.format_ == Heart::TFORMAT_DXT1, w,h) : getPitchFromWidth(w,bitsPerPixel)*h;
        hByte* ptr = (hByte*)textureData.data_.get();

        mips = (Heart::hMipDesc*)hAlloca(sizeof(Heart::hMipDesc)*textureData.mips_);
        for (hUint32 i = 0; i < textureData.mips_; ++i)
        {
            mips[i].width = w;
            mips[i].height = h;
            mips[i].data = new hByte[size];
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
            size = textureData.compressed_ ? getDXTTextureSize(textureData.format_ == Heart::TFORMAT_DXT1, w,h) : getPitchFromWidth(w,bitsPerPixel)*h;
        }
    }

    if (gammaCorrect)
    {
        textureData.format_ = (Heart::hTextureFormat)(textureData.format_ | Heart::TFORMAT_GAMMA_sRGB);
    }

    lua_getglobal(L, "buildpathresolve");
    lua_pushvalue(L, 4);
    lua_call(L, 1, 1);
    const hChar* outputpath=lua_tostring(L, -1);
    std::ofstream output;
    output.open(outputpath, std::ios_base::out|std::ios_base::binary);

    if (!output.is_open()) {
        luaL_error(L, "Unable to open output file %s for writing", outputpath);
        return 0;
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

    output.write((hChar*)&header, sizeof(header));

    //Write mip info
    for (hUint32 i = 0; i < header.mipCount; ++i)
    {
        Heart::hMipDesc mdesc;
        mdesc = mips[i];
        mdesc.data = 0;
        output.write((hChar*)&mdesc, sizeof(mdesc));
    }

    //Write Texture data
    for (hUint32 i = 0; i < header.mipCount; ++i)
    {
        output.write((hChar*)mips[i].data, mips[i].size);
        //Release Data
        delete[] mips[i].data;
    }
    output.close();
    lua_newtable(L); // push empty table, this resource won't have any dependencies
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool ReadDDSFileData(const hChar* filepath, RawTextureData* outData) {
    DDSHeader header;
    boost::system::error_code ec;
    hUint filesize=(hUint)boost::filesystem::file_size(filepath, ec);
    if (ec) {
        return hFalse;
    }
    hUint32 textureSize = filesize - sizeof(header);
    std::ifstream file;
    file.open(filepath);
    if (!file.is_open()) {
        return hFalse;
    }
    file.read((hChar*)&header, sizeof(header));

    if (header.dwMagic != 0x20534444) {
        return hFalse;
    }

    outData->height_        = header.dwHeight;
    outData->width_         = header.dwWidth;
    outData->pitch_         = (header.dwFlags & DDSD_PITCH) ? header.dwPitchOrLinearSize : header.dwWidth;
    outData->mips_          = (header.dwFlags & DDSD_MIPMAPCOUNT) ? header.dwMipMapCount : 1;
    outData->compressed_    = hFalse;
    if ((header.ddspf.dwFlags & DDPF_FOURCC) == 0) {
        outData->bytesPerPixel_ = header.ddspf.dwRGBBitCount / 8;
        if (outData->bytesPerPixel_ == 1) {
            outData->format_        = Heart::TFORMAT_L8;
        } else if (outData->bytesPerPixel_ = 4) {
            outData->format_ = (header.ddspf.dwFlags & DDPF_ALPHA) ? Heart::TFORMAT_ARGB8 : Heart::TFORMAT_XRGB8;
        } else {
            return hFalse;
        }
    } else {
        outData->bytesPerPixel_ = 0;
        outData->compressed_    = hTrue;
        if ((MAKEFOURCC('D','X','T','1') == header.ddspf.dwFourCC) ||
            (MAKEFOURCC('D','X','T','2') == header.ddspf.dwFourCC)) {
            outData->format_ = Heart::TFORMAT_DXT1;
        } else if ((MAKEFOURCC('D','X','T','3') == header.ddspf.dwFourCC) ||
                 (MAKEFOURCC('D','X','T','4') == header.ddspf.dwFourCC)) {
            outData->format_ = Heart::TFORMAT_DXT3;
        } else if ((MAKEFOURCC('D','X','T','5') == header.ddspf.dwFourCC)) {
            outData->format_ = Heart::TFORMAT_DXT5;
        } else {
            return hFalse;
        }
    }


    outData->data_ = boost::shared_ptr<void>(new hByte[textureSize]);
    file.read((hChar*)outData->data_.get(), textureSize);

    file.close();
    return hTrue;
}

extern "C" {
//Lua entry point calls
DLL_EXPORT int TB_API luaopen_texture(lua_State *L) {
    static const luaL_Reg texturelib[] = {
        {"compile",textureCompile},
        {NULL, NULL}
    };
    luaL_newlib(L, texturelib);
    return 1;
}
}
