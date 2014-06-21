/********************************************************************

    Copyright (c) James Moran
    
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    
    3. This notice may not be removed or altered from any source
    distribution.

*********************************************************************/

#pragma once

#include "base/hRendererConstants.h"
#include <d3d11.h>

namespace Heart {
namespace hRenderer {
class hShaderStage {
public:
    hShaderStage(hShaderType type) 
        : type_(type)
        , shaderPtr_(nullptr) {
    }
    ~hShaderStage() {
    }
    
    hShaderType                 type_;
    union 
    {
        ID3D11PixelShader*      pixelShader_;
        ID3D11VertexShader*     vertexShader_;
        ID3D11GeometryShader*   geomShader_;
        ID3D11HullShader*       hullShader_;
        ID3D11DomainShader*     domainShader_;
        ID3D11ComputeShader*    computeShader_;
        void*                   shaderPtr_;
    };
};
}
}