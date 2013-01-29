
/********************************************************************

    filename: 	DeviceDX11RenderSubmissionCtx.cpp	
    
    Copyright (c) 18:12:2011 James Moran
    
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

    hBool hdDX11RenderInputObject::BindShaderProgram(hdDX11ShaderProgram* prog)
    {
        switch(prog->type_)
        {
        case ShaderType_FRAGMENTPROG: {
            pixelShader_ = prog->pixelShader_; 
            boundProgs_[hdDX11PixelProg] = prog;
            break;
        }
        case ShaderType_VERTEXPROG: {
            vertexShader_ = prog->vertexShader_; 
            boundProgs_[hdDX11VertexProg] = prog;
            break;
        }
        default: return false; // Assert here?
        }
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdDX11RenderInputObject::BindConstantBuffer(hShaderParameterID paramID, hdDX11ParameterConstantBlock* buffer)
    {
        hcAssert(paramID != 0);
        hcAssert(buffer);
        hUint32 cbidx;
        hBool succ = false;

        for (hUint32 p = 0; p < hdDX11ProgMax; ++p) {
            if (!boundProgs_[p]) continue;
            cbidx = boundProgs_[p]->GetConstantBlockRegister(paramID);
            if (cbidx > HEART_MAX_CONSTANT_BLOCKS) continue;
            inputData_[p].programInputs_[cbidx] = buffer->constBuffer_;
            succ = true;
        }

        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdDX11RenderInputObject::BindSamplerInput(hShaderParameterID paramID, hdDX11SamplerState* ss)
    {
        hUint32 idx;
        hBool succ = false;

        for (hUint32 p = 0; p < hdDX11ProgMax; ++p) {
            if (!boundProgs_[p]) continue;
            idx = boundProgs_[p]->GetInputRegister(paramID);
            if (idx > HEART_MAX_RESOURCE_INPUTS) continue;
            inputData_[p].samplerState_[idx] = ss->stateObj_;
            inputData_[p].samplerCount_ = hMax(idx+1, inputData_[p].samplerCount_);
            succ = true;
        }
        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdDX11RenderInputObject::BindResourceView(hShaderParameterID paramID, hdDX11Texture* view)
    {
        hUint32 idx;
        hBool succ = false;

        for (hUint32 p = 0; p < hdDX11ProgMax; ++p) {
            if (!boundProgs_[p]) continue;
            idx = boundProgs_[p]->GetInputRegister(paramID);
            if (idx > HEART_MAX_RESOURCE_INPUTS) continue;
            inputData_[p].resourceViews_[idx] = view ? view->shaderResourceView_ : NULL;
            inputData_[p].resourceViewCount_ = hMax(idx+1, inputData_[p].resourceViewCount_);
            succ = true;
        }
        return succ;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderStreamsObject::bindIndexVertex(hdDX11IndexBuffer* index, hIndexBufferType format)
    {
        index_ = index ? index->buffer_ : NULL;
        indexFormat_= format == hIndexBufferType_Index16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderStreamsObject::bindVertexStream(hUint16 stream, hdDX11VertexBuffer* vertexbuffer, hUint stride)
    {
        hcAssert(stream < HEART_MAX_INPUT_STREAMS);
        hBool setlb = false;

        boundStreams_[stream] = vertexbuffer;
        streams_[stream] = vertexbuffer->buffer_;
        strides_[stream] = stride;
        for (hUint16 i = 0; i < HEART_MAX_INPUT_STREAMS; ++i) {
            if (streams_[i]) {
                if (!setlb) {
                    streamLower_ = i;
                    setlb = hTrue;
                }
                streamUpper_ = i+1;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderStreamsObject::setPrimType(PrimitiveType primType)
    {
        if ( primType == PRIMITIVETYPE_LINELIST ) topology_ = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        else if ( primType == PRIMITIVETYPE_TRILIST) topology_ = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        else if ( primType == PRIMITIVETYPE_TRISTRIP) topology_ = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        else hcAssert( hFalse );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderStreamsObject::bindVertexFetch(hdDX11ShaderProgram* prog) {
        hcAssert(prog);
        hInputLayoutDesc* desc=NULL, *dptr=NULL;
        hUint descn=0;
        for(hUint i=0; i<HEART_MAX_INPUT_STREAMS; ++i) {
            if (boundStreams_[i]) {
                descn+=boundStreams_[i]->streamDescCount_;
            }
        }
        desc=(hInputLayoutDesc*)hAlloca(sizeof(hInputLayoutDesc)*descn);
        dptr=desc;
        for(hUint i=0; i<HEART_MAX_INPUT_STREAMS; ++i) {
            if (boundStreams_[i]) {
                dptr->inputStream_=i;
                hMemCpy(dptr, boundStreams_[i]->streamLayoutDesc_, sizeof(hInputLayoutDesc)*boundStreams_[i]->streamDescCount_);
                dptr+=boundStreams_[i]->streamDescCount_;
            }
        }
        hdInputLayout* vtxlo=prog->createVertexLayout(desc, descn);
        if (vtxlo) {
            layout_=vtxlo->layout_;
        }else{
            layout_=NULL;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetInputStreams(hdDX11RenderStreamsObject* streams)
    {
        hcAssert(streams->layout_);
        hcAssertMsg(streams->streamUpper_ >= streams->streamLower_, "Render Stream Object contains an invalid stream count");
        UINT offsets[HEART_MAX_INPUT_STREAMS] = {0};
        if (primType_ != streams->topology_) {
            device_->IASetPrimitiveTopology(streams->topology_);
        }
        device_->IASetInputLayout(streams->layout_);
        device_->IASetIndexBuffer(streams->index_, streams->indexFormat_, 0);
        device_->IASetVertexBuffers(
            streams->streamLower_,
            streams->streamUpper_-streams->streamLower_,
            streams->streams_+streams->streamLower_,
            streams->strides_+streams->streamLower_,
            offsets);
        primType_ = streams->topology_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetRenderInputObject(hdRenderInputObject* inputobj)
    {
        device_->VSSetShader(inputobj->vertexShader_, NULL, 0);
        if (inputobj->vertexShader_) {
            hdRenderInputObject::RendererInputs* inputs = inputobj->inputData_+hdRenderInputObject::hdDX11VertexProg;
            device_->VSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, inputs->programInputs_);
            device_->VSSetSamplers(0, inputs->samplerCount_, inputs->samplerState_);
            device_->VSSetShaderResources(0, inputs->resourceViewCount_, inputs->resourceViews_);
        }

        device_->PSSetShader(inputobj->pixelShader_, NULL, 0);
        if (inputobj->pixelShader_) {
            hdRenderInputObject::RendererInputs* inputs = inputobj->inputData_+hdRenderInputObject::hdDX11PixelProg;
            device_->PSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, inputs->programInputs_);
            device_->PSSetSamplers(0, inputs->samplerCount_, inputs->samplerState_);
            device_->PSSetShaderResources(0, inputs->resourceViewCount_, inputs->resourceViews_);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetRenderStateBlock( hdDX11BlendState* st )
    {
        hFloat factors[4] = { 1.f, 1.f, 1.f, 1.f };
        device_->OMSetBlendState( st->stateObj_, factors, ~0U );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetRenderStateBlock( hdDX11DepthStencilState* st )
    {
        device_->OMSetDepthStencilState( st->stateObj_, st->stencilRef_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetRenderStateBlock( hdDX11RasterizerState* st )
    {
        device_->RSSetState( st->stateObj_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetRenderTarget( hUint32 idx , hdDX11Texture* target )
    {
        if ( !target && idx == 0)
        {
            renderTargetViews_[0] = defaultRenderView_;
        }
        else if ( !target )
        {
            renderTargetViews_[idx] = NULL;
        }
        else
        {
            hcAssertMsg( target->renderTargetView_, "Texture not created with RESOURCE_RENDERTARGET flag" );
            renderTargetViews_[idx] = target->renderTargetView_;
        }

        device_->OMSetRenderTargets( 4, renderTargetViews_, depthStencilView_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetDepthTarget( hdDX11Texture* depth )
    {
        if ( !depth )
        {
            depthStencilView_ = defaultDepthView_;
        }
        else
        {
            depthStencilView_ = depth->depthStencilView_;
        }

        //if ( depthStencilView_ != depth->depthStencilView_ )
        device_->OMSetRenderTargets( 4, renderTargetViews_, depthStencilView_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetViewport( const hViewport& viewport )
    {
        D3D11_VIEWPORT vp;
        vp.TopLeftX = (FLOAT)viewport.x_;
        vp.TopLeftY = (FLOAT)viewport.y_;
        vp.Width = (FLOAT)viewport.width_;
        vp.Height = (FLOAT)viewport.height_;
        vp.MinDepth = 0.f;
        vp.MaxDepth = 1.f;
        device_->RSSetViewports( 1, &vp );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetScissorRect( const hScissorRect& scissor )
    {
        D3D11_RECT s;
        s.left = scissor.left_;
        s.right = scissor.right_;
        s.top = scissor.top_;
        s.bottom = scissor.bottom_;
        device_->RSSetScissorRects( 1, &s );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::ClearTarget( hBool clearColour, const hColour& colour, hBool clearZ, hFloat z )    
    {
        if ( clearColour )
        {
            for ( hUint32 i = 0; i < MAX_RENDERTARGE_VIEWS; ++i )
            {
                if ( renderTargetViews_[i] )
                    device_->ClearRenderTargetView( renderTargetViews_[i], (FLOAT*)&colour );
            }
        }
        if ( clearZ )
            device_->ClearDepthStencilView( depthStencilView_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, z, 0 );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::DrawPrimitive( hUint32 nPrimatives, hUint32 start )
    {
        hUint32 verts;
        switch ( primType_ )
        {
        case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
            verts = nPrimatives / 2; break;
        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
            verts = nPrimatives * 3; break;
        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
            verts = nPrimatives + 2; break;
        }
        device_->Draw( verts, start );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::DrawIndexedPrimitive( hUint32 nPrimatives, hUint32 start )
    {
        hUint32 verts;
        switch ( primType_ )
        {
        case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
            verts = nPrimatives / 2; break;
        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
            verts = nPrimatives * 3; break;
        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
            verts = nPrimatives + 2; break;
        }
        device_->DrawIndexed( verts, start, 0 );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::DrawIndexedPrimitiveInstanced(hUint instanceCount, hUint32 nPrims, hUint startVtx) {
        hUint32 verts;
        switch ( primType_ ) {
        case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
            verts = nPrims / 2; break;
        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
            verts = nPrims * 3; break;
        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
            verts = nPrims + 2; break;
        }
        device_->DrawIndexedInstanced(verts, instanceCount, startVtx, 0, 0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    Heart::hdDX11CommandBuffer hdDX11RenderSubmissionCtx::SaveToCommandBuffer()
    {
        hdDX11CommandBuffer ret;
        //We never save/restore state
        HRESULT hr = device_->FinishCommandList( FALSE, &ret );
        hcAssert( SUCCEEDED( hr ) );
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::RunSubmissionBuffer( hdDX11CommandBuffer cmdBuf )
    {
        //We never save/restore state
        device_->ExecuteCommandList( cmdBuf, FALSE );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::Map( hdDX11Texture* tex, hUint32 level, hdDX11MappedResourceData* data )
    {
        //TODO: none dynamic resources need to allocate memory here.
        HRESULT hr;
        hcAssert( tex && data );
        hr = device_->Map( tex->dx11Texture_, level, D3D11_MAP_WRITE_DISCARD, 0, data );
        hcAssert( SUCCEEDED( hr ) );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::Map( hdDX11IndexBuffer* ib, hdDX11MappedResourceData* data )
    {
        if ( ib->flags_ & RESOURCEFLAG_DYNAMIC )
        {
            HRESULT hr;
            hcAssert( ib && ib->buffer_ && data );
            hr = device_->Map( ib->buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, data );
            hcAssert( SUCCEEDED( hr ) );
        }
        else 
        {
            data->pData = (*alloc_)( ib->dataSize_ );
            data->DepthPitch = 0;
            data->RowPitch = 0;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::Map( hdDX11VertexBuffer* vb, hdDX11MappedResourceData* data )
    {
        if ( vb->flags_ & RESOURCEFLAG_DYNAMIC )
        {
            HRESULT hr;
            hcAssert( vb && vb->buffer_ && data );
            hr = device_->Map( vb->buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, data );
            hcAssert( SUCCEEDED( hr ) );
        }
        else    
        {
            data->pData = (*alloc_)( vb->dataSize_ );
            data->DepthPitch = 0;
            data->RowPitch = 0;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::Map(hdDX11ParameterConstantBlock* cb, hdDX11MappedResourceData* data)
    {
        HRESULT hr;
        hr = device_->Map(cb->constBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, data);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::Unmap( hdDX11Texture* tex, hUint32 level, void* ptr )
    {
        device_->Unmap( tex->dx11Texture_, level );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::Unmap( hdDX11IndexBuffer* ib, void* ptr )
    {
        if ( ib->flags_ & RESOURCEFLAG_DYNAMIC )
        {
            device_->Unmap( ib->buffer_, 0 );
        }
        else 
        {
            device_->UpdateSubresource( ib->buffer_, 0, NULL, ptr, 0, 0 );
            (*free_)( ptr );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::Unmap( hdDX11VertexBuffer* vb, void* ptr )
    {
        if ( vb->flags_ & RESOURCEFLAG_DYNAMIC )
        {
            device_->Unmap( vb->buffer_, 0 );
        }
        else 
        {
            device_->UpdateSubresource( vb->buffer_, 0, NULL, ptr, 0, 0 );
            (*free_)( ptr );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::Unmap(hdDX11ParameterConstantBlock* cb, void* ptr)
    {
        device_->Unmap(cb->constBuffer_, 0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetDefaultTargets( ID3D11RenderTargetView* target, ID3D11DepthStencilView* depth )
    {
        defaultRenderView_ = target; 
        defaultDepthView_ = depth;
        renderTargetViews_[0] = defaultRenderView_;
        depthStencilView_ = defaultDepthView_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::Update( hdDX11ParameterConstantBlock* cb )
    {
        device_->UpdateSubresource(cb->constBuffer_, 0, NULL, cb->mapData_, 0, 0);
    }

}