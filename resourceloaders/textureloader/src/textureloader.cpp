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
#include <iostream>

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
#include "builder.pb.h"
#include "getopt.h"
#include <memory>
#ifdef _WIN32
#   include <io.h>
#   include <fcntl.h>
#endif

static const char argopts[] = "vi:";
static struct option long_options[] = {
    { "version", no_argument, 0, 'z' },
    { 0, 0, 0, 0 }
};

#define fatal_error_check(x, msg, ...) if (!(x)) {fprintf(stderr, msg, __VA_ARGS__); exit(-1);}

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

struct EnumName {
    const char* name;
    int value;
};
#define define_enum_name(name, value) {name, (int)value}

EnumName textureFormats[] = {
    // No compression.
    define_enum_name("rgba", nvtt::Format_RGBA),// Format_RGBA = Format_RGB,
    // // DX10 formats.
    define_enum_name("bc1", nvtt::Format_BC1),// Format_BC1 = Format_DXT1,
    define_enum_name("bc1a", nvtt::Format_BC1),// Format_BC1a = Format_DXT1a,
    define_enum_name("bc2", nvtt::Format_BC2),// Format_BC2 = Format_DXT3,
    define_enum_name("bc3", nvtt::Format_BC3),// Format_BC3 = Format_DXT5,
    define_enum_name("bc3n", nvtt::Format_BC3n),// Format_BC3n = Format_DXT5n,
    define_enum_name("bc4", nvtt::Format_BC4),// Format_BC4,     // ATI1
    define_enum_name("bc5", nvtt::Format_BC5),// Format_BC5,     // 3DC, ATI2
};

EnumName qualityFormats[] = {
    define_enum_name("fastest",nvtt::Quality_Fastest),      // Quality_Fastest,
    define_enum_name("normal",nvtt::Quality_Normal),       // Quality_Normal,
    define_enum_name("production",nvtt::Quality_Production),   // Quality_Production,
    define_enum_name("highest",nvtt::Quality_Highest),      // Quality_Highest,
};

template <typename t_ty, size_t n>
t_ty getEnumFromName(const char* name, EnumName (&names)[n], t_ty def) {
    for (const auto i : names) {
        if (strcmp(name, i.name) == 0) {
            return (t_ty)i.value;
        }
    }
    return def;
}

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
static bool writeOutTexture(const char* input_path, bool gammacorrect, nvtt::Format format, nvtt::Quality quality) {
    Heart::builder::Output output;
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

            output.mutable_pkgdata()->set_type_name(textureRes.GetTypeName());
            output.mutable_pkgdata()->set_messagedata(textureRes.SerializeAsString());
        } else {
            return false;
        }
        fclose(file);
    } else {
        return false;
    }

    google::protobuf::io::OstreamOutputStream filestream(&std::cout);
    return output.SerializeToZeroCopyStream(&filestream);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif

    google::protobuf::io::IstreamInputStream input_stream(&std::cin);
    Heart::builder::Input input_pb;

    int c;
    int option_index = 0;
    bool verbose = false, use_stdin = true;

    while ((c = gop_getopt_long(argc, argv, argopts, long_options, &option_index)) != -1) {
        switch (c) {
        case 'z': fprintf(stdout, "heart texture builder v0.8.0"); exit(0);
        case 'v': verbose = 1; break;
        case 'i': {
            std::ifstream input_file_stream;
            input_file_stream.open(optarg, std::ios_base::binary | std::ios_base::in);
            if (input_file_stream.is_open()) {
                google::protobuf::io::IstreamInputStream file_stream(&input_file_stream);
                input_pb.ParseFromZeroCopyStream(&file_stream);
                use_stdin = false;
            }
        } break;
        default: return 2;
        }
    }

    if (use_stdin) {
        input_pb.ParseFromZeroCopyStream(&input_stream);
    }

    bool gammaCorrect = false;
    nvtt::Format texFmt = nvtt::Format_BC1;
    nvtt::Quality quality = nvtt::Quality_Fastest;

    for (int i=0, n=input_pb.buildparameters_size(); i < n; ++i) {
        auto& param = input_pb.buildparameters(i);
        if (param.name() == "sRGB") {
            fatal_error_check(param.values_size() && param.values(0).has_boolvalue(), "sRGB is not a boolean value");
            gammaCorrect = param.values(0).boolvalue();
        } else if (param.name() == "format") {
            fatal_error_check(param.values_size() && param.values(0).has_strvalue(), "format is not a boolean value");
            texFmt = getEnumFromName<nvtt::Format>(param.values(0).strvalue().c_str(), textureFormats, nvtt::Format_BC1);
        } else if (param.name() == "quality") {
            fatal_error_check(param.values_size() && param.values(0).has_strvalue(), "quality is not a boolean value");
            quality = getEnumFromName<nvtt::Quality>(param.values(0).strvalue().c_str(), qualityFormats, nvtt::Quality_Fastest);
        }
    }

    if (!writeOutTexture(input_pb.resourceinputpath().c_str(), gammaCorrect, texFmt, quality)) {
        return -2;
    }

    return 0;
}
