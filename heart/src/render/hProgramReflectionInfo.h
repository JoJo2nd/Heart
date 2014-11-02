/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"

namespace Heart {
namespace hRenderer {

struct hShaderStage;
struct hProgramReflectionInfo;

enum class ShaderParamType {
    Unknown,

    Float,
    Float2,
    Float3,
    Float4,

    Float22,
    Float23,
    Float24,
    Float32,
    Float33,
    Float34,
    Float42,
    Float43,
    Float44,

    Sampler1D,
    Sampler2D,
    Sampler3D,
    SamplerCube,
};

struct hShaderParamInfo {
    hUint           uniformBlock;
    hUint           uniformBlockOffset;
    ShaderParamType type;
    hUint           count;
};

hProgramReflectionInfo* createProgramReflectionInfo(hShaderStage* vertex, hShaderStage* pixel, hShaderStage* geom, hShaderStage* hull, hShaderStage* domain);
void destroyProgramReflectionInfo(hProgramReflectionInfo* p);

hShaderParamInfo getParameterInfo(hProgramReflectionInfo* p, const hChar* name);

}
}