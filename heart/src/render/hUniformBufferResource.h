/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hStringID.h"
#include "components/hObjectFactory.h"
#include "render/hRenderer.h"
#include "resource_uniform_buffer.pb.h"

namespace Heart {
class hUniformBufferResource {
    hUint byteSize;
    hStringID name;
    hRenderer::hUniformBufferUniquePtr uniformBuffer;
    std::vector<hStringID> layoutNames;
    std::vector<hRenderer::hUniformLayoutDesc> uniformBufferLayout;

public:
    hObjectType(hUniformBufferResource, Heart::proto::uniformbuffer::Descriptor);

    hStringID getName() const { return name; }
    const hRenderer::hUniformLayoutDesc* getLayoutDesc() const { return uniformBufferLayout.data(); }
    hUint getLayoutDescSize() const { return (hUint)uniformBufferLayout.size(); }
    hUint getByteSize() const { return byteSize; }
    hRenderer::hUniformBuffer* getSharedUniformBuffer() const { return uniformBuffer.get(); }
};
}
