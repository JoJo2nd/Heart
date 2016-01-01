/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "2d/hRenderPlane2D.h"
#include "2d/hSprite2D.h"
#include "2d/hDynamicTileSet2D.h"
#include "2d/hTileRenderer2DConstants.h"
#include "core/hConfigOptions.h"
#include "components/hEntity.h"
#include "components/hEntityFactory.h"
#include "components/hObjectFactory.h"
#include "render/hRenderer.h"
#include "render/hUniformBufferFlags.h"
#include "render/hRendererCamera.h"
#include "render/hVertexBufferFlags.h"
#include "utils/hFreelistAllocator.h"
#include "shaders/ViewportConstants.hpp"

namespace Heart {
namespace hTileRenderer2D {
namespace {
hFreeListAllocator<hDynamicTileSet2D> dynmaicTileSetFreelist;
hFreeListAllocator<hRenderPlane2D> renderPlaneFreelist;
hFreeListAllocator<hSprite2D> spriteFreelist;
std::vector<hRenderPlane2D*> renderPlanes;
std::vector<hDynamicTileSet2D*> tileSets;
std::vector<hSprite2D*> sprites;
hRendererCamera viewingCamera;
hUint spriteVertexBufferSize;
hRenderer::hUniformBufferUniquePtr viewUniformBuffer;
hRenderer::hUniformBufferUniquePtr tileRendererSharedBuffer;
hRenderer::hVertexBufferUniquePtr spriteVertexBuffer;
hRenderer::hPipelineStateUniquePtr pipelineState;
hRenderer::hInputStateUniquePtr inputState;
hUint textureOverrideSlot = 0;
hUint currentFence = 0;
std::vector<hRenderer::hRenderFence*> fences;
}

hBool registerComponents() {
    hBool ret = hTrue;
    ret &= hRenderPlane2D::auto_object_registered;
    ret &= hSprite2D::auto_object_registered;
    ret &= hDynamicTileSet2D::auto_object_registered;

    Heart::hEntityFactory::hComponentMgt c;
    c.object_def = hObjectFactory::getObjectDefinition(hRenderPlane2D::getTypeNameStatic());
    c.construct = [](hEntity* owner, hObjectMarshall* marshall) -> hEntityComponent* { 
        auto* real_marshall = static_cast<hRenderPlane2D::MarshallType*>(marshall);
        auto* ptr = new (renderPlaneFreelist.allocate()) hRenderPlane2D(real_marshall);
        renderPlanes.push_back(ptr);
        return ptr;
    };
    c.destruct = [](hEntityComponent* ptr) {
        auto it = std::find(renderPlanes.begin(), renderPlanes.end(), ptr);
        *it = std::move(renderPlanes.back());
        renderPlanes.resize(renderPlanes.size()-1);
        ptr->~hEntityComponent();
        renderPlaneFreelist.release(ptr);
    };
    hEntityFactory::registerComponentManagement(c);

    c.object_def = hObjectFactory::getObjectDefinition(hSprite2D::getTypeNameStatic());
    c.construct = [](hEntity* owner, hObjectMarshall* marshall) -> hEntityComponent* {
        auto* real_marshall = static_cast<hSprite2D::MarshallType*>(marshall);
        auto* ptr = new (spriteFreelist.allocate()) hSprite2D(real_marshall);
        sprites.push_back(ptr);
        return ptr;
    };
    c.destruct = [](hEntityComponent* ptr) {
        auto it = std::find(sprites.begin(), sprites.end(), ptr);
        *it = std::move(sprites.back());
        sprites.resize(sprites.size() - 1);
        ptr->~hEntityComponent();
        spriteFreelist.release(ptr);
    };
    hEntityFactory::registerComponentManagement(c);

    c.object_def = hObjectFactory::getObjectDefinition(hDynamicTileSet2D::getTypeNameStatic());
    c.construct = [](hEntity* owner, hObjectMarshall* marshall) -> hEntityComponent* {
        auto* real_marshall = static_cast<hDynamicTileSet2D::MarshallType*>(marshall);
        auto* ptr = new (dynmaicTileSetFreelist.allocate()) hDynamicTileSet2D(real_marshall);
        tileSets.push_back(ptr);
        return ptr;
    };
    c.destruct = [](hEntityComponent* ptr) { 
        auto it = std::find(tileSets.begin(), tileSets.end(), ptr);
        *it = std::move(tileSets.back());
        tileSets.resize(tileSets.size() - 1);
        ptr->~hEntityComponent();
        dynmaicTileSetFreelist.release(ptr); 
    };
    hEntityFactory::registerComponentManagement(c);

    return ret;
}

hRenderer::hUniformBuffer* getViewUniformBuffer() {
    return viewUniformBuffer.get();
}

hBool initialise() {
    spriteVertexBufferSize = hConfigurationVariables::getCVarUint("renderer.spritevertexbuffersize", 1000);
    hUint32 layout_count;
    fences.resize(g_RenderFenceCount);
    auto* layout = ViewportConstants::getLayout(&layout_count);
    viewUniformBuffer.reset(hRenderer::createUniformBuffer(nullptr, layout, layout_count, sizeof(ViewportConstants), (hUint32)hRenderer::hUniformBufferFlags::Dynamic));
    spriteVertexBuffer.reset(hRenderer::createVertexBuffer(nullptr, sizeof(Vert2D), spriteVertexBufferSize, (hUint32)hRenderer::hVertexBufferFlags::DynamicBuffer));
    return hTrue;
}

void initialiseResources() {

}

void setView(const hRendererCamera& camera) {
    viewingCamera = camera;
}

void transformPlanesToViewSpace() {
    auto* frustum = viewingCamera.getViewFrustum();
    hVec3 topLeft = frustum->frustumPoint(hViewFrustum::Point::NearTopLeft);
    hVec3 bottomRight = frustum->frustumPoint(hViewFrustum::Point::NearBottomRight);
    for (auto& i : renderPlanes) {
        i->updateView(topLeft, bottomRight);
    }
}

void updateDynamicRenderResources(hRenderer::hCmdList* cl) {
    if (fences[currentFence]) {
        hRenderer::wait(fences[currentFence]);
        fences[currentFence] = nullptr;
    }
    for (auto& i : tileSets) {
        i->flushNewTextureData(cl);
    }
    for (auto& i : renderPlanes) {
        i->updateSectorVertexBuffer(cl, currentFence);
    }
    
}

void renderTilePlanes(hRenderer::hCmdList* cl) {
    //update the shared view constants buffer
    ViewportConstants* vp = (ViewportConstants*)hRenderer::getMappingPtr(viewUniformBuffer.get());
    auto view = viewingCamera.getViewMatrix();
    auto view_inverse = inverse(view);
    auto proj = hRenderer::getClipspaceMatrix()*viewingCamera.getProjectionMatrix();
    auto view_proj = proj*view;
   
    vp->view = viewingCamera.getViewMatrix();
    vp->projection = proj;
    vp->viewProjection = proj*view;
    vp->projectionInverse = inverse(proj);
    vp->viewInverse = view_inverse;
    vp->viewInverseTranspose = transpose(view_inverse);
    vp->viewProjectionInverse = inverse(view_proj);

    hRenderer::flushUnibufferMemoryRange(cl, viewUniformBuffer.get(), 0, sizeof(ViewportConstants));
    auto* vertex_base = (Vert2D*)hRenderer::getMappingPtr(spriteVertexBuffer.get(), nullptr); //TODO: get right fence

    hUint prevRenderPlane = 0;
    for (auto& i : renderPlanes) {
        hSprite2D::submitSpriteLayers(prevRenderPlane, i->getPlaneLayer(), cl, pipelineState.get(), inputState.get(), textureOverrideSlot, 0, vertex_base, spriteVertexBufferSize);
        i->submitPlane(cl, currentFence);
    }

    // Once done, create a fence
    fences[currentFence] = hRenderer::fence(cl);
    currentFence = (currentFence+1)%fences.size();
}

hRenderer::hUniformBuffer* getSharedUniformBuffer() {
    return nullptr;
}

}
}
