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
#include <assert.h>

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
#include <thread>
#include <mutex>
#include <algorithm>

enum class LongOptions : int {
    CubeTexture   = 0x1000,
    AtlasTexture  = 0x2000,
    PrintVersion  = 0x4000,
	Multithreaded = 0x8000,
};

static const char argopts[] = "vi:";
static struct option long_options[] = {
    { "version", no_argument, 0, (int)LongOptions::PrintVersion },
    { "cube", no_argument, 0, (int)LongOptions::CubeTexture },
    { "atlas", no_argument, 0, (int)LongOptions::AtlasTexture },
	{ "multithreaded", no_argument, 0, (int)LongOptions::Multithreaded },
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
size_t getBCPitch(bool dxt1, size_t width) {
    size_t bytecount = std::max( 1ull, ((width+3)/4) );
    bytecount *= (dxt1) ? 8 : 16;
    return bytecount;
}
size_t getBCTextureSize(bool dxt1, size_t width, size_t height) {
    // compute the storage requirements
    size_t blockcount = std::max( 1ull, ( width + 3 )/4 ) * std::max( 1ull, ( height + 3 )/4 );
    size_t blocksize = (dxt1) ? 8 : 16;
    return blockcount*blocksize;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static bool writeOutTexture(const char* input_path, bool gammacorrect, nvtt::Format format, nvtt::Quality quality, bool multithreaded) {
    Heart::builder::Output output;
    output.add_filedependency(input_path);
    FreeImageIO fiIO;
    fiIO.read_proc=&FreeImageFileIO::read_proc;
    fiIO.seek_proc=&FreeImageFileIO::seek_proc;
    fiIO.tell_proc=&FreeImageFileIO::tell_proc;
    fiIO.write_proc=&FreeImageFileIO::write_proc;

    struct TextureWriter : public nvtt::OutputHandler {
        TextureWriter(Heart::proto::TextureResource* resource) 
            : resource_(resource)
            , activeMip_(nullptr)
            , size_(0)
            , reserve_(0)
        {}
        /// Indicate the start of a new compressed image that's part of the final texture.
        virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) override {
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
        virtual bool writeData(const void * data, int size) override {
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
        // Indicate the end of the compressed image. (New in NVTT 2.1)
        virtual void endImage() override {}

        Heart::proto::TextureResource*      resource_;
        Heart::proto::TextureMip*           activeMip_;
        std::unique_ptr<unsigned char>      data_;
        size_t                              size_;
        size_t                              reserve_;
    };

	struct MipHandler : public nvtt::OutputHandler {
		struct MipLevel {
			MipLevel(size_t in_datasize, size_t in_miplevel, int in_width, int in_height)
				: data(new unsigned char[in_datasize]), dataSize(in_datasize), miplevel(in_miplevel), width(in_width), height(in_height)
			{}
			MipLevel(MipLevel&& rhs) {
				data = std::move(rhs.data);
				dataSize = rhs.dataSize;
				miplevel = rhs.miplevel;
				width = rhs.width;
				height = rhs.height;
			}
			std::unique_ptr<unsigned char[]> data;
			size_t dataSize;
			size_t miplevel;
			int width;
			int height;
		};
		MipHandler() 
			: activeMip(nullptr)
			, written(0) {

		}
		/// Indicate the start of a new compressed image that's part of the final texture.
		virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) override {
			mipLevels.emplace_back(size, miplevel, width, height);
			activeMip = &mipLevels[mipLevels.size()-1];
			written = 0;
		}
		/// Output data. Compressed data is output as soon as it's generated to minimize memory allocations.
		virtual bool writeData(const void * data, int size) override {
            if (!activeMip) {
                return true;
            }
			assert(activeMip && written+size <= activeMip->dataSize);
			memcpy(activeMip->data.get() + written, data, size);
			written += size;

			return true;
		}
        // Indicate the end of the compressed image. (New in NVTT 2.1)
        virtual void endImage() override {}

		std::vector<MipLevel> mipLevels;
		MipLevel* activeMip;
		size_t written;
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

			if (multithreaded && format != nvtt::Format_RGB && format != nvtt::Format_RGBA) {
				// Generate mip maps;
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
				if (format == nvtt::Format_BC1a || format == nvtt::Format_BC5 || format == nvtt::Format_DXT1a || format == nvtt::Format_DXT5) {
					inputOptions.setAlphaMode(nvtt::AlphaMode_Transparency);
				} else {
					inputOptions.setAlphaMode(nvtt::AlphaMode_None);
				}
				inputOptions.setMipmapData(data, width, height);

				MipHandler mipHandler;
				nvtt::OutputOptions outputOptions;
				outputOptions.setOutputHandler(&mipHandler);

				nvtt::CompressionOptions compresserOptions;
				compresserOptions.setFormat(nvtt::Format_RGBA);
				compresserOptions.setQuality(quality);

				nvtt::Compressor compressor;
				compressor.process(inputOptions, compresserOptions, outputOptions);

				// Compress mip maps;
				inputOptions.setMipmapGeneration(false);
				inputOptions.setGamma(1.f, 1.f);

                //Create output locations
                struct MipOutputData {
                    MipOutputData() {}
                    MipOutputData(MipOutputData&& rhs) {
                        data = std::move(rhs.data);
                        width = rhs.width;
                        height = rhs.height;
                        mipSize = rhs.mipSize;
                        miplevel = rhs.miplevel;
                    }
                    int width;
                    int height;
                    size_t miplevel;
                    size_t mipSize;
                    std::unique_ptr<unsigned char[]> data;
                };
                struct PixelWorkBlock {
                    PixelWorkBlock() {}
                    PixelWorkBlock(int w, int h, int dx, int dy, size_t ss, size_t ds, void* s, void* d)
                        : width(w), height(h), destX(dx), destY(dy), srcSize(ss), destSize(ds), src(s), dest(d)
                    {}
                    int width;
                    int height;
                    int destX;
                    int destY;
                    size_t srcSize;
                    size_t destSize;
                    void* src;
                    void* dest;
                };
                std::vector<MipOutputData> compressedMips;
                std::vector<PixelWorkBlock> jobs;
                int BlockSize = 4; // work in 4x4 pixel blocks.
                for (const auto& m : mipHandler.mipLevels) {
                    bool bc1size = format == nvtt::Format_BC1 || format == nvtt::Format_BC1a;
                    size_t block_size = bc1size ? 8 : 16;
                    MipOutputData mip;
                    mip.miplevel = m.miplevel;
                    mip.width = m.width;
                    mip.height = m.height;
                    size_t bc_size = getBCTextureSize(bc1size, mip.width, mip.height);
                    mip.mipSize = bc_size;
                    mip.data.reset(new unsigned char[bc_size]);
                    //Create jobs
                    for (int y=0; y < m.height; y+=BlockSize) {
                        auto dest_y_offset = getBCPitch(bc1size, mip.width)*(y/4);
                        for (int x=0; x < m.width; x+=BlockSize) {
                            auto x_size = BlockSize < (m.width-x)? BlockSize : (m.width-x);
                            auto y_size = BlockSize < (m.height-y)? BlockSize : (m.height-y);
                            auto dest_offset = ((x/4)*block_size)+dest_y_offset;
                            assert(dest_offset+block_size <= bc_size);
                            jobs.emplace_back(PixelWorkBlock(
                                x_size, y_size, x, y,
                                x_size*y_size*4, getBCTextureSize(bc1size, x_size, y_size), 
                                m.data.get()+((y*m.width+x)*4), mip.data.get()+dest_offset
                            ));
                        }
                    }
                    compressedMips.push_back(std::move(mip));
                }

                //Create jobs and spawn worker threads
                std::mutex mtx;
                std::vector<std::thread> workers;
                for (auto i = 0; i < 8; ++i) {
                    workers.push_back(std::thread([&](){
                        struct BCMipHandler : public nvtt::OutputHandler {
                            BCMipHandler(PixelWorkBlock* in_job) 
                                : job(in_job), doneHeader(false), written(0) {}
                            virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) override {
                                doneHeader = true;
                            }
                            virtual bool writeData(const void * data, int size) override {
                                if (!doneHeader) {
                                    return true;
                                }
                                assert(written + size <= job->destSize);
                                memcpy(job->dest, data, size);
                                written += size;
                                return true;
                            }
                            // Indicate the end of the compressed image. (New in NVTT 2.1)
                            virtual void endImage() override {}

                            PixelWorkBlock* job;
                            size_t written;
                            bool doneHeader;
                        };

                        nvtt::OutputOptions outputOptions;
                        nvtt::CompressionOptions compresserOptions;
                        nvtt::Compressor compressor;
                        nvtt::InputOptions bc_input_options;

                        bc_input_options.setMipmapFilter(nvtt::MipmapFilter_Kaiser);
                        bc_input_options.setKaiserParameters(3.0f, 4.0f, 1.0f);
                        bc_input_options.setMipmapGeneration(false);
                        bc_input_options.setGamma(1.f, 1.f);
                        if (format == nvtt::Format_BC1a || format == nvtt::Format_BC5 || format == nvtt::Format_DXT1a || format == nvtt::Format_DXT5) {
                            bc_input_options.setAlphaMode(nvtt::AlphaMode_Transparency);
                        }
                        else {
                            bc_input_options.setAlphaMode(nvtt::AlphaMode_None);
                        }

                        compresserOptions.setFormat(format);
                        compresserOptions.setQuality(quality);

                        for (;;){
                            PixelWorkBlock block;
                            {
                                std::unique_lock<std::mutex> lck(mtx);
                                if (!jobs.size()) {
                                    return;
                                }
                                block = jobs.back();
                                jobs.pop_back();
                            }
                            BCMipHandler output_handler(&block);
                            bc_input_options.setTextureLayout(nvtt::TextureType_2D, block.width, block.height);
                            bc_input_options.setMipmapData(block.src, block.width, block.height);
                            outputOptions.setOutputHandler(&output_handler);
                            compressor.process(bc_input_options, compresserOptions, outputOptions);
                        }

                    }));
                }
                for (auto& i : workers) {
                    i.join();
                }

                for (const auto& m : compressedMips) {
                    auto* tex_mip = textureRes.add_mips();
                    tex_mip->set_width(m.width);
                    tex_mip->set_height(m.height);
                    tex_mip->set_data(m.data.get(), m.mipSize);
                }
			} else {
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
			}

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
    bool verbose = false, use_stdin = true, generate_atlas = false, generate_cube = false, multithreaded = false;

    while ((c = gop_getopt_long(argc, argv, argopts, long_options, &option_index)) != -1) {
        switch (c) {
        case 'z': case LongOptions::PrintVersion: fprintf(stdout, "heart texture builder v0.8.0"); exit(0);
        case LongOptions::AtlasTexture: generate_atlas = true; break;
        case LongOptions::CubeTexture: generate_cube = true; break;
		case LongOptions::Multithreaded: multithreaded = true; break;
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
    //TODO:!
    if (generate_atlas || generate_cube) {
        return 2;
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

    if (!writeOutTexture(input_pb.resourceinputpath().c_str(), gammaCorrect, texFmt, quality, multithreaded)) {
        return -2;
    }

    return 0;
}
