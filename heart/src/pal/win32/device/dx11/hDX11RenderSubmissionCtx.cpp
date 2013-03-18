
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

    hBool hdDX11RenderInputObject::bindShaderProgram(hdDX11ShaderProgram* prog)
    {
        if (!prog) return false;
        switch(prog->type_)
        {
        case ShaderType_FRAGMENTPROG: {
            pixelShader_ = prog->pixelShader_; 
            boundProgs_[hdDX11PixelProg] = prog;
        } break;
        case ShaderType_VERTEXPROG: {
            vertexShader_ = prog->vertexShader_; 
            boundProgs_[hdDX11VertexProg] = prog;
        } break;
        case ShaderType_GEOMETRYPROG: {
            geometryShader_=prog->geomShader_;
            boundProgs_[hdDX11GemoProg]=prog;
        } break;
        case ShaderType_HULLPROG: {
            hullShader_=prog->hullShader_;
            boundProgs_[hdDX11HullProg]=prog;
        } break;
        case ShaderType_DOMAINPROG: {
            domainShader_=prog->domainShader_;
            boundProgs_[hdDX11DomainProg]=prog;
        } break;
        default: return false; // Assert here?
        }
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdDX11RenderInputObject::bindConstantBuffer(hShaderParameterID paramID, hdDX11ParameterConstantBlock* buffer)
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

    hBool hdDX11RenderInputObject::bindSamplerInput(hShaderParameterID paramID, hdDX11SamplerState* ss)
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

    hBool hdDX11RenderInputObject::bindResourceView(hShaderParameterID paramID, hdDX11ShaderResourceView* view)
    {
        hUint32 idx;
        hBool succ = false;

        for (hUint32 p = 0; p < hdDX11ProgMax; ++p) {
            if (!boundProgs_[p]) continue;
            idx = boundProgs_[p]->GetInputRegister(paramID);
            if (idx > HEART_MAX_RESOURCE_INPUTS) continue;
            inputData_[p].resourceViews_[idx] = view ? view->srv_ : NULL;
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
        streams_[stream] = vertexbuffer ? vertexbuffer->buffer_ : NULL;
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
        
        prog->destroyVertexLayout(layout_);
        hdInputLayout* vtxlo=prog->createVertexLayout(desc, descn);
        if (vtxlo) {
            layout_=vtxlo;
        }else{
            layout_=NULL;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderStreamsObject::unbind(hdDX11ShaderProgram* prog) {
        if (!prog) return;
        prog->destroyVertexLayout(layout_);
        layout_=NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetInputStreams(const hdDX11RenderStreamsObject* streams)
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

    void hdDX11RenderSubmissionCtx::SetRenderInputObject(const hdRenderInputObject* inputobj)
    {
        device_->VSSetShader(inputobj->vertexShader_, NULL, 0);
        if (inputobj->vertexShader_) {
            const hdRenderInputObject::RendererInputs* inputs = inputobj->inputData_+hdRenderInputObject::hdDX11VertexProg;
            device_->VSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, inputs->programInputs_);
            // Putting if guards here reduced GPU time on test scene dramatically (x3-x4 faster)
            // Seems that setting textures/shader resources on VS is expensive (ATI chips)
            // however PS it seems super cheap. Common case optimization?
            if (inputs->samplerCount_) {
                device_->VSSetSamplers(0, inputs->samplerCount_, inputs->samplerState_);
            }
            if (inputs->resourceViewCount_) {
                device_->VSSetShaderResources(0, inputs->resourceViewCount_, inputs->resourceViews_);
            }
        }

        device_->PSSetShader(inputobj->pixelShader_, NULL, 0);
        if (inputobj->pixelShader_) {
            const hdRenderInputObject::RendererInputs* inputs = inputobj->inputData_+hdRenderInputObject::hdDX11PixelProg;
            device_->PSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, inputs->programInputs_);
            device_->PSSetSamplers(0, inputs->samplerCount_, inputs->samplerState_);
            device_->PSSetShaderResources(0, inputs->resourceViewCount_, inputs->resourceViews_);
        }

        device_->GSSetShader(inputobj->geometryShader_, NULL, 0);
        if (inputobj->geometryShader_) {
            const hdRenderInputObject::RendererInputs* inputs = inputobj->inputData_+hdRenderInputObject::hdDX11GemoProg;
            device_->GSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, inputs->programInputs_);
            device_->GSSetSamplers(0, inputs->samplerCount_, inputs->samplerState_);
            device_->GSSetShaderResources(0, inputs->resourceViewCount_, inputs->resourceViews_);
        }

        device_->HSSetShader(inputobj->hullShader_, NULL, 0);
        if (inputobj->hullShader_) {
            const hdRenderInputObject::RendererInputs* inputs = inputobj->inputData_+hdRenderInputObject::hdDX11HullProg;
            device_->HSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, inputs->programInputs_);
            device_->HSSetSamplers(0, inputs->samplerCount_, inputs->samplerState_);
            device_->HSSetShaderResources(0, inputs->resourceViewCount_, inputs->resourceViews_);
        }

        device_->DSSetShader(inputobj->domainShader_, NULL, 0);
        if (inputobj->domainShader_) {
            const hdRenderInputObject::RendererInputs* inputs = inputobj->inputData_+hdRenderInputObject::hdDX11DomainProg;
            device_->DSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, inputs->programInputs_);
            device_->DSSetSamplers(0, inputs->samplerCount_, inputs->samplerState_);
            device_->DSSetShaderResources(0, inputs->resourceViewCount_, inputs->resourceViews_);
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

    void hdDX11RenderSubmissionCtx::setComputeInput(const hdDX11ComputeInputObject* ci) {
        hcAssert(ci);
        device_->CSSetShader(ci->computeShader_, NULL, 0);
        if (ci->computeShader_) {
            device_->CSSetConstantBuffers(0, ci->constCount_, ci->programInputs_);
            device_->CSSetSamplers(0, ci->samplerCount_, ci->samplerState_);
            device_->CSSetShaderResources(0, ci->resourceViewCount_, ci->resourceViews_);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::setTargets(hUint32 n, hdDX11RenderTargetView** target, hdDX11DepthStencilView* depth) {
        ID3D11RenderTargetView* rtviews[HEART_MAX_SIMULTANEOUS_RENDER_TARGETS];
        void* nullout[HEART_MAX_RESOURCE_INPUTS] = {0};
        for (hUint i=0; i<n; ++i) {
            rtviews[i]=target[i] ? target[i]->rtv_ : NULL;
        }
        device_->PSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, (ID3D11Buffer**)&nullout);
        device_->PSSetSamplers(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11SamplerState**)&nullout);
        device_->PSSetShaderResources(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11ShaderResourceView**)&nullout);
        device_->VSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, (ID3D11Buffer**)&nullout);
        device_->VSSetSamplers(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11SamplerState**)&nullout);
        device_->VSSetShaderResources(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11ShaderResourceView**)&nullout);
        device_->GSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, (ID3D11Buffer**)&nullout);
        device_->GSSetSamplers(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11SamplerState**)&nullout);
        device_->GSSetShaderResources(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11ShaderResourceView**)&nullout);
        device_->DSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, (ID3D11Buffer**)&nullout);
        device_->DSSetSamplers(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11SamplerState**)&nullout);
        device_->DSSetShaderResources(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11ShaderResourceView**)&nullout);
        device_->HSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, (ID3D11Buffer**)&nullout);
        device_->HSSetSamplers(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11SamplerState**)&nullout);
        device_->HSSetShaderResources(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11ShaderResourceView**)&nullout);
        device_->CSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, (ID3D11Buffer**)&nullout);
        device_->CSSetSamplers(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11SamplerState**)&nullout);
        device_->CSSetShaderResources(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11ShaderResourceView**)&nullout);
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

    hBool hdDX11ComputeInputObject::bindConstantBuffer(hShaderParameterID paramID, hdDX11ParameterConstantBlock* buffer) {
        if (computeShader_) {
            hUint32 idx = boundComputeProg_->GetInputRegister(paramID);
            if (idx > HEART_MAX_RESOURCE_INPUTS) {
                return hFalse;
            }
            programInputs_[idx] = buffer->constBuffer_;
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