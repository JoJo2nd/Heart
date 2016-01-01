/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "components/hObjectFactory.h"
#include "render/hTextureFormat.h"
#include "render/hMipDesc.h"
#include "render/hRendererTypeHelpers.h"
#include <vector>
#include <memory>
#include "resource_texture.pb.h"

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
        std::vector<hRenderer::hMipDesc> cpuMipData;
        std::unique_ptr<hByte> cpuData;
        union {
            hRenderer::hTexture1D* texture1D_;
            hRenderer::hTexture2D* texture2D_;
            hRenderer::hTexture3D* texture3D_;
        };
        hRenderer::hRenderTargetUniquePtr renderTarget;

    public:
        hObjectType(hTextureResource, Heart::proto::TextureResource);
        ~hTextureResource();

        hUint32 getWidth() const { return width_; }
        hUint32 getHeight() const { return height_; }
        hUint32 getDepth() const { return depth_; }
        hTextureFormat getTextureFormat() const { return format_; }
        hUint getMipCount() const { return mipCount_; }
        const hRenderer::hMipDesc* getCPUMipDesc(hUint i) const { return &cpuMipData[i]; }
        hUint getArraySize() const { return arraySize_; }
        hRenderer::hTexture2D* getTexture2D() const { return texture2D_; }
        hBool isRenderTarget() const { return !!renderTarget; }
        hRenderer::hRenderTarget* getRenderTarget() const { return renderTarget.get(); }
    };

    typedef Heart::proto::AtlasImage hAtlasImage;

    class hTextureAtlasResource {
        hTextureResource textureResource;
        std::vector<hAtlasImage> atlasImages;
    public:
        hObjectType(hTextureAtlasResource, Heart::proto::TextureAtlas);

        enum { InvalidTileHandle = ~0U };

        hTextureResource* getTextureResource() { return &textureResource; }
        hAtlasImage getAtlasImage(hUint32 id) {
            for (const auto& i : atlasImages) {
                if (i.atlasid() == id) return i;
            }
            return hAtlasImage();
        }
    };

namespace hRenderer {
    hTextureFormat convertTextureFormat(proto::TextureFormat in_format, bool sRGB);
    proto::TextureFormat convertTextureFormat(hTextureFormat in_format, bool* out_sRGB);
}
}
