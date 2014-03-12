/********************************************************************

    filename:   hRenderModel.cpp  
    
    Copyright (c) 27:1:2013 James Moran
    
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

namespace Heart
{

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool hRenderModel::bindVertexStream(hUint inputSlot, hVertexBuffer* vtxBuf) {
#pragma message ("Fix binding instance stream on render model")
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
    hRenderCommandGenerator rcGen(&renderCommands_);
    for (auto i=renderables_.begin(),n=renderables_.end(); i<n; ++i) {
        i->initialiseRenderCommands(&rcGen);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hRenderModel::destroyRenderCommands() {
    renderCommands_.reset();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool hRenderModel::resourceUpdate(hStringID resourceid, hResurceEvent event, hResourceManager* resManager, hResourceClassBase* resource) {
#if 0
    if (event == hResourceEvent_DBInsert) {
        attemptResourceInsert(resManager);
    } else if (event == hResourceEvent_DBRemove) {
        resManager->removeResource(getResourceID());
        destroyRenderCommands();
    }
#else
    hcPrintf("Stub "__FUNCTION__);
#endif
    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hRenderModel::listenForResourceEvents(hResourceManager* resmanager) {
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
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hRenderModel::stopListeningForResourceEvents() {
    hUint32 renderablecount = getRenderableCount();
    for(hUint32 i = 0; i < renderablecount; ++i) {
        hRenderable* renderable=getRenderable(i);
        if (renderable->GetMaterial() == 0) {
            hResourceHandle mat(renderable->getMaterialResourceID());
            mat.unregisterForUpdates(hFUNCTOR_BINDMEMBER(hResourceEventProc, hRenderModel, resourceUpdate, this));
        }
    }
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

hBool hRenderModel::attemptResourceInsert(hResourceManager* resManager) {
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
    return hTrue;
}

}