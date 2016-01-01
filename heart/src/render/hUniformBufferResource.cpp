/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "hUniformBufferResource.h"
#include "base/hRendererConstants.h"
#include "render/hProgramReflectionInfo.h"
#include "render/hUniformBufferFlags.h"

namespace Heart {

hRegisterObjectType(hUniformBufferResource, Heart::hUniformBufferResource, Heart::proto::uniformbuffer::Descriptor);

static hRenderer::ShaderParamType convertToShaderParamType(Heart::proto::uniformbuffer::DataType val) {
    switch(val) {
    case Heart::proto::uniformbuffer::float1 : return hRenderer::ShaderParamType::Float;
    case Heart::proto::uniformbuffer::float2 : return hRenderer::ShaderParamType::Float2;
    case Heart::proto::uniformbuffer::float3 : return hRenderer::ShaderParamType::Float3;
    case Heart::proto::uniformbuffer::float4 : return hRenderer::ShaderParamType::Float4;
    /*
    case Heart::proto::uniformbuffer::int1   : return ShaderParamType::Float1;
    case Heart::proto::uniformbuffer::int2   : return ShaderParamType::Float1;
    case Heart::proto::uniformbuffer::int3   : return ShaderParamType::Float1;
    case Heart::proto::uniformbuffer::int4   : return ShaderParamType::Float1;
    case Heart::proto::uniformbuffer::uint1  : return ShaderParamType::Float1;
    case Heart::proto::uniformbuffer::uint2  : return ShaderParamType::Float1;
    case Heart::proto::uniformbuffer::uint3  : return ShaderParamType::Float1;
    case Heart::proto::uniformbuffer::uint4  : return ShaderParamType::Float1;
    */
    case Heart::proto::uniformbuffer::float22: return hRenderer::ShaderParamType::Float22;
    case Heart::proto::uniformbuffer::float23: return hRenderer::ShaderParamType::Float23;
    case Heart::proto::uniformbuffer::float24: return hRenderer::ShaderParamType::Float24;
    case Heart::proto::uniformbuffer::float32: return hRenderer::ShaderParamType::Float32;
    case Heart::proto::uniformbuffer::float33: return hRenderer::ShaderParamType::Float33;
    case Heart::proto::uniformbuffer::float34: return hRenderer::ShaderParamType::Float34;
    case Heart::proto::uniformbuffer::float42: return hRenderer::ShaderParamType::Float42;
    case Heart::proto::uniformbuffer::float43: return hRenderer::ShaderParamType::Float43;
    case Heart::proto::uniformbuffer::float44: return hRenderer::ShaderParamType::Float44;
    }
    return hRenderer::ShaderParamType::Unknown;
}

hUniformBufferResource::hUniformBufferResource(Heart::proto::uniformbuffer::Descriptor* obj) {
    name.set(obj->descriptortypename().c_str());
    uniformBufferLayout.reserve(obj->elements_size());
    layoutNames.reserve(obj->elements_size());
    for (hUint32 i=0, n=obj->elements_size(); i<n; ++i) {
        const auto& e = obj->elements(i);
        hRenderer::hUniformLayoutDesc desc;
        layoutNames.push_back(hStringID(e.name().c_str()));
        desc.fieldName = layoutNames.back().c_str();
        desc.dataOffset = e.offset();
        desc.type = convertToShaderParamType(e.type());
        uniformBufferLayout.push_back(desc);
    }
    byteSize = obj->totalbytesize();
    uniformBuffer.reset(hRenderer::createUniformBuffer(nullptr, uniformBufferLayout.data(), (hUint)uniformBufferLayout.size(), byteSize, (hUint32)hRenderer::hUniformBufferFlags::Dynamic));
}

hBool hUniformBufferResource::serialiseObject(Heart::proto::uniformbuffer::Descriptor* obj, const hSerialisedEntitiesParameters& params) const {
    return hTrue;
}

hBool hUniformBufferResource::linkObject() {
    return hTrue;
}

}
