/********************************************************************

	filename: 	hRenderable.cpp	
	
	Copyright (c) 13:10:2012 James Moran
	
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

}