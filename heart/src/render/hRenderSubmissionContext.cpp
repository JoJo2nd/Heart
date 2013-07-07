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

    void hRenderSubmissionCtx::SetMaterialPass(hMaterialTechniquePass* pass)
    {
        impl_.setRenderStateBlock(pass->GetDepthStencilState());
        impl_.setRenderStateBlock(pass->GetBlendState());
        impl_.setRenderStateBlock(pass->GetRasterizerState());
        impl_.SetRenderInputObject(pass->GetRenderInputObject());
        impl_.SetInputStreams(pass->getRenderStreamsObject());
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

    void hRenderSubmissionCtx::SetScissorRect( const hScissorRect& scissor )
    {
        impl_.SetScissorRect( scissor );
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

    void hRenderSubmissionCtx::DrawIndexedPrimitiveInstanced(hUint instanceCount, hUint32 nPrimatives, hUint32 startVertex) {
        impl_.DrawIndexedPrimitiveInstanced(instanceCount, nPrimatives, startVertex);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::runCommandBuffer( hdRenderCommandBuffer cmdBuf )
    {
        impl_.RunSubmissionBuffer( cmdBuf );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Map(hIndexBuffer* ib, hIndexBufferMapInfo* outInfo)
    {
        hdMappedData md;
        impl_.Map(ib, &md);
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
        impl_.Map(vb, &md);
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
        impl_.Map(tex, level, &md);
        outInfo->tex_ = tex;
        outInfo->ptr_ = md.pData;
        outInfo->pitch_ = md.RowPitch;
        outInfo->level_ = level;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Map(hParameterConstantBlock* cb, hConstBlockMapInfo* outInfo)
    {
        hdMappedData md;
        impl_.Map(cb, &md);
        outInfo->ptr = md.pData;
        outInfo->cb = cb;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Unmap( hIndexBufferMapInfo* outInfo )
    {
        impl_.Unmap(outInfo->ib_, outInfo->ptr_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Unmap( hVertexBufferMapInfo* outInfo )
    {
        impl_.Unmap(outInfo->vb_, outInfo->ptr_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Unmap(hTextureMapInfo* outInfo)
    {
        impl_.Unmap( outInfo->tex_, outInfo->level_, outInfo->ptr_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Unmap(hConstBlockMapInfo* info)
    {
        impl_.Unmap(info->cb, info->ptr);
        info->ptr = NULL;
        info->cb = NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::Initialise( hRenderer* renderer )
    {
        hUint32 size = sizeof( hInstanceConstants );
        renderer_ = renderer;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderSubmissionCtx::~hRenderSubmissionCtx()
    {
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::setTargets(hUint32 n, hRenderTargetView** targets, hDepthStencilView* depth) {
        hdRenderTargetView* dtargets[HEART_MAX_SIMULTANEOUS_RENDER_TARGETS];
        for (hUint i=0; i<n; ++i) {
            dtargets[i]=targets[i];
        }
        impl_.setTargets(n, dtargets, depth);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::clearColour(hRenderTargetView* view, const hColour& colour) {
        hcAssert(view);
        impl_.clearColour(view, colour);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::clearDepth(hDepthStencilView* view, hFloat z) {
        hcAssert(view);
        impl_.clearDepth(view, z);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderSubmissionCtx::runRenderCommands(hRCmd* cmds) {
        hRCmd* cmd=cmds;
        for(;;) {
            switch (cmd->opCode_) {
            case eRenderCmd_NOOP: break;
            case eRenderCmd_Jump: {
                hRCmdJump* jump=static_cast<hRCmdJump*>(cmd);
                runRenderCommands(jump->cmd_);
            } break;
            case eRenderCmd_Return: {
                return;
            };
            case eRenderCmd_SetRenderStates: {
                hRCmdSetStates* states=static_cast<hRCmdSetStates*>(cmd);
                impl_.setRenderStates(states);
            } break;
            case eRenderCmd_SetVertexShader: {
                hRCmdSetVertexShader* setvs=static_cast<hRCmdSetVertexShader*>(cmd);
                impl_.setVertexShader(setvs);
            } break;
            case eRenderCmd_SetPixelShader: {
                hRCmdSetPixelShader* setps=static_cast<hRCmdSetPixelShader*>(cmd);
                impl_.setPixelShader(setps);
            } break;
            case eRenderCmd_SetGeometeryShader: {
                hRCmdSetGeometryShader* setps=static_cast<hRCmdSetGeometryShader*>(cmd);
                impl_.setGeometryShader(setps);
            } break;
            case eRenderCmd_SetHullShader: {
                hRCmdSetHullShader* setps=static_cast<hRCmdSetHullShader*>(cmd);
                impl_.setHullShader(setps);
            } break;
            case eRenderCmd_SetDomainShader: {
                hRCmdSetDomainShader* setps=static_cast<hRCmdSetDomainShader*>(cmd);
                impl_.setDomainShader(setps);
            } break;
            case eRenderCmd_SetVertexInputs: {
                hRCmdSetVertexInputs* setvsi=static_cast<hRCmdSetVertexInputs*>(cmd);
                impl_.setVertexInputs(setvsi);
            } break;
            case eRenderCmd_SetPixelInputs: {
                hRCmdSetPixelInputs* setvsi=static_cast<hRCmdSetPixelInputs*>(cmd);
                impl_.setPixelInputs(setvsi);
            } break;
            case eRenderCmd_SetGeometryInputs: {
                hRCmdSetGeometryInputs* setvsi=static_cast<hRCmdSetGeometryInputs*>(cmd);
                impl_.setGeometryInputs(setvsi);
            } break;
            case eRenderCmd_SetHullInputs: {
                hRCmdSetHullInputs* setvsi=static_cast<hRCmdSetHullInputs*>(cmd);
                impl_.setHullInputs(setvsi);
            } break;
            case eRenderCmd_SetDomainInputs: {
                hRCmdSetDomainInputs* setvsi=static_cast<hRCmdSetDomainInputs*>(cmd);
                impl_.setDomainInputs(setvsi);
            } break;
            case eRenderCmd_SetInputStreams: {
                hRCmdSetInputStreams* set=static_cast<hRCmdSetInputStreams*>(cmd);
                impl_.setInputStreams(set);
            } break;
            case eRenderCmd_Draw: {
                hRCmdDraw* draw=static_cast<hRCmdDraw*>(cmd);
                impl_.DrawPrimitive(draw->nPrimatives_, draw->startVertex_);
            } break;
            case eRenderCmd_DrawIndex: {
                hRCmdDrawIndex* draw=static_cast<hRCmdDrawIndex*>(cmd);
                impl_.DrawIndexedPrimitive(draw->nPrimatives_, draw->startVertex_);
            } break;
            case eRenderCmd_DrawInstanced: {
                hRCmdDrawInstanced* draw=static_cast<hRCmdDrawInstanced*>(cmd);
                impl_.DrawPrimitiveInstanced(draw->nPrimatives_, draw->startVertex_, draw->instanceCount_);
            } break;
            case eRenderCmd_DrawInstancedIndex: {
                hRCmdDrawInstancedIndex* draw=static_cast<hRCmdDrawInstancedIndex*>(cmd);
            } break;
            default: hcAssertFailMsg("Invalid Render Cmd");
            }
            cmd=(hRCmd*)((hByte*)cmd+cmd->size_);
        }
    }

}