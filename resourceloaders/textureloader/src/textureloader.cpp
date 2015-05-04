/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "nvtt/nvtt.h"
#include "FreeImage.h"
#include <fstream>
#include <string>
#include <stdio.h>
#include <vector>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};

#if defined (_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4244)
#   pragma warning(disable:4267)
#else
#   pragma error ("Unknown platform")
#endif
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/io/coded_stream.h"
#if defined (_MSC_VER)
#   pragma warning(pop)
#endif

#include "resource_common.pb.h"
#include "resource_texture.pb.h"
#include <memory>

#if defined PLATFORM_WINDOWS
#   define TB_API __cdecl
#elif PLATFORM_LINUX
#   if BUILD_64_BIT
#       define TB_API
#   else
#       define TB_API __attribute__((cdecl))
#   endif
#else
#   error
#endif

#if defined (PLATFORM_WINDOWS)
#   if defined (texture_builder_EXPORTS)
#       define DLL_EXPORT __declspec(dllexport)
#   else
#       define DLL_EXPORT __declspec(dllimport)
#   endif
#else
#   define DLL_EXPORT
#endif

namespace FreeImageFileIO
{
    unsigned int DLL_CALLCONV
        read_proc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
            return (unsigned int)fread(buffer, size, count, (FILE *)handle);
    }

    unsigned int DLL_CALLCONV
        write_proc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
            return (unsigned int)fwrite(buffer, size, count, (FILE *)handle);
    }

    int DLL_CALLCONV
        seek_proc(fi_handle handle, long offset, int origin) {
            return fseek((FILE *)handle, offset, origin);
    }

    long DLL_CALLCONV
        tell_proc(fi_handle handle) {
            return ftell((FILE *)handle);
    }
}

const char* textureFormats[] = {
    // No compression.
    "rgba",// Format_RGBA = Format_RGB,
    // // DX10 formats.
    "bc1",// Format_BC1 = Format_DXT1,
    "bc1a",// Format_BC1a = Format_DXT1a,
    "bc2",// Format_BC2 = Format_DXT3,
    "bc3",// Format_BC3 = Format_DXT5,
    "bc3n",// Format_BC3n = Format_DXT5n,
    "bc4",// Format_BC4,     // ATI1
    "bc5",// Format_BC5,     // 3DC, ATI2
};

const char* qualityFormats[] = {
    "fastest",      // Quality_Fastest,
    "normal",       // Quality_Normal,
    "production",   // Quality_Production,
    "highest",      // Quality_Highest,
};

#define getPitchFromWidth(w,bitsPerPixel) (( w * bitsPerPixel + 7 ) / 8)
size_t getDXTTextureSize(bool dxt1, size_t width, size_t height) {
    // compute the storage requirements
    size_t blockcount = ( ( width + 3 )/4 ) * ( ( height + 3 )/4 );
    size_t blocksize = (dxt1) ? 8 : 16;
    return blockcount*blocksize;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static bool writeOutTexture(const char* input_path, const char* output_path, bool gammacorrect, nvtt::Format format, nvtt::Quality quality) {
    FreeImageIO fiIO;
    fiIO.read_proc=&FreeImageFileIO::read_proc;
    fiIO.seek_proc=&FreeImageFileIO::seek_proc;
    fiIO.tell_proc=&FreeImageFileIO::tell_proc;
    fiIO.write_proc=&FreeImageFileIO::write_proc;

    struct TextureWriter : public nvtt::OutputHandler
    {
        TextureWriter(Heart::proto::TextureResource* resource) 
            : resource_(resource)
            , activeMip_(nullptr)
            , size_(0)
            , reserve_(0)
        {}
        /// Indicate the start of a new compressed image that's part of the final texture.
        virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) {
            activeMip_=resource_->add_mips();
            activeMip_->set_width(width);
            activeMip_->set_height(height);
            size_ = 0;
            if (size > reserve_) {
                data_.reset(new unsigned char[size]);
                reserve_ = size;
            }
        }

        /// Output data. Compressed data is output as soon as it's generated to minimize memory allocations.
        virtual bool writeData(const void * data, int size) {
            if (activeMip_) {
                if (size_ + size > reserve_) {
                    unsigned char* newdata = new unsigned char[(size_+size)*2];
                    memcpy(newdata, data, size_);
                    reserve_ = (size_+size)*2;
                    data_.reset(newdata);
                }
                memcpy(data_.get()+size_, data, size);
                size_+=size;
                activeMip_->set_data(data_.get(), size_);
            }

            return true;
        }

        Heart::proto::TextureResource*      resource_;
        Heart::proto::TextureMip*           activeMip_;
        std::unique_ptr<unsigned char>      data_;
        size_t                              size_;
        size_t                              reserve_;
    };

    FILE* file=fopen(input_path, "rb");
    if (file) {
        Heart::proto::TextureResource textureRes;
        FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromHandle(&fiIO, (fi_handle)file, 0);
        if(fif != FIF_UNKNOWN) {
            // load from the file handle
            FIBITMAP* dib = FreeImage_LoadFromHandle(fif, &fiIO, (fi_handle)file, 0);
            unsigned int width=FreeImage_GetWidth(dib);
            unsigned int height=FreeImage_GetHeight(dib);
            unsigned int bpp=FreeImage_GetBPP(dib);
            size_t bytes=width*height*4;
            unsigned char* data=new unsigned char[bytes];

            //Free image stores upside down so re-arrange
            switch(bpp) {
            case 32: {
                for(size_t y=height-1, d=0; y<height; --y) {
                    unsigned char* scanline=FreeImage_GetScanLine(dib, (int)y);
                    for(size_t x=0, slw=(width*4); x<slw; x+=4) {
                        data[d++]=scanline[x+0];
                        data[d++]=scanline[x+1];
                        data[d++]=scanline[x+2];
                        data[d++]=scanline[x+3];
                    }
                }
                break;
                        }
            case 24: {
                for(size_t y=height-1, d=0; y<height; --y) {
                    unsigned char* scanline=FreeImage_GetScanLine(dib, (int)y);
                    for(size_t x=0, slw=(width*3); x<slw; x+=3) {
                        data[d++]=scanline[x+0];
                        data[d++]=scanline[x+1];
                        data[d++]=scanline[x+2];
                        data[d++]=0xFF;
                    }
                }
                break;
                        }
            case 8: {
                for(size_t y=height-1, d=0; y<height; --y) {
                    unsigned char* scanline=FreeImage_GetScanLine(dib, (int)y);
                    for(size_t x=0; x<width; ++x) {
                        for (size_t i=0; i<4; ++i) {
                            data[d++]=scanline[x];
                        }
                    }
                }
                break;
                    }
            default:
                printf("Couldn't read pixel format with bpp of %d", bpp);
                return false;
            }

            textureRes.set_width(width);
            textureRes.set_height(height);
            textureRes.set_srgb(gammacorrect);
            textureRes.set_depth(1);
            switch(format) {
            case nvtt::Format_RGB: textureRes.set_format(Heart::proto::RGBA8_unorm); break;
            case nvtt::Format_BC1: textureRes.set_format(Heart::proto::BC1_unorm); break;
            case nvtt::Format_BC1a: textureRes.set_format(Heart::proto::BC1a_unorm); break;
            case nvtt::Format_BC2: textureRes.set_format(Heart::proto::BC2_unorm); break;
            case nvtt::Format_BC3: textureRes.set_format(Heart::proto::BC3_unorm); break;
            case nvtt::Format_BC3n: textureRes.set_format(Heart::proto::BC3_unorm); break;
            case nvtt::Format_BC4: textureRes.set_format(Heart::proto::BC4_unorm); break;
            case nvtt::Format_BC5: textureRes.set_format(Heart::proto::BC5_unorm); break;
            }

            nvtt::InputOptions inputOptions;
            inputOptions.setTextureLayout(nvtt::TextureType_2D, width, height);
            inputOptions.setMipmapFilter(nvtt::MipmapFilter_Kaiser);
            inputOptions.setKaiserParameters(3.0f, 4.0f, 1.0f);
            inputOptions.setMipmapGeneration(true);
            if (gammacorrect) {
                inputOptions.setGamma(2.2f, 2.2f);
            } else {
                inputOptions.setGamma(1.f, 1.f);
            }
            if (format == nvtt::Format_BC1a || format == nvtt::Format_BC5 || format == nvtt::Format_DXT1a ||
                format == nvtt::Format_DXT5 || format == nvtt::Format_RGBA) {
                inputOptions.setAlphaMode(nvtt::AlphaMode_Transparency);
            } else {
                inputOptions.setAlphaMode(nvtt::AlphaMode_None);
            }
            inputOptions.setMipmapData(data, width, height);

            TextureWriter texWriter(&textureRes);
            nvtt::OutputOptions outputOptions;
            outputOptions.setOutputHandler(&texWriter);

            nvtt::CompressionOptions compresserOptions;
            compresserOptions.setFormat(format);
            compresserOptions.setQuality(quality);

            nvtt::Compressor compressor;
            compressor.process(inputOptions, compresserOptions, outputOptions);

            FreeImage_Unload(dib);
            delete[] data; 
            data=nullptr;

            std::ofstream output;
            output.open(output_path, std::ios_base::out|std::ios_base::binary);
            if (!output.is_open()) {
                printf("Couldn't open %s for writing", output_path);
                return false;
            }

            {
                google::protobuf::io::OstreamOutputStream filestream(&output);
                google::protobuf::io::CodedOutputStream outputstream(&filestream);
                Heart::proto::MessageContainer msgContainer;
                msgContainer.set_type_name(textureRes.GetTypeName());
                msgContainer.set_messagedata(textureRes.SerializeAsString());
                msgContainer.SerializePartialToCodedStream(&outputstream);
            }
            output.close();
        }
        fclose(file);
    }
    else {
        //error
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int TB_API textureCompile(lua_State* L) {
    /* Args from Lua 1: input file, 2: dep files table, 3: parameter table, 4: outputpath)*/
    const char* input_path = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    const char* output_path = luaL_checkstring(L, 4);
    bool gammaCorrect = false;
    nvtt::Format texFmt;
    nvtt::Quality quality;

    lua_getfield(L, 3, "sRGB");
    if (lua_isboolean(L, -1)) {
        gammaCorrect = lua_toboolean(L, -1) != 0;
    }
    lua_pop(L, 1);

    lua_getfield(L, 3, "format");
    texFmt = (nvtt::Format)luaL_checkoption(L, -1, "bc1", textureFormats);
    lua_pop(L, 1);

    lua_getfield(L, 3, "quality");
    quality = (nvtt::Quality)luaL_checkoption(L, -1, "normal", qualityFormats);
    lua_pop(L, 1);

    if (!writeOutTexture(input_path, output_path, gammaCorrect, texFmt, quality)) {
        return luaL_error(L, "Texture build failed");
    }

    //return an empty table
    lua_newtable(L);
    return 1;
}

extern "C" {

    int TB_API version(lua_State* L) {
        lua_pushstring(L, "1.0.0");
        return 1;
    }

//Lua entry point calls
DLL_EXPORT int TB_API luaopen_texture(lua_State *L) {
    static const luaL_Reg texturelib[] = {
        {"build",textureCompile},
        { "version", version },
        {NULL, NULL}
    };
    luaL_newlib(L, texturelib);
    return 1;
}
}
