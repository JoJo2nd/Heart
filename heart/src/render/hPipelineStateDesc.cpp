/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hPipelineStateDesc.h"
#include "base/hMemoryUtil.h"
#include "base/hStringUtil.h"
#include "render/hRenderer.h"
#include "render/hProgramReflectionInfo.h"

namespace Heart {
namespace hRenderer {

hPipelineStateDescBase::hPipelineStateDescBase() {
    clearDescription();
}

void hPipelineStateDescBase::clearDescription() {
    for (auto& s : samplerStates_) {
        s.name_ = hStringID();
        s.sampler_ = hSamplerStateDesc();
    }
    hZeroMem(vertexLayout_, sizeof(vertexLayout_));
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

void hPipelineStateDescBase::setSampler(hStringID name, const hSamplerStateDesc& ssd) {
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

void hPipelineStateDescBase::setVertexBufferLayout(hVertexBufferLayout* layout, hUint count) {
    hcAssert(count < vertexLayoutMax_);
    vertexLayoutCount = count;
    hMemCpy(vertexLayout_, layout, count*sizeof(hVertexBufferLayout));
}

hInputStateDescBase::hInputStateDescBase() {
    clearDescription();
}

void hInputStateDescBase::clearDescription() {
    for (auto& t : textureSlots_) {
        t.name_ = hStringID();
        t.t1D_ = nullptr;
        t.texType_ = 0;
    }
    for (auto& u : uniformBuffers_) {
        u.name_ = hStringID();
    }
}

void hInputStateDescBase::setTextureSlot(hStringID name, hTexture1D* tex) {
    for (auto& t : textureSlots_) {
        if (t.name_ == name) {
            t.t1D_ = tex;
            t.texType_ = 1;
            return;
        }
        else if (t.name_.is_default()) {
            t.name_ = name;
            t.t1D_ = tex;
            t.texType_ = 1;
            return;
        }
    }

    hcAssertFailMsg("Too many textures bound");
}

void hInputStateDescBase::setTextureSlotWithOverride(hStringID name, hTexture2D* tex, hUint32 override_slot) {
    for (auto& t : textureSlots_) {
        if (t.name_ == name) {
            t.t2D_ = tex;
            t.texType_ = 2;
            t.overrideSlot = override_slot;
            return;
        } else if (t.name_.is_default()) {
            t.name_ = name;
            t.t2D_ = tex;
            t.texType_ = 2;
            t.overrideSlot = override_slot;
            return;
        }
    }

    hcAssertFailMsg("Too many textures bound");
}

void hInputStateDescBase::setTextureSlot(hStringID name, hTexture3D* tex) {
    for (auto& t : textureSlots_) {
        if (t.name_ == name) {
            t.t3D_ = tex;
            t.texType_ = 3;
            return;
        }
        else if (t.name_.is_default()) {
            t.name_ = name;
            t.t3D_ = tex;
            t.texType_ = 3;
            return;
        }
    }

    hcAssertFailMsg("Too many textures bound");
}

void hInputStateDescBase::setTextureSlot(hStringID tex_name, hTexture2D* t) {
    setTextureSlotWithOverride(tex_name, t, ~0UL);
}

void hInputStateDescBase::setUniformBuffer(hStringID name, const hUniformBuffer* ub) {
    for (auto& t : uniformBuffers_) {
        if (t.name_ == name) {
            t.ub_ = ub;
            return;
        }
        else if (t.name_.is_default()) {
            t.name_ = name;
            t.ub_ = ub;
            return;
        }
    }
}

bool hInputStateDesc::findNamedParameter(const hChar* pname, hUint* outindex, hUint* outoffset, hUint* outsize, ShaderParamType* outtype) const {
    const hChar* fieldname = hStrChr(pname, '.');
    if (fieldname) {
        ++fieldname;
    }
    else {
        fieldname = pname;
    }
    for (hUint i = 0, n = uniformBufferMax_; i < n && !uniformBuffers_[i].name_.is_default(); ++i) {
        hUint count;
        auto* layout = hRenderer::getUniformBufferLayoutInfo(uniformBuffers_[i].ub_, &count);
        for (hUint loi = 0, lon = count; loi < lon; ++loi) {
            if (hStrCmp(layout[loi].fieldName, fieldname) == 0) {
                *outindex = i;
                *outoffset = layout[loi].dataOffset;
                *outsize = getParameterTypeByteSize(layout[loi].type);
                *outtype = layout[loi].type;
                if (*outsize == -1) {
                    return false;
                }
                return true;
            }
        }
    }
    return false;
}

}
}
