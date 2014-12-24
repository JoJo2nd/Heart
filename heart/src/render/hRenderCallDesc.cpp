/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hRenderCallDesc.h"
#include "base/hMemoryUtil.h"

namespace Heart {
namespace hRenderer {

hRenderCallDesc::hRenderCallDesc() {
    clearDescription();
}

void hRenderCallDesc::clearDescription() {
    for (auto& s : samplerStates_) {
        s.name_ = hStringID();
        s.sampler_ = hSamplerStateDesc();
    }
    hZeroMem(vertexLayout_, sizeof(vertexLayout_));
    for (auto& t : textureSlots_) {
        t.name_ = hStringID();
        t.t1D_ = nullptr;
        t.texType_ = 0;
    }
    for (auto& u : uniformBuffers_) {
        u.name_ = hStringID();
    }
    for (auto& v : vertexLayout_) {
        v.bindPoint_ = hStringID();
    }
    blend_ = hBlendStateDesc();
    depthStencil_ = hDepthStencilStateDesc();
    rasterizer_ = hRasterizerStateDesc();
    indexBuffer_ = nullptr;
    vertexBuffer_ = nullptr;
    vertex_ = nullptr;
    fragment_ = nullptr;
}

void hRenderCallDesc::setSampler(hStringID name, const hSamplerStateDesc& ssd) {
    for (auto& s : samplerStates_) {
        if (s.name_==name) {
            s.sampler_=ssd;
            return;
        } else if (s.name_.is_default()) {
            s.name_=name;
            s.sampler_=ssd;
            return;
        }
    }

    hcAssertFailMsg("Too many sampler states");
}

void hRenderCallDesc::setTextureSlot(hStringID name, const hTexture1D* tex) {
    for (auto& t : textureSlots_) {
        if (t.name_==name) {
            t.t1D_=tex;
            t.texType_ = 1;
            return;
        } else if (t.name_.is_default()) {
            t.name_=name;
            t.t1D_=tex;
            t.texType_ = 1;
            return;
        }
    }

    hcAssertFailMsg("Too many textures bound");
}

void hRenderCallDesc::setTextureSlot(hStringID name, const hTexture2D* tex) {
    for (auto& t : textureSlots_) {
        if (t.name_==name) {
            t.t2D_=tex;
            t.texType_ = 2;
            return;
        } else if (t.name_.is_default()) {
            t.name_=name;
            t.t2D_=tex;
            t.texType_ = 2;
            return;
        }
    }

    hcAssertFailMsg("Too many textures bound");
}

void hRenderCallDesc::setTextureSlot(hStringID name, const hTexture3D* tex) {
    for (auto& t : textureSlots_) {
        if (t.name_==name) {
            t.t3D_=tex;
            t.texType_ = 3;
            return;
        } else if (t.name_.is_default()) {
            t.name_=name;
            t.t3D_=tex;
            t.texType_ = 3;
            return;
        }
    }

    hcAssertFailMsg("Too many textures bound");
}

void hRenderCallDesc::setUniformBuffer(hStringID name, const hUniformBuffer* ub) {
    for (auto& t : uniformBuffers_) {
        if (t.name_==name) {
            t.ub_=ub;
            return;
        } else if (t.name_.is_default()) {
            t.name_=name;
            t.ub_=ub;
            return;
        }
    }
}

void hRenderCallDesc::setVertexBufferLayout(hVertexBufferLayout* layout, hUint count) {
    hcAssert(count < vertexLayoutMax_);
    hMemCpy(vertexLayout_, layout, count*sizeof(hVertexBufferLayout));
}

}
}