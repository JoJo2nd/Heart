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
#include "texture_atlas.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_iterators.hpp"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "minfs.h"
#include <memory>
#ifdef _WIN32
#   include <io.h>
#   include <fcntl.h>
#endif
#include <thread>
#include <mutex>
#include <algorithm>

enum class LongOptions : int {
    CubeTexture = 0x01000,
    AtlasTexture = 0x02000,
    PrintVersion = 0x04000,
    Multithreaded = 0x08000,
    PremultipliedAlpha = 0x10000,
    RenderTarget = 0x20000,
};

static const char argopts[] = "vi:";
static struct option long_options[] = {
    { "version", no_argument, 0, (int)LongOptions::PrintVersion },
    { "cube", no_argument, 0, (int)LongOptions::CubeTexture },
    { "atlas", no_argument, 0, (int)LongOptions::AtlasTexture },
	{ "multithreaded", no_argument, 0, (int)LongOptions::Multithreaded },
    { "premultipliedalpha", no_argument, 0, (int)LongOptions::PremultipliedAlpha },
    { "rendertarget", no_argument, 0, (int)LongOptions::RenderTarget },
    { 0, 0, 0, 0 }
};

struct TextureParams {
    bool noVRAM = false;
    bool keepCPU = false;
    int width = 0;
    int height = 0;
    int atlasPaddingX = 0;
    int atlasPaddingY = 0;
};

#define fatal_error(msg, ...) {fprintf(stderr, msg, __VA_ARGS__); exit(-1);}
#define fatal_error_check(x, msg, ...) if (!(x)) {fprintf(stderr, msg, __VA_ARGS__); exit(-1);}
#define ensure_condition(x, msg, ...) fatal_error_check(x, msg, __VA_ARGS__)

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

struct TextureData {
    int32_t width = 0;
    int32_t height = 0;
    bool hasAlpha = false;
    std::unique_ptr<uint8_t> rgba;

    const uint8_t* getPixelClamped(int32_t x, int32_t y) const {
        x = std::max(x, 0);
        x = std::min(x, width-1);
        y = std::max(y, 0);
        y = std::min(y, height-1);
        return rgba.get()+(y*width*4)+(x*4);
    }
};

static bool loadTextureDataAsRGBA(const char* input_path, TextureData* out_tdata) {
    FreeImageIO fiIO;
    fiIO.read_proc = &FreeImageFileIO::read_proc;
    fiIO.seek_proc = &FreeImageFileIO::seek_proc;
    fiIO.tell_proc = &FreeImageFileIO::tell_proc;
    fiIO.write_proc = &FreeImageFileIO::write_proc;
    FILE* file = fopen(input_path, "rb");
    if (!file) return false;
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromHandle(&fiIO, (fi_handle)file, 0);
    if (fif != FIF_UNKNOWN) {
        // load from the file handle
        FIBITMAP* dib = FreeImage_LoadFromHandle(fif, &fiIO, (fi_handle)file, 0);
        unsigned int width = FreeImage_GetWidth(dib);
        unsigned int height = FreeImage_GetHeight(dib);
        unsigned int bpp = FreeImage_GetBPP(dib);
        size_t bytes = width*height * 4;
        unsigned char* data = new unsigned char[bytes];

        out_tdata->width = width;
        out_tdata->height = height;
        out_tdata->hasAlpha = false;
        out_tdata->rgba.reset(data);

        //Free image stores upside down so re-arrange
        switch (bpp) {
        case 32: {
            out_tdata->hasAlpha = true;
            for (size_t y = height - 1, d = 0; y < height; --y) {
                unsigned char* scanline = FreeImage_GetScanLine(dib, (int)y);
                for (size_t x = 0, slw = (width * 4); x < slw; x += 4) {
                    data[d++] = scanline[x + 0];
                    data[d++] = scanline[x + 1];
                    data[d++] = scanline[x + 2];
                    data[d++] = scanline[x + 3];
                }
            }
            break;
        }
        case 24: {
            for (size_t y = height - 1, d = 0; y < height; --y) {
                unsigned char* scanline = FreeImage_GetScanLine(dib, (int)y);
                for (size_t x = 0, slw = (width * 3); x < slw; x += 3) {
                    data[d++] = scanline[x + 0];
                    data[d++] = scanline[x + 1];
                    data[d++] = scanline[x + 2];
                    data[d++] = 0xFF;
                }
            }
            break;
        }
        case 8: {
            for (size_t y = height - 1, d = 0; y < height; --y) {
                unsigned char* scanline = FreeImage_GetScanLine(dib, (int)y);
                for (size_t x = 0; x < width; ++x) {
                    for (size_t i = 0; i < 4; ++i) {
                        data[d++] = scanline[x];
                    }
                }
            }
            break;
        }
        default:
            fclose(file);
            fprintf(stderr, "Couldn't read pixel format with bpp of %d", bpp);
            return false;
        }
    }
    fclose(file);
    return true;
}

struct TextureAtlasMemData {
    uint32_t texture_width;
    uint32_t texture_height;
    int32_t padding_x = 1;
    int32_t padding_y = 1;
    struct AtlasImage {
        uint32_t id;
        TextureData tdata;
        std::string texturePath;
        std::string fullTexturePath;
        // 0-1 locations
        float u1, v1 ,u2, v2;
        // Pixel locations
        uint32_t x1,y1,x2,y2;
    };
    std::vector<AtlasImage> imageLocations;
    std::vector<char> data;
    utils::TextureAtlasBuilder builder;

    bool sortAndBuild() {
        padding_x = std::max(padding_x, 0);
        padding_y = std::max(padding_y, 0);
        std::stable_sort(imageLocations.begin(), imageLocations.end(), [](const AtlasImage& lhs, const AtlasImage& rhs) {
            return (lhs.tdata.width*lhs.tdata.height) < (rhs.tdata.width*rhs.tdata.height);
        });
        builder.initialise(texture_width, texture_height, 4, padding_x, padding_y);
        for (auto& i : imageLocations) {
            auto* node = builder.insert((padding_x*2)+i.tdata.width, (padding_y*2)+i.tdata.height, nullptr);
            i.u1 = (float)node->x/(float)texture_width;
            i.v1 = (float)node->y / (float)texture_height;
            i.u2 = (float)(node->x+node->w) / (float)texture_width;
            i.v2 = (float)(node->y+node->h) / (float)texture_height;
            i.x1 = node->x;
            i.y1 = node->y;
            i.x2 = node->x + node->w;
            i.y2 = node->y + node->h;
            // Not the fastest in the world but go for correct first.
            for (int32_t y=-padding_y, sy=0, yn=i.tdata.height+padding_y; y<yn; ++y, ++sy) {
                for (int32_t x=-padding_x, sx=0, xn=i.tdata.width+padding_x; x<xn; ++x, ++sx) {
                    memcpy(builder.getTextureLocationPtrMutable(node->x+sx, node->y+sy), i.tdata.getPixelClamped(x, y), 4);
                }
            }
        }
        return true;
    }
};

static bool loadTextureAtlas(const char* atlas_data, size_t atlas_data_size, const char* root_path, uint32_t t_width, uint32_t t_height, TextureAtlasMemData* atlas) {
    atlas->texture_width = t_width;
    atlas->texture_height = t_height;
    atlas->data.resize(atlas_data_size+1, 0);
    memcpy(atlas->data.data(), atlas_data, atlas_data_size);

    rapidxml::xml_document<> xml;
    
    xml.parse<0>(atlas->data.data());

    rapidxml::node_iterator<char> n;
    rapidxml::attribute_iterator<char> an;
    auto* root = xml.first_node("tileset");
    if (!root) return false;
    for (rapidxml::node_iterator<char> i(root); i != n; ++i) {
        if (strcmp(i->name(), "tile") == 0) {
            TextureAtlasMemData::AtlasImage img;
            for (rapidxml::attribute_iterator<char> att(i.get()); att != an; ++att) {
                if (strcmp(att->name(), "id") == 0) {
                    img.id = atoi(att->value());
                }
            }
            for (rapidxml::node_iterator<char> tile(i.get()); tile != n; ++tile) {
                if (strcmp(tile->name(), "image") == 0) {
                    for (rapidxml::attribute_iterator<char> att(tile.get()); att != an; ++att) {
                        if (strcmp(att->name(), "width") == 0) {
                        } else if (strcmp(att->name(), "height") == 0) {
                        } else if (strcmp(att->name(), "source") == 0) {
                            img.texturePath = att->value();
                        }
                    }
                }
            }
            atlas->imageLocations.emplace_back(std::move(img));
        }
    }

    for (auto& i : atlas->imageLocations) {
        char full_path[260];
        minfs_path_join(root_path, i.texturePath.c_str(), full_path, sizeof(full_path));
        i.fullTexturePath = full_path;
        if (!loadTextureDataAsRGBA(full_path, &i.tdata)) {
            fatal_error("Failed to load texture '%s' for atlas", full_path);
        }
    }
    return true;
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
static bool createCompressedTexture(const void* in_texture_data, uint32_t t_width, uint32_t t_height, bool gammacorrect, nvtt::Format format, nvtt::Quality quality, bool multithreaded, bool premultiplied_alpha, const TextureParams& params, Heart::proto::TextureResource* textureRes) {
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

    unsigned int width=t_width;
    unsigned int height=t_height;
    size_t bytes=width*height*4;
    const unsigned char* data=(const unsigned char*)in_texture_data;

    textureRes->set_width(width);
    textureRes->set_height(height);
    textureRes->set_srgb(gammacorrect);
    textureRes->set_depth(1);
    textureRes->set_dontusevram(params.noVRAM);
    textureRes->set_keepcpudata(params.keepCPU);
    switch(format) {
    case nvtt::Format_RGB: textureRes->set_format(Heart::proto::RGBA8_unorm); break;
    case nvtt::Format_BC1: textureRes->set_format(Heart::proto::BC1_unorm); break;
    case nvtt::Format_BC1a: textureRes->set_format(Heart::proto::BC1a_unorm); break;
    case nvtt::Format_BC2: textureRes->set_format(Heart::proto::BC2_unorm); break;
    case nvtt::Format_BC3: textureRes->set_format(Heart::proto::BC3_unorm); break;
    case nvtt::Format_BC3n: textureRes->set_format(Heart::proto::BC3_unorm); break;
    case nvtt::Format_BC4: textureRes->set_format(Heart::proto::BC4_unorm); break;
    case nvtt::Format_BC5: textureRes->set_format(Heart::proto::BC5_unorm); break;
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
            // Don't do premultipled alpha yet, wait until final part
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
            PixelWorkBlock(int fw, int fh, int w, int h, int dx, int dy, size_t ss, size_t ds, void* s, void* d)
                : width(w), height(h), destX(dx), destY(dy), srcSize(ss), destSize(ds), dest(d)
            {
                assert(ss <= 16*sizeof(uint32_t));
                auto* p = (uint32_t*)s;
                for (int y=0; y<4; ++y) {
                    for (int x=0; x<4; ++x) {
                        if (dy+y < fh && dx+w < fw) {
                            src[(y*4)+x] = p[(y*fw)+x];
                        }
                    }
                }
            }
            int width;
            int height;
            int destX;
            int destY;
            size_t srcSize;
            size_t destSize;
            uint32_t src[16]; // for 4x4 block
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
                        m.width, m.height,
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
                    if (premultiplied_alpha) inputOptions.setAlphaMode(nvtt::AlphaMode_Premultiplied);
                    else inputOptions.setAlphaMode(nvtt::AlphaMode_Transparency);
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
            auto* tex_mip = textureRes->add_mips();
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
            if (premultiplied_alpha) inputOptions.setAlphaMode(nvtt::AlphaMode_Premultiplied);
            else inputOptions.setAlphaMode(nvtt::AlphaMode_Transparency);
		} else {
			inputOptions.setAlphaMode(nvtt::AlphaMode_None);
		}
		inputOptions.setMipmapData(data, width, height);

		TextureWriter texWriter(textureRes);
		nvtt::OutputOptions outputOptions;
		outputOptions.setOutputHandler(&texWriter);

		nvtt::CompressionOptions compresserOptions;
		compresserOptions.setFormat(format);
		compresserOptions.setQuality(quality);

		nvtt::Compressor compressor;
		compressor.process(inputOptions, compresserOptions, outputOptions);
	}

    return true;
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
    bool verbose = false, use_stdin = true, generate_atlas = false, generate_cube = false, multithreaded = false, premultiplied_alpha = false;
    bool generate_rendertarget = false;

    while ((c = gop_getopt_long(argc, argv, argopts, long_options, &option_index)) != -1) {
        switch (c) {
        case 'z': case LongOptions::PrintVersion: fprintf(stdout, "heart texture builder v0.8.0"); exit(0);
        case LongOptions::AtlasTexture: generate_atlas = true; break;
        case LongOptions::CubeTexture: generate_cube = true; break;
		case LongOptions::Multithreaded: multithreaded = true; break;
        case LongOptions::PremultipliedAlpha: premultiplied_alpha = true; break;
        case LongOptions::RenderTarget: generate_rendertarget = true; break;
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
    if (generate_cube) {
        return 2;
    }

    if (use_stdin) {
        input_pb.ParseFromZeroCopyStream(&input_stream);
    }

    bool gammaCorrect = false;
    nvtt::Format texFmt = nvtt::Format_BC1;
    nvtt::Quality quality = nvtt::Quality_Fastest;
    TextureParams params;

    for (int i=0, n=input_pb.buildparameters_size(); i < n; ++i) {
        auto& param = input_pb.buildparameters(i);
        if (param.name() == "sRGB") {
            fatal_error_check(param.values_size() && param.values(0).has_boolvalue(), "sRGB is not a boolean value");
            gammaCorrect = param.values(0).boolvalue();
        } else if (param.name() == "format") {
            fatal_error_check(param.values_size() && param.values(0).has_strvalue(), "format is not a string value");
            texFmt = getEnumFromName<nvtt::Format>(param.values(0).strvalue().c_str(), textureFormats, nvtt::Format_BC1);
        } else if (param.name() == "quality") {
            fatal_error_check(param.values_size() && param.values(0).has_strvalue(), "quality is not a string value");
            quality = getEnumFromName<nvtt::Quality>(param.values(0).strvalue().c_str(), qualityFormats, nvtt::Quality_Fastest);
        } else if (param.name() == "novram") {
            fatal_error_check(param.values_size() && param.values(0).has_boolvalue(), "novram is not a boolean value");
            params.noVRAM = param.values(0).boolvalue();
        } else if (param.name() == "keepcpudata") {
            fatal_error_check(param.values_size() && param.values(0).has_boolvalue(), "keepcpudata is not a boolean value");
            params.keepCPU = param.values(0).boolvalue();
        } else if (param.name() == "atlas_width") {
            fatal_error_check(param.values_size() && param.values(0).has_intvalue(), "atlas_width is not a int value");
            params.width = param.values(0).intvalue();
        } else if (param.name() == "atlas_height") {
            fatal_error_check(param.values_size() && param.values(0).has_intvalue(), "atlas_height is not a int value");
            params.height = param.values(0).intvalue();
        } else if (param.name() == "atlas_padding_x") {
            fatal_error_check(param.values_size() && param.values(0).has_intvalue(), "atlas_padding_x is not a int value");
            params.atlasPaddingX = param.values(0).intvalue();
        } else if (param.name() == "atlas_padding_y") {
            fatal_error_check(param.values_size() && param.values(0).has_intvalue(), "atlas_padding_y is not a int value");
            params.atlasPaddingY = param.values(0).intvalue();
        }
    }

    Heart::builder::Output output;

    if (generate_atlas) {
        fatal_error_check(params.atlasPaddingX < params.width, "atlas X padding (%d) is too large (greater than %d)", params.atlasPaddingX, params.width);
        fatal_error_check(params.atlasPaddingY < params.height, "atlas Y padding (%d) is too large (greater than %d)", params.atlasPaddingY, params.height);
        TextureAtlasMemData texture_atlas;
        Heart::proto::TextureAtlas textureAtlas;
        char root_path[260];
        minfs_path_parent(input_pb.resourceinputpath().c_str(), root_path, sizeof(root_path));
        size_t file_size = minfs_get_file_size(input_pb.resourceinputpath().c_str());
        FILE* f = fopen(input_pb.resourceinputpath().c_str(), "rb");
        fatal_error_check(f, "Unable to open file %s", input_pb.resourceinputpath().c_str());
        std::unique_ptr<char> xml_data(new char[file_size]);
        fread(xml_data.get(), 1, file_size, f);
        fclose(f);
        f = nullptr;
        loadTextureAtlas(xml_data.get(), file_size, root_path, params.width, params.height, &texture_atlas);
        texture_atlas.sortAndBuild();
        bool compressed = createCompressedTexture(texture_atlas.builder.getTextureDataPtr(), texture_atlas.texture_width, texture_atlas.texture_height, gammaCorrect, texFmt, quality, multithreaded, premultiplied_alpha, params, textureAtlas.mutable_texture());
        fatal_error_check(compressed, "Failed to compress texture");
        for (const auto& i : texture_atlas.imageLocations) {
            output.add_filedependency(i.fullTexturePath);
            auto* img = textureAtlas.add_images();
            img->set_atlasid(i.id);
            img->set_u1(i.u1);
            img->set_v1(i.v1);
            img->set_u2(i.u2);
            img->set_v2(i.v2);
            img->set_x1(i.x1);
            img->set_y1(i.y1);
            img->set_x2(i.x2);
            img->set_y2(i.y2);
        }

        output.mutable_pkgdata()->set_type_name(textureAtlas.GetTypeName());
        output.mutable_pkgdata()->set_messagedata(textureAtlas.SerializeAsString());
        google::protobuf::io::OstreamOutputStream filestream(&std::cout);
        return output.SerializeToZeroCopyStream(&filestream) ? 0 : -2;
    } else if (generate_rendertarget) {
        Heart::proto::TextureResource textureRes;
        rapidjson::Document rt_doc;
        std::vector<char> json_data;
        size_t fsize = minfs_get_file_size(input_pb.resourceinputpath().c_str());
        json_data.resize(fsize+1, 0);
        FILE* f = fopen(input_pb.resourceinputpath().c_str(), "rb");
        fread(json_data.data(), 1, fsize, f);
        fclose(f);
        rt_doc.ParseInsitu(json_data.data());
        ensure_condition(!rt_doc.HasParseError(), "Failed to parse JSON file %s. Error '%s' at '%zu'", input_pb.resourceinputpath().c_str(), rapidjson::GetParseError_En(rt_doc.GetParseError()), rt_doc.GetErrorOffset());
        const auto& rt = rt_doc["rendertarget"];
        ensure_condition(rt.HasMember("format"), "Render target is missing format");
        Heart::proto::TextureFormat format;
        ensure_condition(Heart::proto::TextureFormat_Parse(rt["format"].GetString(), &format), "Unable to parse texture format '%s'", rt["format"].GetString());
        textureRes.set_format(format);
        textureRes.set_useasrendertarget(true);
        textureRes.set_srgb(rt.HasMember("srgb") && rt["srgb"].GetBool());
        Heart::proto::RenderTargetInfo* rt_info_pb = textureRes.mutable_targetinfo();
        if (rt.HasMember("aspect")) {
            ensure_condition((rt.HasMember("percentagewidth") || rt.HasMember("percentageheight") || rt.HasMember("fixedwidth") || rt.HasMember("fixedheight")), "If aspect is given, at least one width or height parameter must be given");
            rt_info_pb->set_aspect((float)rt["aspect"].GetDouble());
        }
        if (rt.HasMember("percentagewidth")) {
            ensure_condition(rt["percentagewidth"].IsDouble(), "percentagewidth parameter is not a number");
            rt_info_pb->set_percentagewidth((float)rt["percentagewidth"].GetDouble());
        }
        if (rt.HasMember("percentageheight")) {
            ensure_condition(rt["percentageheight"].IsDouble(), "percentageheight parameter is not a number");
            rt_info_pb->set_percentageheight((float)rt["percentageheight"].GetDouble());
        }
        if (rt.HasMember("fixedwidth")) {
            ensure_condition(rt["fixedwidth"].IsUint(), "fixedwidth parameter is not an unsigned int");
            rt_info_pb->set_fixedwidth(rt["fixedwidth"].GetUint());
        }
        if (rt.HasMember("fixedheight")) {
            ensure_condition(rt["fixedheight"].IsUint(), "fixedheight parameter is not an unsigned int");
            rt_info_pb->set_fixedheight(rt["fixedheight"].GetUint());
        }

        output.mutable_pkgdata()->set_type_name(textureRes.GetTypeName());
        output.mutable_pkgdata()->set_messagedata(textureRes.SerializeAsString());
        google::protobuf::io::OstreamOutputStream filestream(&std::cout);
        return output.SerializeToZeroCopyStream(&filestream) ? 0 : -2;
    } else {
        TextureData tdata;
        Heart::proto::TextureResource textureRes;
        output.add_filedependency(input_pb.resourceinputpath());
        bool loaded = loadTextureDataAsRGBA(input_pb.resourceinputpath().c_str(), &tdata);
        fatal_error_check(loaded, "Failed to load texture %s", input_pb.resourceinputpath().c_str());
        bool compressed = createCompressedTexture(tdata.rgba.get(), tdata.width, tdata.height, gammaCorrect, texFmt, quality, multithreaded, premultiplied_alpha, params, &textureRes);
        fatal_error_check(compressed, "Failed to compress texture");

        output.mutable_pkgdata()->set_type_name(textureRes.GetTypeName());
        output.mutable_pkgdata()->set_messagedata(textureRes.SerializeAsString());
        google::protobuf::io::OstreamOutputStream filestream(&std::cout);
        return output.SerializeToZeroCopyStream(&filestream) ? 0 : -2;
    }
    return -1;
}
