/********************************************************************

	filename: 	hRenderSubmissionContext.cpp	
	
	Copyright (c) 3:1:2012 James Moran
	
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

    void hRenderSubmissionCtx::SetIndexStream( hIndexBuffer* idxBuf )
    {
        impl_.SetIndexStream( idxBuf->pImpl() );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::SetVertexStream( hUint32 stream, hVertexBuffer* vtxBuf )
    {
        impl_.SetVertexStream( 0, vtxBuf->pImpl(), vtxBuf->GetStride() );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::SetRenderTarget( hUint32 idx , hTexture* target )
    {
        impl_.SetRenderTarget( idx, target ? target->pImpl() : NULL );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::SetDepthTarget( hTexture* depth )
    {
        impl_.SetDepthTarget( depth ? depth->pImpl() : NULL );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::SetViewport( const hViewport& viewport )
    {
        impl_.SetViewport( viewport );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::SetScissorRect( const ScissorRect& scissor )
    {
        impl_.SetScissorRect( scissor );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::SetPixelShader( hShaderProgram* ps )
    {
        impl_.SetPixelShader( ps->pImpl() );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::SetVertexShader( hShaderProgram* vs )
    {
        impl_.SetVertexShader( vs->pImpl() );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::SetConstantBuffer( hdParameterConstantBlock* constBuffer )
    {
        impl_.SetConstantBlock( constBuffer );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::SetSampler( hUint32 idx, hTexture* tex, hdSamplerState* samplerState )
    {
        impl_.SetSampler( idx, tex->pImpl(), samplerState );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::ClearTarget( hBool clearColour, const hColour& colour, hBool clearZ, hFloat z )
    {
        impl_.ClearTarget( clearColour, colour, clearZ, z );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::SetPrimitiveType( PrimitiveType primType )
    {
        impl_.SetPrimitiveType( primType );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::DrawPrimitive( hUint32 nPrimatives, hUint32 startVertex )
    {
        impl_.DrawPrimitive( nPrimatives, startVertex );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::DrawIndexedPrimitive( hUint32 nPrimatives, hUint32 startVertex )
    {
        impl_.DrawIndexedPrimitive( nPrimatives, startVertex );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::RunCommandBuffer( hdRenderCommandBuffer cmdBuf )
    {
        impl_.RunSubmissionBuffer( cmdBuf );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Map( hIndexBuffer* ib, hIndexBufferMapInfo* outInfo )
    {
        hdMappedData md;
        impl_.Map( ib->pImpl(), &md );
        outInfo->ib_ = ib;
        outInfo->ptr_ = md.pData;
        outInfo->size_ = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Map( hVertexBuffer* vb, hVertexBufferMapInfo* outInfo )
    {
        hdMappedData md;
        impl_.Map( vb->pImpl(), &md );
        outInfo->vb_ = vb;
        outInfo->ptr_ = md.pData;
        outInfo->size_ = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Map( hTexture* tex, hUint32 level, hTextureMapInfo* outInfo )
    {
        hdMappedData md;
        impl_.Map( tex->pImpl(), level, &md );
        outInfo->tex_ = tex;
        outInfo->ptr_ = md.pData;
        outInfo->pitch_ = md.RowPitch;
        outInfo->level_ = level;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Unmap( hIndexBufferMapInfo* outInfo )
    {
        impl_.Unmap( outInfo->ib_->pImpl(), outInfo->ptr_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Unmap( hVertexBufferMapInfo* outInfo )
    {
        impl_.Unmap( outInfo->vb_->pImpl(), outInfo->ptr_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Unmap( hTextureMapInfo* outInfo )
    {
        impl_.Unmap( outInfo->tex_->pImpl(), outInfo->level_, outInfo->ptr_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::RenderDebugSphere( const hVec3& centre, hFloat radius, const hColour& colour )
    {
        if ( !debugEnabled_ )
            return;

        hMatrix m = hMatrixFunc::scale( radius, radius, radius ) * hMatrixFunc::Translation( centre );
        SetWorldMatrix( m );
//         instanceConstants_->world_ = m;
//         instanceConstants_->worldView_ = m * viewportConstants_->view_;
//         instanceConstants_->worldViewProj_ = m * viewportConstants_->view_ * viewportConstants_->projection_;

        debugMaterial_->SetShaderParameter( debugColourParameter_, (hFloat*)&colour, 4 );
        debug_.SetPrimitiveType( PRIMITIVETYPE_TRILIST );

        hUint32 passes = debugTechnique_->GetPassCount();
        for ( hUint32 i = 0; i < passes; ++i )
        {
            hMaterialTechniquePass* pass = debugTechnique_->GetPass( i );
            debug_.SetVertexShader( pass->GetVertexShader()->pImpl() );
            debug_.SetPixelShader( pass->GetPixelShader()->pImpl() );
            debug_.SetRenderStateBlock( pass->GetBlendState() );
            debug_.SetRenderStateBlock( pass->GetDepthStencilState() );
            debug_.SetRenderStateBlock( pass->GetRasterizerState() );

            for ( hUint32 i = 0; i < debugMaterial_->GetConstantBufferCount(); ++i )
            {
                debug_.SetConstantBlock( debugMaterial_->GetConstantBlock(i) );
            }

            debug_.SetIndexStream( debugIB_->pImpl() );
            debug_.SetVertexStream( 0, debugVB_->pImpl(), debugVB_->GetStride() );
            debug_.DrawIndexedPrimitive( debugIB_->GetIndexCount() / 3, 0 );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::InitialiseDebugInterface( hIndexBuffer* sphereIB, hVertexBuffer* sphereVB, hMaterial* material )
    {
        debugIB_ = sphereIB;
        debugVB_ = sphereVB;
        //debugMaterial_ = material->CreateMaterialInstance();
        //debugTechnique_ = debugMaterial_->GetTechniqueByName( "main" );
        //debugColourParameter_ = debugMaterial_->GetShaderParameter( "debugColour" );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::SetRendererCamera( hRendererCamera* camera )
    {
        viewportConstants_ = camera->GetViewportConstants();
        viewportConstantsBlock_ = camera->GetViewportConstantBlock();
        techniqueMask_ = camera->GetTechniqueMask();

        SetRenderTarget( 0, camera->GetRenderTarget(0) );
        SetRenderTarget( 1, camera->GetRenderTarget(1) );
        SetRenderTarget( 2, camera->GetRenderTarget(2) );
        SetRenderTarget( 3, camera->GetRenderTarget(3) );
        SetDepthTarget( camera->GetDepthTarget() );
        SetViewport( camera->GetViewport() );

        impl_.SetConstantBlock( viewportConstantsBlock_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::SetMaterialInstance( hMaterialInstance* instance )
    {
        currentMaterial_ = instance;
        currentTechnique_ = instance->GetTechniqueByMask( techniqueMask_ );
        hcAssert( currentTechnique_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::BeingMaterialInstancePass( hUint32 i )
    {
        hcAssert( currentTechnique_ );
        hcAssert( currentMaterial_ );

        hMaterialTechniquePass* pass = currentTechnique_->GetPass( i );
        SetVertexShader( pass->GetVertexShader() );
        SetPixelShader( pass->GetPixelShader() );
        SetRenderStateBlock( pass->GetBlendState() );
        SetRenderStateBlock( pass->GetDepthStencilState() );
        SetRenderStateBlock( pass->GetRasterizerState() );

        for ( hUint32 i = 0; i < currentMaterial_->GetConstantBufferCount(); ++i )
        {
            SetConstantBuffer( currentMaterial_->GetConstantBlock(i) );
        }

        for ( hUint32 i = 0; i < currentMaterial_->GetSamplerCount(); ++i )
        {
            const hSamplerParameter* samp = currentMaterial_->GetSamplerParameter( i );
            SetSampler( samp->samplerReg_, samp->boundTexture_, samp->samplerState_ );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::EndMaterialInstancePass()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::SetWorldMatrix( const hMatrix& world )
    {
        instanceConstants_->world_ = world;
        instanceConstants_->worldView_ = world * viewportConstants_->view_;
        instanceConstants_->worldViewProj_ = world * viewportConstants_->view_ * viewportConstants_->projection_;

        impl_.SetConstantBlock( instanceConstantsBlock_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Initialise( hRenderer* renderer )
    {
        hUint32 size = sizeof( hInstanceConstants );
        hUint32 reg = HEART_INSTANCE_CONSTANTS_REGISTIER;
        instanceConstantsBlock_ = renderer->CreateConstantBuffers( &size, &reg, 1 );
        instanceConstants_ = (hInstanceConstants*)instanceConstantsBlock_->GetBufferAddress();
    }

}