/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hRenderable.h"
#include "render/hRenderShaderProgram.h"
#include "render/hRenderer.h"

namespace Heart
{
#if 0
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hRenderable::setMaterial(const hResourceHandle& material)
{
    materialKey_=0;
    materialHandle_ = material;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hRenderable::initialiseRenderCommands(hRenderCommandGenerator* rcGen) {

    hMaterial* material;
    hResourceHandleScope<hMaterial> materialResScope(materialHandle_, &material);
    materialKey_=0;

    if (!material) {
        return;
    }

    materialKey_ = material->GetMatKey();

    hUint layoutdesccount=0;
    hInputLayoutDesc* layoutdesc=hNullptr;

    for (hUint i=0, n=(hUint)vertexBuffer_.size(); i<n; ++i) {
        if (vertexBuffer_[i]) {
            layoutdesccount+=vertexBuffer_[i]->getDescCount();
        }
    }
    layoutdesc=(hInputLayoutDesc*)hAlloca(sizeof(hInputLayoutDesc)*layoutdesccount);
    layoutdesccount=0;
    for (hUint i=0, n=(hUint)vertexBuffer_.size(); i<n; ++i) {
        if (vertexBuffer_[i]) {
            hMemCpy(layoutdesc+layoutdesccount, vertexBuffer_[i]->getLayoutDesc(), vertexBuffer_[i]->getDescCount()*sizeof(hInputLayoutDesc));
            layoutdesccount+=vertexBuffer_[i]->getDescCount();
        }
    }

    cmdLookUp_.init(material);
    for(hUint g=0, ng=material->getGroupCount(); g<ng; ++g) {
        hMaterialGroup* group=material->getGroup(g);
        for (hUint t=0, nt=group->getTechCount(); t<nt; ++t) {
            hMaterialTechnique* tech=group->getTech(t);
            for (hUint p=0, np=tech->GetPassCount(); p<np; ++p) {
                hMaterialTechniquePass* pass=tech->GetPass(p);
                hdInputLayout* inputlayout=pass->GetVertexShader()->createVertexLayout(layoutdesc, layoutdesccount);
                inputLayouts_.push_back(inputlayout);
                cmdLookUp_.setCommand(g, t, p, rcGen->getRenderCommandsSize());
                rcGen->setJump(material->getRenderCommandsBegin(g, t, p));
                rcGen->setStreamInputs(primType_, 
                    indexBuffer_, 
                    indexBuffer_ ? indexBuffer_->getIndexBufferType() : hIndexBufferType_Index16, 
                    inputlayout, 
                    vertexBuffer_.data(), 
                    0, (hUint)vertexBuffer_.size());
                if (indexBuffer_) {
                    rcGen->setDrawIndex(primCount_, 0);
                } else {
                    rcGen->setDraw(primCount_, 0);
                }
                rcGen->setReturn();
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool hRenderable::resourceUpdate(hStringID resourceid, hResurceEvent event) {
#if 0
    if (event==hResourceEvent_DBInsert || event==hResourceEvent_HotSwap) {

    } else if (event==hResourceEvent_DBRemove) {

    }
#else
    hcPrintf("Stub "__FUNCTION__);
#endif
    return hTrue;
}
#endif
}