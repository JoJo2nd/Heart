/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "hRenderPlane2D.h"
#include "base/hUUID.h"
#include "core/hResourceManager.h"
#include "math/hVec4.h"
#include "math/hVector.h"
#include "render/hProgramReflectionInfo.h"
#include "render/hVertexBufferLayout.h"
#include "render/hRenderer.h"
#include "render/hVertexBufferFlags.h"
#include "render/hUniformBufferFlags.h"
#include "render/hPipelineStateDesc.h"
#include "render/hRenderShaderProgram.h"
#include "render/hTextureResource.h"
#include "shaders/hTileRenderer2DConstants.h"
#include "2d/hTileRenderer2D.h"
#include "2d/hDynamicTileSet2D.h"

namespace Heart {
hRegisterObjectType(hRenderPlane2D, Heart::hRenderPlane2D, Heart::proto::RenderPlane2D);

hRenderPlane2D::hRenderPlane2D(Heart::proto::RenderPlane2D* obj) {
    linkInfo.reset(new LoadLinkInfo);
    name = hStringID(obj->planename().c_str());
    if (obj->has_planewidth()) gridSizeX = obj->planewidth();
    if (obj->has_planeheight()) gridSizeY = obj->planeheight();
    tileSizeX = obj->tilesize();
    tileSizeY = obj->tilesize();
    renderOrder = obj->renderorder();
    if (obj->has_tilesetresource()) linkInfo->tileSetResource = hStringID(obj->tilesetresource().c_str());
    if (obj->has_tilesetentity()) linkInfo->tileSetEntity = hUUID::fromString(obj->tilesetentity().c_str(), obj->tilesetentity().size());
    xWrapAround = obj->xwraparound();
    yWrapAround = obj->ywraparound();
    renderSectorSize = obj->sectorsize();
    if (obj->has_shaderresource()) linkInfo->shaderResource = hStringID(obj->shaderresource().c_str());
    dynamic = obj->dynamic();
    if (obj->tiledata_size()) {
        // Load in the tile data
    } else {
        // Initialise to zero size of plane
        tiles.resize(gridSizeX*gridSizeY);
    }
}

hBool hRenderPlane2D::serialiseObject(Heart::proto::RenderPlane2D* obj) const {
    return hTrue;
}

hBool hRenderPlane2D::linkObject() {
    if (!linkInfo) return hTrue;
    hBool linked = hTrue;
    if (!linkInfo->tileSetResource.is_default() && !staticTileSet) {
        //Find our resource...
        staticTileSet = hResourceManager::weakResource<hTextureAtlasResource>(linkInfo->tileSetResource);
        linked &= !!staticTileSet;
    } else if (!hUUID::isNull(linkInfo->tileSetEntity)) {
        // Otherwise, wait our tile set to be applied
        if (auto* entity = hEntityFactory::findEntity(linkInfo->tileSetEntity)) {
            tileSet = entity->getComponent<hDynamicTileSet2D>();
        }
        linked &= !!tileSet;
    }
    if (!shaderProgram) {
        shaderProgram = hResourceManager::weakResource<hShaderProgram>(linkInfo->shaderResource);
    }
    linked &= shaderProgram != nullptr;
    if (linked) {
        hRenderer::hVertexBufferLayout lo[] = {
            { hStringID("in_position"), hRenderer::hSemantic::Position, 0, 2, hRenderer::hVertexInputType::Float    ,                  0, hFalse, sizeof(hTileRenderer2D::Vert2D) },
            { hStringID("in_uv")      , hRenderer::hSemantic::Texcoord, 0, 2, hRenderer::hVertexInputType::HalfFloat, sizeof(hFloat) * 2, hFalse, sizeof(hTileRenderer2D::Vert2D) },
        };
        hUint gridVertexBufferSize = 6*renderSectorSize*renderSectorSize;
        if (dynamic) {
            gridVertexBufferSize *= g_RenderFenceCount;
        }
        gridVertexBuffer.reset(hRenderer::createVertexBuffer(nullptr, sizeof(hTileRenderer2D::Vert2D), gridVertexBufferSize, (hUint32)hRenderer::hVertexBufferFlags::DynamicBuffer));
        hUint32 layout_count;
        auto* layout = hTileRenderer2D::getTilePlaneParametersLayout(&layout_count);
        planeUniformBuffer.reset(hRenderer::createUniformBuffer(nullptr, layout, layout_count, sizeof(hTileRenderer2D::TilePlaneParameters), g_RenderFenceCount, (hUint32)hRenderer::hUniformBufferFlags::Dynamic));

        hRenderer::hPipelineStateDesc plsd;
        plsd.blend_.blendEnable_ = hFalse;
        plsd.blend_.srcBlend_ = proto::renderstate::BlendOne;
        plsd.blend_.destBlend_ = proto::renderstate::BlendInverseSrcAlpha;
        plsd.blend_.blendOp_ = proto::renderstate::Add;
        plsd.vertex_ = shaderProgram->getShader(hRenderer::getActiveProfile(hShaderFrequency::Vertex));
        plsd.fragment_ = shaderProgram->getShader(hRenderer::getActiveProfile(hShaderFrequency::Pixel));
        plsd.vertexBuffer_ = gridVertexBuffer.get();
        plsd.setVertexBufferLayout(lo, (hUint)hStaticArraySize(lo));
        hRenderer::hPipelineStateDesc::hSamplerStateDesc ssd;
        plsd.setSampler(hStringID("tSampler"), ssd);
        pipelineState.reset(hRenderer::createRenderPipelineState(plsd));
        hRenderer::hInputStateDesc isd;
        isd.setTextureSlot(hStringID("tilesTexture"), tileSet->getTexture2D());
        isd.setUniformBuffer(hStringID("ViewportConstants"), hTileRenderer2D::getViewUniformBuffer());
        //isd.setUniformBuffer(hStringID("tileRendererParameters"), hTileRenderer2D::getSharedUniformBuffer());
        isd.setUniformBuffer(hStringID("tilePlaneParameters"), planeUniformBuffer.get());
        inputState.reset(hRenderer::createRenderInputState(isd, plsd));

        linkInfo.reset();
    }
    return linked;
}

void hRenderPlane2D::updateTileData(hRect region, hUint* data) {
    hUint* dst = tiles.data()+(gridSizeX*region.top_)+region.left_;
    hUint* src = data;
    hUint src_pitch = region.right_-region.left_;
    for (hUint y=region.top_; y<region.bottom_ && y<gridSizeY; ++y) {
        for (hUint x=region.left_; x<region.right_ && x<gridSizeX; ++x) {
            hMemCpy(dst, src, sizeof(hUint)*src_pitch);
        }
        dst += gridSizeX;
        src += src_pitch;
    }
    dirty = true;
}

void hRenderPlane2D::updateView(hVec3 topLeft, hVec3 bottomRight) {
    hFloat tilesX = fabs(bottomRight.getX()-topLeft.getX())+(tileSizeX-1)/tileSizeX;
    hFloat tilesY = fabs(bottomRight.getY() - topLeft.getY())+(tileSizeY-1) / tileSizeY;
    tilesX = hMin(tilesX, renderSectorSize);
    tilesY = hMin(tilesY, renderSectorSize);
    offsetX = hClamp((hInt)(topLeft.getX() / tileSizeX), 0, (hInt)gridSizeX);
    offsetY = hClamp((hInt)(bottomRight.getY() / tileSizeY), 0, (hInt)gridSizeY);
    viewSizeX = (hInt)tilesX;
    viewSizeY = (hInt)tilesY;
}

void hRenderPlane2D::updateSectorVertexBuffer(hRenderer::hCmdList* cl, hUint current_fence) {
    if (!dynamic) return;
    hUint32 vb_size;
    hUint32 updateSize = 6*renderSectorSize*renderSectorSize;
    auto* vb = (hTileRenderer2D::Vert2D*)hRenderer::getMappingPtr(gridVertexBuffer.get(), &vb_size);
    vb += current_fence*updateSize;
    hUint* dst = tiles.data() + (gridSizeX*offsetY) + offsetX;
    for (hUint y = offsetY, yn=offsetY+viewSizeY; y < yn && y < gridSizeY; ++y) {
        for (hUint x = offsetX, xn=offsetX+viewSizeX; x < xn && x < gridSizeX; ++x) {
            auto* t = tileSet->getTileInfo(*(dst+x));
            hFloat x_pos = (hFloat)x*tileSizeX;
            hFloat y_pos = (hFloat)y*tileSizeY;
            hHalfFloat u1 = hFloatToHalf((hFloat)t->x/(hFloat)tileSet->getWidth());
            hHalfFloat v1 = hFloatToHalf((hFloat)t->y/(hFloat)tileSet->getHeight());
            hHalfFloat u2 = hFloatToHalf((hFloat)(t->x+t->w)/(hFloat)tileSet->getWidth());
            hHalfFloat v2 = hFloatToHalf((hFloat)(t->y+t->h)/(hFloat)tileSet->getHeight());
            // Tri 1
            vb->pos.x = x_pos; vb->pos.y = y_pos;
            vb->uv.x = u1; vb->uv.y = v2;
            ++vb;
            vb->pos.x = x_pos; vb->pos.y = y_pos + tileSizeY;
            vb->uv.x = u1; vb->uv.y = v1;
            ++vb;
            vb->pos.x = x_pos + tileSizeX; vb->pos.y = y_pos + tileSizeY;
            vb->uv.x = u2; vb->uv.y = v1;
            ++vb;
            // Tri 2
            vb->pos.x = x_pos; vb->pos.y = y_pos;
            vb->uv.x = u1; vb->uv.y = v2;
            ++vb;
            vb->pos.x = x_pos + tileSizeX; vb->pos.y = y_pos + tileSizeY;
            vb->uv.x = u2; vb->uv.y = v1;
            ++vb;
            vb->pos.x = x_pos + tileSizeX; vb->pos.y = y_pos;
            vb->uv.x = u2; vb->uv.y = v2;
            ++vb;
        }
        dst += gridSizeX;
    }
    hRenderer::flushVertexBufferMemoryRange(cl, gridVertexBuffer.get(), updateSize*currentFence*sizeof(hTileRenderer2D::Vert2D), updateSize*sizeof(hTileRenderer2D::Vert2D));
}

void hRenderPlane2D::submitPlane(hRenderer::hCmdList* cl, hUint current_fence) {
    hUint count = 2*viewSizeX*viewSizeY;
    hUint stride = 6*renderSectorSize*renderSectorSize;
    hRenderer::draw(cl, pipelineState.get(), inputState.get(), hRenderer::Primative::Triangles, count, stride*current_fence);
}

}