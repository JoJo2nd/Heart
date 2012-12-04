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
        HEART_RESOURCE_SAFE_RELEASE(vertexProgram_);
        HEART_RESOURCE_SAFE_RELEASE(fragmentProgram_);
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

    hBool hMaterialTechniquePass::Link( hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager )
    {
        vertexProgram_   = static_cast< hShaderProgram* >( resManager->ltGetResource(vertexProgramID_) );
        fragmentProgram_ = static_cast< hShaderProgram* >( resManager->ltGetResource(fragmentProgramID_) );

        if (!vertexProgram_ || !fragmentProgram_) return hFalse;

        //blendState_        = renderer->CreateBlendState(blendStateDesc_);
        //rasterizerState_   = renderer->CreateRasterizerState(rasterizerStateDesc_);
        //depthStencilState_ = renderer->CreateDepthStencilState(depthStencilStateDesc_);

        return hTrue;
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

    void hMaterialTechnique::AppendPass( const hMaterialTechniquePass& pass )
    {
        passes_.PushBack(pass);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterialTechnique::Link( hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager )
    {
        hBool linkok = hTrue;
        hUint32 nPasses = passes_.GetSize();
        mask_ = matManager->AddRenderTechnique( name_ )->mask_;
        for ( hUint32 pass = 0; pass < nPasses; ++pass )
        {
            linkok &= passes_[pass].Link(resManager, renderer, matManager);
        }

        return linkok;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialTechnique::SetPasses( hUint32 count )
    {
        passes_.Reserve(count);
    }

}