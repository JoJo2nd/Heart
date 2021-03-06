/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "base/hRendererConstants.h"
#include "components/hObjectFactory.h"
#include "resource_shader.pb.h"

namespace Heart {
namespace hRenderer {
    struct hShaderStage;
}
class  hShaderProgram {
    struct ProgramProfile {
        hRenderer::hShaderStage*   shader_;
        hShaderProfile             profile_;
    } *shaderProfiles_;
    hUint32 profileCount_;

public:
    hObjectType(hShaderProgram, Heart::proto::ShaderResourceContainer);

    hShaderProgram() 
        : shaderProfiles_(nullptr)
        , profileCount_(0) {

    }
    ~hShaderProgram() {
        delete[] shaderProfiles_;
    }

    hRenderer::hShaderStage* getShader(hShaderProfile profile) const {
        if (!shaderProfiles_) {
            return nullptr;
        }
        for (hUint32 i=0; i<profileCount_; ++i) {
            // if shaderProfiles_[i].shader_ is null then the profile isn't supported by this platform
            if (shaderProfiles_[i].profile_ == profile) {
                hcAssertMsg(shaderProfiles_[i].shader_, "Requested shader profile was found but is not supported");
                return shaderProfiles_[i].shader_;
            }
        }
        return nullptr;
    }
};    
}