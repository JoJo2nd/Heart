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
        if (blendState_)
            renderer->DestroyBlendState(blendState_);
        blendState_ = NULL;
        if (depthStencilState_)
            renderer->DestoryDepthStencilState(depthStencilState_);
        depthStencilState_ = NULL;
        if (rasterizerState_)
            renderer->DestoryRasterizerState(rasterizerState_);
        rasterizerState_ = NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::BindConstantBuffer(hShaderParameterID paramID, hdParameterConstantBlock* buffer)
    {
        return renderInput_.BindConstantBuffer(paramID, buffer);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::BindSamplerInput(hShaderParameterID paramID, hdSamplerState* srv)
    {
        return renderInput_.BindSamplerInput(paramID, srv);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::BindResourceView(hShaderParameterID paramID, hTexture* view)
    {
        return renderInput_.BindResourceView(paramID, view ? view->pImpl() : NULL);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::BindShaderProgram(hdShaderProgram* prog)
    {
        return renderInput_.BindShaderProgram(prog);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechniquePass::bindInputStreams(PrimitiveType type, hIndexBuffer* idx, hVertexBuffer** vtx, hUint streamcnt) {
        boundStreams_.setPrimType(type);
        boundStreams_.bindIndexVertex(idx ? idx->pImpl() : NULL, idx ? idx->getIndexBufferType() : hIndexBufferType_Index16);
        for(hUint i=0; i<streamcnt; ++i) {
            if (vtx[i]) {
                boundStreams_.bindVertexStream(i, vtx[i]->pImpl(), vtx[i]->getStride());
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
            boundStreams_.bindVertexStream(slot, vtx->pImpl(), vtx->getStride());
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

}