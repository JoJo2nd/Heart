/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "resource_renderstate.pb.h"
#include "base/hStringID.h"
#include "base/hColour.h"
#include "render/hVertexBufferLayout.h"

#pragma once

namespace Heart {
namespace hRenderer {

    struct hTexture1D;
    struct hTexture2D;
    struct hTexture3D;
    struct hShaderStage;
    struct hIndexBuffer;
    struct hVertexBuffer;
    struct hUniformBuffer;

    struct hRenderCallDesc {
        struct hBlendStateDesc {
            hBlendStateDesc() {
                blendEnable_           = hFalse;
                srcBlend_              = proto::renderstate::BlendOne;
                destBlend_             = proto::renderstate::BlendZero;
                blendOp_               = proto::renderstate::Add;
                srcBlendAlpha_         = proto::renderstate::BlendOne;
                destBlendAlpha_        = proto::renderstate::BlendZero;
                blendOpAlpha_          = proto::renderstate::Add;
                renderTargetWriteMask_ = ~0u;
            }

            hBool                               blendEnable_;
            proto::renderstate::BlendOp         srcBlend_;
            proto::renderstate::BlendOp         destBlend_;
            proto::renderstate::BlendFunction   blendOp_;
            proto::renderstate::BlendOp         srcBlendAlpha_;
            proto::renderstate::BlendOp         destBlendAlpha_;
            proto::renderstate::BlendFunction   blendOpAlpha_;
            hUint32                             renderTargetWriteMask_;
        };

        struct hDepthStencilStateDesc {
            hDepthStencilStateDesc() {
                depthEnable_        = hFalse;
                depthWriteMask_     = ~0u;
                depthFunc_          = proto::renderstate::CompareLess;
                stencilEnable_      = hFalse;
                stencilReadMask_    = ~0u;
                stencilWriteMask_   = ~0u;
                stencilFailOp_      = proto::renderstate::StencilKeep;
                stencilDepthFailOp_ = proto::renderstate::StencilKeep;
                stencilPassOp_      = proto::renderstate::StencilKeep;
                stencilFunc_        = proto::renderstate::CompareAlways;
                stencilRef_         = 0;  
            }

            hBool                               depthEnable_;
            hUint32                             depthWriteMask_;
            proto::renderstate::FunctionCompare depthFunc_;
            hBool                               stencilEnable_;
            hUint32                             stencilReadMask_;
            hUint32                             stencilWriteMask_;
            proto::renderstate::StencilOp       stencilFailOp_;
            proto::renderstate::StencilOp       stencilDepthFailOp_;
            proto::renderstate::StencilOp       stencilPassOp_;
            proto::renderstate::FunctionCompare stencilFunc_;
            hUint32                             stencilRef_;
        };

        struct hRasterizerStateDesc {
            hRasterizerStateDesc() {
                fillMode_              = proto::renderstate::Solid;
                cullMode_              = proto::renderstate::CullNone;
                frontCounterClockwise_ = 0;
                depthBias_             = 0;
                depthBiasClamp_        = 0.f;
                slopeScaledDepthBias_  = 0.f;
                depthClipEnable_       = 1;
                scissorEnable_         = 0;
            }
 
            proto::renderstate::FillMode fillMode_;
            proto::renderstate::CullMode cullMode_;
            hUint32                      frontCounterClockwise_;
            hInt32                       depthBias_;
            hFloat                       depthBiasClamp_;
            hFloat                       slopeScaledDepthBias_;
            hUint32                      depthClipEnable_;
            hUint32                      scissorEnable_;
        };

        struct hSamplerStateDesc {
            hSamplerStateDesc() {
                filter_        = proto::renderstate::point;
                addressU_      = proto::renderstate::wrap;
                addressV_      = proto::renderstate::wrap;
                addressW_      = proto::renderstate::wrap;
                mipLODBias_    = 0;
                maxAnisotropy_ = 16;
                borderColour_  = WHITE;
                minLOD_        = -1000;
                maxLOD_        = 1000;
            }

            proto::renderstate::SamplerState  filter_;
            proto::renderstate::SamplerBorder addressU_;
            proto::renderstate::SamplerBorder addressV_;
            proto::renderstate::SamplerBorder addressW_;
            hFloat                            mipLODBias_;
            hUint32                           maxAnisotropy_;
            hColour                           borderColour_;
            hFloat                            minLOD_;
            hFloat                            maxLOD_;
        };

        hRenderCallDesc();
        void clearDescription();
        void setSampler(hStringID name, const hSamplerStateDesc& ssd);
        void setTextureSlot(hStringID name, const hTexture1D* t);
        void setTextureSlot(hStringID name, const hTexture2D* t);
        void setTextureSlot(hStringID name, const hTexture3D* t);
        void setUniformBuffer(hStringID name, const hUniformBuffer* ub);
        void setVertexBufferLayout(hVertexBufferLayout* layout, hUint count);

        static const hUint      vertexLayoutMax_  = 16;
        static const hUint      samplerStateMax_  = 16;
        static const hUint      uniformBufferMax_ = 32;
        static const hUint      textureSlotMax_   = 32;
        
        struct {
            hStringID           name_;
            hSamplerStateDesc   sampler_;
        } samplerStates_[samplerStateMax_];
        hVertexBufferLayout     vertexLayout_[vertexLayoutMax_];
        struct {
            hStringID   name_;
            union {
                const hTexture1D* t1D_;
                const hTexture2D* t2D_;
                const hTexture3D* t3D_;
            };
            hUint8 texType_; // 1, 2 or 3
        } textureSlots_[textureSlotMax_];
        struct {
            hStringID name_;
            const hUniformBuffer* ub_;
        } uniformBuffers_[uniformBufferMax_];
        hBlendStateDesc         blend_;
        hDepthStencilStateDesc  depthStencil_;
        hRasterizerStateDesc    rasterizer_;
        hIndexBuffer*           indexBuffer_;
        hVertexBuffer*          vertexBuffer_;
        hShaderStage*           vertex_;
        hShaderStage*           fragment_;
    };
} 
}