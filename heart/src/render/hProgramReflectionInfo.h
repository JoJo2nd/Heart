/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"

#ifndef HEART_STATIC_RENDER_API
#   define HEART_STATIC_RENDER_API (0)
#endif

#if !HEART_STATIC_RENDER_API
#   define HEART_MODULE_API(r, fn_name) extern r (HEART_API *fn_name)
#else
#   define HEART_MODULE_API(r, fn_name) r fn_name
#endif

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

struct hUniformBlockInfo {
    const hChar*    name;
    hUint           index;
    hInt            size;
};

struct hUniformLayoutDesc {
    char*           fieldName;
    ShaderParamType type;
    hUint16         dataOffset;
};

HEART_MODULE_API(hProgramReflectionInfo*, createProgramReflectionInfo)(hShaderStage* vertex, hShaderStage* pixel, hShaderStage* geom, hShaderStage* hull, hShaderStage* domain);
HEART_MODULE_API(void, destroyProgramReflectionInfo)(hProgramReflectionInfo* p);
HEART_MODULE_API(hShaderParamInfo, getParameterInfo)(hProgramReflectionInfo* p, const hChar* name);
HEART_MODULE_API(hUint, getUniformBlockCount)(hProgramReflectionInfo* p);
HEART_MODULE_API(hUniformBlockInfo, getUniformBlockInfo)(hProgramReflectionInfo* p, hUint i);
HEART_MODULE_API(hUint, getParameterTypeByteSize)(ShaderParamType type);

}
}
