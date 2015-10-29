/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "core/hProtobuf.h"
#include "components/hObjectFactory.h"
#include "render/hTextureFormat.h"

namespace Heart {
namespace hRenderer {
    struct hTexture1D;
    struct hTexture2D;
    struct hTexture3D;
}

    class  hTextureResource {
        hUint           arraySize_;
        hUint           mipCount_;
        hUint           width_;
        hUint           height_;
        hUint           depth_;
        hTextureFormat  format_;
        union {
            hRenderer::hTexture1D* texture1D_;
            hRenderer::hTexture2D* texture2D_;
            hRenderer::hTexture3D* texture3D_;
        };

    public:
        hObjectType(hTextureResource, Heart::proto::TextureResource);

        hTextureResource() 
            : arraySize_(0)
            , mipCount_(0)
            , width_(0)
            , height_(0)
            , depth_(0)
            , format_(hTextureFormat::Unknown) {
            texture1D_ = nullptr;
        }
        ~hTextureResource();

        hUint32                 getWidth() const { return width_; }
        hUint32                 getHeight() const { return height_; }
        hUint32                 getDepth() const { return depth_; }
        hTextureFormat          getTextureFormat() const { return format_; }
        hUint                   getMipCount() const { return mipCount_; }
        hUint                   getArraySize() const { return arraySize_; }
        hRenderer::hTexture2D*  getTexture2D() const { return texture2D_; }
    };

}
