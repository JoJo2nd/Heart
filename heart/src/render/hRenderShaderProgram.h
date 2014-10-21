/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "base/hProtobuf.h"
#include "base/hRendererConstants.h"
#include "components/hObjectFactory.h"

namespace Heart {
namespace hRenderer {
    class hShaderReflection;
    struct hShaderStage;

    /*
        Shader Stage interface
    */
    hShaderType getShaderType(const hShaderStage* stage);
    /*
        hShaderLinkedProgram interface
    */
    class hShaderLinkedProgram;
    const hShaderReflection* getReflectionInfo(const hShaderLinkedProgram* prog);
    void attachStage(hShaderLinkedProgram* prog, hShaderStage* stage);
    void link(hShaderLinkedProgram* prog);
}

class  hShaderProgram {
    hRenderer::hShaderStage*   shader_;

public:
    hObjectType(Heart::hShaderProgram, Heart::proto::ShaderResourceContainer);

    hShaderProgram() 
    : shader_(nullptr){

    }
    ~hShaderProgram() {
    }

    hRenderer::hShaderStage* getShader() const {
        return shader_;
    }
};    
}