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

void hRenderable::SetMaterial(hMaterialInstance* material)
{
    hcAssertMsg(material, "Material cannot be NULL");

    matInstance_ = material;
    materialKey_ = matInstance_->getMaterialKey();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hRenderable::initialiseRenderCommands(hRenderCommandGenerator* rcGen) {
    hcAssert(matInstance_);
    hUint layoutdesccount=0;
    hInputLayoutDesc* layoutdesc=hNullptr;

    for (hUint i=0, n=vertexBuffer_.GetSize(); i<n; ++i) {
        if (vertexBuffer_[i]) {
            layoutdesccount+=vertexBuffer_[i]->getDescCount();
        }
    }
    layoutdesc=(hInputLayoutDesc*)hAlloca(sizeof(hInputLayoutDesc)*layoutdesccount);
    layoutdesccount=0;
    for (hUint i=0, n=vertexBuffer_.GetSize(); i<n; ++i) {
        if (vertexBuffer_[i]) {
            hMemCpy(layoutdesc+layoutdesccount, vertexBuffer_[i]->getLayoutDesc(), vertexBuffer_[i]->getDescCount()*sizeof(hInputLayoutDesc));
            layoutdesccount+=vertexBuffer_[i]->getDescCount();
        }
    }

    cmdLookUp_.init(matInstance_->getParentMaterial());
    for(hUint g=0, ng=matInstance_->getGroupCount(); g<ng; ++g) {
        hMaterialGroup* group=matInstance_->getGroup(g);
        for (hUint t=0, nt=group->getTechCount(); t<nt; ++t) {
            hMaterialTechnique* tech=group->getTech(t);
            for (hUint p=0, np=tech->GetPassCount(); p<np; ++p) {
                hMaterialTechniquePass* pass=tech->GetPass(p);
                hdInputLayout* inputlayout=pass->GetVertexShader()->createVertexLayout(layoutdesc, layoutdesccount);
                inputLayouts_.PushBack(inputlayout);
                cmdLookUp_.setCommand(g, t, p, rcGen->getRenderCommandsSize());
                rcGen->setJump(matInstance_->getRenderCommandsBegin(g, t, p));
                rcGen->setStreamInputs(primType_, 
                    indexBuffer_, 
                    indexBuffer_ ? indexBuffer_->getIndexBufferType() : hIndexBufferType_Index16, 
                    inputlayout, 
                    vertexBuffer_.GetBuffer(), 
                    0, vertexBuffer_.GetSize());
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

}