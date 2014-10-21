/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hTexture.h"
#include "render/hMipDesc.h"
#include "render/hRenderer.h"

namespace Heart {

hRegisterObjectType(TextureResource, Heart::hTextureResource, Heart::proto::TextureResource);

hTextureResource::~hTextureResource() {
    hRenderer::destroyTexture2D(texture2D_);
}

hBool hTextureResource::serialiseObject(Heart::proto::TextureResource* obj) const {
    return hTrue;
}

hBool hTextureResource::deserialiseObject(Heart::proto::TextureResource* obj) {
    // don't support texture arrays just yet...
    arraySize_ = 1;
    width_ = obj->width();
    height_ = obj->height();
    depth_ = obj->depth();
    mipCount_ = obj->mips_size();
    switch(obj->format()) {
    case proto::TextureFormat::RGBA8_unorm: format_ = obj->srgb() ? hTextureFormat::RGBA8_sRGB_unorm: hTextureFormat::RGBA8_unorm; break;
    case proto::TextureFormat::BC3_unorm  : format_ = obj->srgb() ? hTextureFormat::BC3_sRGB_unorm  : hTextureFormat::BC3_unorm  ; break; 
    case proto::TextureFormat::BC2_unorm  : format_ = obj->srgb() ? hTextureFormat::BC2_sRGB_unorm  : hTextureFormat::BC2_unorm  ; break; 
    case proto::TextureFormat::BC1_unorm  : format_ = obj->srgb() ? hTextureFormat::BC1_sRGB_unorm  : hTextureFormat::BC1_unorm  ; break; 
    case proto::TextureFormat::BC3a_unorm : format_ = obj->srgb() ? hTextureFormat::BC3_sRGB_unorm  : hTextureFormat::BC3_unorm  ; break;
    case proto::TextureFormat::BC1a_unorm : format_ = obj->srgb() ? hTextureFormat::BC1_sRGB_unorm  : hTextureFormat::BC1_unorm  ; break;
    case proto::TextureFormat::BC4_unorm  : format_ = hTextureFormat::BC4_unorm  ; break;
    case proto::TextureFormat::BC5_unorm  : format_ = hTextureFormat::BC5_unorm  ; break;
    default: break;
    }

    hRenderer::hMipDesc* mips = (hRenderer::hMipDesc*)hAlloca(sizeof(hRenderer::hMipDesc)*mipCount_);
    for (auto i=0u, n=mipCount_; i<n; ++i) {
        mips[i].width = obj->mips(i).width();
        mips[i].height = obj->mips(i).height();
        mips[i].data = (hByte*)obj->mips(i).data().c_str();
        mips[i].size = (hUint32)obj->mips(i).data().size();
    }

    texture2D_ = hRenderer::createTexture2D(mipCount_, mips, format_, 0);

    return hTrue;
}

hBool hTextureResource::linkObject() {
    return hTrue;
}
}