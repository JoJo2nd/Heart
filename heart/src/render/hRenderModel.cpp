/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hRenderModel.h"
#include "render/hRenderer.h"

namespace Heart
{

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool hRenderModel::bindVertexStream(hUint inputSlot, hVertexBuffer* vtxBuf) {
#if defined (HEART_PLAT_WINDOWS)
#   pragma message ("Fix binding instance stream on render model")
#endif
//     for (hUint lodIdx=0,lodCnt=levelCount_; lodIdx<lodCnt; ++lodIdx) {
//         hGeomLODLevel* lod=&lodLevels_[lodIdx];
//         for (hUint rndIdx=0,rndCnt=lodLevels_[lodIdx].renderObjects_.GetSize(); rndIdx<rndCnt; ++rndIdx) {
//             lod->renderObjects_[rndIdx].bindVertexStream(inputSlot, vtxBuf);
//         }
//     }
    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hRenderModel::initialiseRenderCommands() {
#if 0
    hRenderCommandGenerator rcGen(&renderCommands_);
    for (auto i=renderables_.begin(),n=renderables_.end(); i<n; ++i) {
        i->initialiseRenderCommands(&rcGen);
    }
#else
    hStub();
#endif
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hRenderModel::destroyRenderCommands() {
    //renderCommands_.reset();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool hRenderModel::resourceUpdate(hStringID resourceid, hResurceEvent event) {
#if 0
    if (event == hResourceEvent_DBInsert) {
        attemptResourceInsert(resManager);
    } else if (event == hResourceEvent_DBRemove) {
        resManager->removeResource(getResourceID());
        destroyRenderCommands();
    }
#else
    hStub();
#endif
    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hRenderModel::listenForResourceEvents() {
#if 0
    hUint32 renderablecount = getRenderableCount();
    for(hUint32 i = 0; i < renderablecount; ++i) {
        hRenderable* renderable=getRenderable(i);
        hResourceHandle mat(renderable->getMaterialResourceID());
        if (mat.getIsValid()) {
            mat.registerForUpdates(hFUNCTOR_BINDMEMBER(hResourceEventProc, hRenderModel, resourceUpdate, this));
        }
    }
    //update
    attemptResourceInsert(resmanager);
#else
    hStub();
#endif
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hRenderModel::stopListeningForResourceEvents() {
#if 0
    hUint32 renderablecount = getRenderableCount();
    for(hUint32 i = 0; i < renderablecount; ++i) {
        hRenderable* renderable=getRenderable(i);
        if (renderable->GetMaterial() == 0) {
            hResourceHandle mat(renderable->getMaterialResourceID());
            mat.unregisterForUpdates(hFUNCTOR_BINDMEMBER(hResourceEventProc, hRenderModel, resourceUpdate, this));
        }
    }
#else
    hStub();
#endif
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hRenderModel::cleanUp() {
    stopListeningForResourceEvents();
    destroyRenderCommands();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool hRenderModel::attemptResourceInsert() {
#if 0
    hUint32 renderablecount = getRenderableCount();
    for(hUint32 i = 0; i < renderablecount; ++i) {
        hRenderable* renderable=getRenderable(i);
        if (renderable->GetMaterial() == 0) {
            hResourceHandle mat(renderable->getMaterialResourceID());
            if (mat.getIsValid() && !mat.weakPtr()) {
                return hFalse;
            }
        }
    }
    // all in place
    initialiseRenderCommands();
    resManager->insertResource(getResourceID(), this);
#else
    hStub();
#endif
    return hTrue;
}

}
