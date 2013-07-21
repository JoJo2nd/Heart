
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

    void hdDX11RenderSubmissionCtx::setRenderStateBlock( hdDX11BlendState* st )
    {
        hFloat factors[4] = { 1.f, 1.f, 1.f, 1.f };
        device_->OMSetBlendState( st->stateObj_, factors, ~0U );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setRenderStateBlock( hdDX11DepthStencilState* st )
    {
        device_->OMSetDepthStencilState( st->stateObj_, st->stencilRef_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setRenderStateBlock( hdDX11RasterizerState* st )
    {
        device_->RSSetState( st->stateObj_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setComputeInput(const hdDX11ComputeInputObject* ci) {
        hcAssert(ci);
        device_->CSSetShader(ci->computeShader_, NULL, 0);
        if (ci->computeShader_) {
            device_->CSSetConstantBuffers(0, ci->constCount_, ci->programInputs_);
            device_->CSSetSamplers(0, ci->samplerCount_, ci->samplerState_);
            device_->CSSetShaderResources(0, ci->resourceViewCount_, ci->resourceViews_);
            device_->CSSetUnorderedAccessViews(0, ci->uavCount_, ci->unorderdAccessView_, NULL);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setTargets(hUint32 n, hdDX11RenderTargetView** target, hdDX11DepthStencilView* depth) {
        ID3D11RenderTargetView* rtviews[HEART_MAX_SIMULTANEOUS_RENDER_TARGETS];
        for (hUint i=0; i<n; ++i) {
            rtviews[i]=target[i] ? target[i]->rtv_ : NULL;
        }
        hdDX11RenderDevice::clearDeviceInputs(device_);
        device_->OMSetRenderTargets(n, rtviews, depth ? depth->dsv_ : NULL);
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

    void hdDX11RenderSubmissionCtx::setViewPixel(hUint slot, hdDX11ShaderResourceView* buffer) {
        ID3D11ShaderResourceView* srv=buffer ? buffer->srv_ : hNullptr;
        device_->PSSetShaderResources(0, 1, &srv);
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
            verts = nPrimatives*2; break;
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

    void hdDX11RenderSubmissionCtx::DrawPrimitiveInstanced(hUint instanceCount, hUint32 nPrimatives, hUint32 startVertex) {
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
        device_->DrawInstanced(verts, instanceCount, startVertex, 0);
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

    void hdDX11RenderSubmissionCtx::dispatch(hUint x, hUint y, hUint z) {
        device_->Dispatch(x, y, z);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11CommandBuffer hdDX11RenderSubmissionCtx::SaveToCommandBuffer()
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

    void hdDX11RenderSubmissionCtx::Map(hdDX11Buffer* cb, hdDX11MappedResourceData* data)
    {
        HRESULT hr;
        hr = device_->Map(cb->buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, data);
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

    void hdDX11RenderSubmissionCtx::Unmap(hdDX11Buffer* cb, void* ptr)
    {
        device_->Unmap(cb->buffer_, 0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::clearColour(hdDX11RenderTargetView* target, const hColour& colour) {
        hcAssert(target);
        device_->ClearRenderTargetView(target->rtv_,  (FLOAT*)&colour);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::clearDepth(hdDX11DepthStencilView* view, hFloat z) {
        hcAssert(view);
        device_->ClearDepthStencilView(view->dsv_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, z, 0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setVertexShader(hRCmdSetVertexShader* prog) {
        device_->VSSetShader(prog->shader_, hNullptr, 0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setPixelShader(hRCmdSetPixelShader* prog) {
        device_->PSSetShader(prog->shader_, hNullptr, 0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setGeometryShader(hRCmdSetGeometryShader* prog) {
        device_->GSSetShader(prog->shader_, hNullptr, 0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setHullShader(hRCmdSetHullShader* prog) {
        device_->HSSetShader(prog->shader_, hNullptr, 0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setDomainShader(hRCmdSetDomainShader* prog) {
        device_->DSSetShader(prog->shader_, hNullptr, 0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setVertexInputs(hRCmdSetVertexInputs* cmd) {
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(cmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+cmd->resourceViewCount_);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+cmd->samplerCount_);
        if (cmd->resourceViewCount_) {
            device_->VSSetShaderResources(0, cmd->resourceViewCount_, cmdsrv);
        }
        if (cmd->samplerCount_) {
            device_->VSSetSamplers(0, cmd->samplerCount_, cmdsamp);
        }
        if (cmd->bufferCount_) {
            device_->VSSetConstantBuffers(0, cmd->bufferCount_, cmdpcb);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setPixelInputs(hRCmdSetPixelInputs* cmd) {
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(cmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+cmd->resourceViewCount_);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+cmd->samplerCount_);
        if (cmd->resourceViewCount_) {
            device_->PSSetShaderResources(0, cmd->resourceViewCount_, cmdsrv);
        }
        if (cmd->samplerCount_) {
            device_->PSSetSamplers(0, cmd->samplerCount_, cmdsamp);
        }
        if (cmd->bufferCount_) {
            device_->PSSetConstantBuffers(0, cmd->bufferCount_, cmdpcb);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setGeometryInputs(hRCmdSetGeometryInputs* cmd) {
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(cmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+cmd->resourceViewCount_);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+cmd->samplerCount_);
        if (cmd->resourceViewCount_) {
            device_->GSSetShaderResources(0, cmd->resourceViewCount_, cmdsrv);
        }
        if (cmd->samplerCount_) {
            device_->GSSetSamplers(0, cmd->samplerCount_, cmdsamp);
        }
        if (cmd->bufferCount_) {
            device_->GSSetConstantBuffers(0, cmd->bufferCount_, cmdpcb);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setHullInputs(hRCmdSetHullInputs* cmd) {
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(cmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+cmd->resourceViewCount_);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+cmd->samplerCount_);
        if (cmd->resourceViewCount_) {
            device_->HSSetShaderResources(0, cmd->resourceViewCount_, cmdsrv);
        }
        if (cmd->samplerCount_) {
            device_->HSSetSamplers(0, cmd->samplerCount_, cmdsamp);
        }
        if (cmd->bufferCount_) {
            device_->HSSetConstantBuffers(0, cmd->bufferCount_, cmdpcb);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setDomainInputs(hRCmdSetDomainInputs* cmd) {
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(cmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+cmd->resourceViewCount_);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+cmd->samplerCount_);
        if (cmd->resourceViewCount_) {
            device_->DSSetShaderResources(0, cmd->resourceViewCount_, cmdsrv);
        }
        if (cmd->samplerCount_) {
            device_->DSSetSamplers(0, cmd->samplerCount_, cmdsamp);
        }
        if (cmd->bufferCount_) {
            device_->DSSetConstantBuffers(0, cmd->bufferCount_, cmdpcb);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setRenderStates(hRCmdSetStates* cmd) {
        hFloat factors[4] = { 1.f, 1.f, 1.f, 1.f };
        device_->OMSetBlendState( cmd->blendState_, factors, ~0U );
        device_->OMSetDepthStencilState( cmd->depthState_, cmd->stencilRef_ );
        device_->RSSetState( cmd->rasterState_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setInputStreams(hRCmdSetInputStreams* cmd) {
        hUint streamcount=cmd->lastStream_-cmd->firstStream_;
        ID3D11Buffer** vbs=(ID3D11Buffer**)(cmd+1);
        UINT* strides=(UINT*)(vbs+streamcount);
        UINT offsets[HEART_MAX_INPUT_STREAMS] = {0};
        if (primType_ != cmd->topology_) {
            device_->IASetPrimitiveTopology(cmd->topology_);
        }
        device_->IASetInputLayout(cmd->layout_);
        device_->IASetIndexBuffer(cmd->index_, cmd->indexFormat_, 0);
        device_->IASetVertexBuffers(
            cmd->firstStream_,
            streamcount,
            vbs,
            strides,
            offsets);
        primType_ = cmd->topology_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdDX11ComputeInputObject::bindShaderProgram(hdDX11ShaderProgram* prog) {
        if (prog) {
            boundComputeProg_=prog;
            computeShader_=prog->computeShader_;
        } else {
            boundComputeProg_=NULL;
            computeShader_=NULL;
        }
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdDX11ComputeInputObject::bindSamplerInput(hShaderParameterID paramID, hdDX11SamplerState* srv) {
        if (computeShader_) {
            hUint32 idx = boundComputeProg_->GetInputRegister(paramID);
            if (idx > HEART_MAX_RESOURCE_INPUTS) {
                return hFalse;
            }
            samplerState_[idx] = srv->stateObj_;
            samplerCount_ = (hUint16)hMax(idx+1, samplerCount_);
            return hTrue;
        }
        return hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdDX11ComputeInputObject::bindResourceView(hShaderParameterID paramID, hdDX11ShaderResourceView* view) {
        if (computeShader_) {
            hUint32 idx = boundComputeProg_->GetInputRegister(paramID);
            if (idx > HEART_MAX_RESOURCE_INPUTS) {
                return hFalse;
            }
            resourceViews_[idx] = view->srv_;
            resourceViewCount_ = (hUint16)hMax(idx+1, resourceViewCount_);
            return hTrue;
        }
        return hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdDX11ComputeInputObject::bindConstantBuffer(hShaderParameterID paramID, hdDX11Buffer* buffer) {
        if (computeShader_) {
            hUint32 idx = boundComputeProg_->GetInputRegister(paramID);
            if (idx > HEART_MAX_RESOURCE_INPUTS) {
                return hFalse;
            }
            programInputs_[idx] = buffer->buffer_;
            constCount_ = (hUint16)hMax(idx+1, constCount_);
            return hTrue;
        }
        return hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdDX11ComputeInputObject::bindUAV( hShaderParameterID paramID, hdDX11ComputeUAV* uav )
    {
        if (computeShader_) {
            hUint32 idx = boundComputeProg_->GetInputRegister(paramID);
            if (idx > HEART_MAX_RESOURCE_INPUTS) {
                return hFalse;
            }
            unorderdAccessView_[idx] = uav ? uav->uav_ : NULL;
            uavCount_ = (hUint16)hMax(idx+1, uavCount_);
            return hTrue;
        }
        return hFalse;
    }

}