/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hTextureResource.h"
#include "render/hMipDesc.h"
#include "render/hRenderer.h"

namespace Heart {
namespace hRenderer {
hTextureFormat convertTextureFormat(proto::TextureFormat in_format, bool sRGB) {
    hTextureFormat format_ = hTextureFormat::Unknown;
    switch (in_format) {
    case proto::TextureFormat::RGBA8_unorm: format_ = sRGB ? hTextureFormat::RGBA8_sRGB_unorm : hTextureFormat::RGBA8_unorm; break;
    case proto::TextureFormat::BC3_unorm: format_ = sRGB ? hTextureFormat::BC3_sRGB_unorm : hTextureFormat::BC3_unorm; break;
    case proto::TextureFormat::BC2_unorm: format_ = sRGB ? hTextureFormat::BC2_sRGB_unorm : hTextureFormat::BC2_unorm; break;
    case proto::TextureFormat::BC1_unorm: format_ = sRGB ? hTextureFormat::BC1_sRGB_unorm : hTextureFormat::BC1_unorm; break;
    case proto::TextureFormat::BC3a_unorm: format_ = sRGB ? hTextureFormat::BC3_sRGB_unorm : hTextureFormat::BC3_unorm; break;
    case proto::TextureFormat::BC1a_unorm: format_ = sRGB ? hTextureFormat::BC1_sRGB_unorm : hTextureFormat::BC1_unorm; break;
    case proto::TextureFormat::BC4_unorm: format_ = hTextureFormat::BC4_unorm; break;
    case proto::TextureFormat::BC5_unorm: format_ = hTextureFormat::BC5_unorm; break;
    default: break;
    }
    return format_;
}
proto::TextureFormat convertTextureFormat(hTextureFormat in_format, bool* out_sRGB) {
    proto::TextureFormat format_ = (proto::TextureFormat)proto::TextureFormat_ARRAYSIZE;
    switch (in_format) {
    case hTextureFormat::RGBA8_unorm: *out_sRGB = false; return proto::TextureFormat::RGBA8_unorm;
    case hTextureFormat::BC1_unorm: *out_sRGB = false; return proto::TextureFormat::BC1_unorm;
    case hTextureFormat::BC2_unorm: *out_sRGB = false; return proto::TextureFormat::BC2_unorm;
    case hTextureFormat::BC3_unorm: *out_sRGB = false; return proto::TextureFormat::BC3_unorm;
    case hTextureFormat::BC4_unorm: *out_sRGB = false; return proto::TextureFormat::BC4_unorm;
    case hTextureFormat::BC5_unorm: *out_sRGB = false; return proto::TextureFormat::BC5_unorm;
    case hTextureFormat::RGBA8_sRGB_unorm: *out_sRGB = true; return proto::TextureFormat::RGBA8_unorm;
    case hTextureFormat::BC1_sRGB_unorm: *out_sRGB = true; return proto::TextureFormat::BC1_unorm;
    case hTextureFormat::BC2_sRGB_unorm: *out_sRGB = true; return proto::TextureFormat::BC2_unorm;
    case hTextureFormat::BC3_sRGB_unorm: *out_sRGB = true; return proto::TextureFormat::BC3_unorm;
    default: break;
    }
    return format_;
}
}

hRegisterObjectType(TextureResource, Heart::hTextureResource, Heart::proto::TextureResource);

hTextureResource::hTextureResource(Heart::proto::TextureResource* obj) {
    // don't support texture arrays just yet...
    arraySize_ = 1;
    width_ = obj->width();
    height_ = obj->height();
    depth_ = obj->depth();
    mipCount_ = obj->mips_size();
    format_ = hRenderer::convertTextureFormat(obj->format(), obj->srgb());

    if (obj->has_keepcpudata() && obj->keepcpudata()) {
        hRenderer::hMipDesc* mips = (hRenderer::hMipDesc*)hAlloca(sizeof(hRenderer::hMipDesc)*mipCount_);
        hSize_t texture_data_size = true;
        for (auto i = 0u, n = mipCount_; i < n; ++i) {
            texture_data_size += (hUint32)obj->mips(i).data().size();
        }
        cpuData.reset(new hByte[texture_data_size]);
        cpuMipData.resize(mipCount_);
        auto* dst = cpuData.get();
        for (auto i = 0u, n = mipCount_; i < n; ++i) {
            cpuMipData[i].width = obj->mips(i).width();
            cpuMipData[i].height = obj->mips(i).height();
            cpuMipData[i].data = dst;
            cpuMipData[i].size = (hUint32)obj->mips(i).data().size();
            hMemCpy(dst, obj->mips(i).data().c_str(), obj->mips(i).data().size());
            dst += obj->mips(i).data().size();
        }
    }
    if (!obj->has_dontusevram() || !obj->dontusevram()) {
        hRenderer::hMipDesc* mips = (hRenderer::hMipDesc*)hAlloca(sizeof(hRenderer::hMipDesc)*mipCount_);
        for (auto i = 0u, n = mipCount_; i < n; ++i) {
            mips[i].width = obj->mips(i).width();
            mips[i].height = obj->mips(i).height();
            mips[i].data = (hByte*)obj->mips(i).data().c_str();
            mips[i].size = (hUint32)obj->mips(i).data().size();
        }

        texture2D_ = hRenderer::createTexture2D(mipCount_, mips, format_, 0);
    }
}

hTextureResource::~hTextureResource() {
    hRenderer::destroyTexture2D(texture2D_);
}

hBool hTextureResource::serialiseObject(Heart::proto::TextureResource* obj) const {
    return hTrue;
}

hBool hTextureResource::linkObject() {
    return hTrue;
}

hRegisterObjectType(TextureAtlas, Heart::hTextureAtlasResource, Heart::proto::TextureAtlas);

hTextureAtlasResource::hTextureAtlasResource(Heart::proto::TextureAtlas* obj) 
    : textureResource(obj->mutable_texture()) {
    atlasImages.reserve(obj->images_size());
    for (hUint32 i=0, n=obj->images_size(); i<n; ++i) {
        atlasImages.push_back(obj->images(i));
    }
}

hBool hTextureAtlasResource::serialiseObject(Heart::proto::TextureAtlas* obj) const {
    return hTrue;
}

hBool hTextureAtlasResource::linkObject() {
    return hTrue;
}

}
