/********************************************************************

    filename: 	hMaterialTypes.cpp	
    
    Copyright (c) 6:9:2012 James Moran
    
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

    void hMaterialTechniquePass::ReleaseResources(hRenderer* renderer)
    {
        vertexProgram_ = NULL;
        fragmentProgram_ = NULL;
        if (blendState_) {
            blendState_->DecRef();
        }
        blendState_ = NULL;
        if (depthStencilState_) {
            depthStencilState_->DecRef();
        }
        depthStencilState_ = NULL;
        if (rasterizerState_) {
            rasterizerState_->DecRef();
        }
        rasterizerState_ = NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::setConstantBuffer(hShaderParameterID paramID, hRenderBuffer* buffer)
    {
        hBool succ=false;
        for (hUint i=0, n=s_maxPrograms; i<n; ++i) {
            if (programs_[i]) {
                hUint32 idx = programs_[i]->GetInputRegister(paramID);
                if (idx > HEART_MAX_RESOURCE_INPUTS) {
                    continue;
                }
                succ=true;
                if (idx+1>inputResources_[i].buffers_.GetSize()) {
                    hUint oldsize=inputResources_[i].buffers_.GetSize();
                    inputResources_[i].buffers_.Resize(idx+1);
                    for (hUint b=oldsize; b<idx+1; ++b) {
                        inputResources_[i].buffers_[b]=hNullptr;
                    }
                }
                inputResources_[i].buffers_[idx]=buffer;
            }
        }
        return succ;// || renderInput_.bindConstantBuffer(paramID, buffer);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::setSamplerInput(hShaderParameterID paramID, hSamplerState* srv)
    {
        hBool succ=false;
        for (hUint i=0, n=s_maxPrograms; i<n; ++i) {
            if (programs_[i]) {
                hUint32 idx = programs_[i]->GetInputRegister(paramID);
                if (idx > HEART_MAX_RESOURCE_INPUTS) {
                    continue;
                }
                succ=true;
                if (idx+1>inputResources_[i].samplerStates_.GetSize()) {
                    hUint oldsize=inputResources_[i].samplerStates_.GetSize();
                    inputResources_[i].samplerStates_.Resize(idx+1);
                    for (hUint b=oldsize; b<idx+1; ++b) {
                        inputResources_[i].samplerStates_[b]=hNullptr;
                    }
                }
                inputResources_[i].samplerStates_[idx]=srv;
            }
        }
        return renderInput_.bindSamplerInput(paramID, srv);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::setResourceView(hShaderParameterID paramID, hShaderResourceView* view)
    {
        hBool succ=false;
        for (hUint i=0, n=s_maxPrograms; i<n; ++i) {
            if (programs_[i]) {
                hUint32 idx = programs_[i]->GetInputRegister(paramID);
                if (idx > HEART_MAX_RESOURCE_INPUTS) {
                    continue;
                }
                succ=true;
                if (idx+1>inputResources_[i].srView_.GetSize()) {
                    hUint oldsize=inputResources_[i].srView_.GetSize();
                    inputResources_[i].srView_.Resize(idx+1);
                    for (hUint b=oldsize; b<idx+1; ++b) {
                        inputResources_[i].srView_[b]=hNullptr;
                    }
                }
                inputResources_[i].srView_[idx]=view;
            }
        }
        return renderInput_.bindResourceView(paramID, view);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::bindShaderProgram(hdShaderProgram* prog)
    {
        return renderInput_.bindShaderProgram(prog);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::bindInputStreams(PrimitiveType type, hIndexBuffer* idx, hVertexBuffer** vtx, hUint streamcnt) {
        boundStreams_.setPrimType(type);
        boundStreams_.bindIndexVertex(idx, idx ? idx->getIndexBufferType() : hIndexBufferType_Index16);
        for(hUint i=0; i<streamcnt; ++i) {
            if (vtx[i]) {
                boundStreams_.bindVertexStream(i, vtx[i], vtx[i]->getStride());
            }
        }
        boundStreams_.bindVertexFetch(vertexProgram_);
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::bindInputStream(hUint slot, hVertexBuffer* vtx) {
        hcAssert(slot < HEART_MAX_INPUT_STREAMS);
        if (vtx) {
            boundStreams_.bindVertexStream(slot, vtx, vtx->getStride());
        } else {
            boundStreams_.bindVertexStream(slot, NULL, 0);
        }
        boundStreams_.bindVertexFetch(vertexProgram_);
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialTechniquePass::unbind() {
        if (vertexProgram_) {
            boundStreams_.unbind(vertexProgram_);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialTechnique::AppendPass( const hMaterialTechniquePass& pass )
    {
        passes_.PushBack(pass);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialTechnique::SetPasses( hUint32 count )
    {
        passes_.Reserve(count);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialTechnique* hMaterialGroup::getTechniqueByName(const hChar* name) {
        for (hUint i=0,n=techniques_.GetSize(); i<n; ++i) {
            if (hStrICmp(name, techniques_[i].GetName()) == 0) {
                return &techniques_[i];
            }
        }
        return hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialTechnique* hMaterialGroup::getTechniqueByMask(hUint32 mask) {
        for (hUint i=0,n=techniques_.GetSize(); i<n; ++i) {
            if (techniques_[i].GetMask()==mask) {
                return &techniques_[i];
            }
        }
        return hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hMaterialGroup::getTechniqueIndexByMask(hUint32 mask) {
        for (hUint i=0,n=techniques_.GetSize(); i<n; ++i) {
            if (techniques_[i].GetMask()==mask) {
                return i;
            }
        }
        return hErrorCode;
    }

}