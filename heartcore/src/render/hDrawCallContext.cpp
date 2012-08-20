/********************************************************************

	filename: 	hDrawCallContext.cpp	
	
	Copyright (c) 6:8:2012 James Moran
	
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

    void hDrawCallContext::Begin( hRenderer* renderer )
    {
        hcAssertMsg(renderer_ == NULL && calls_ == 0, "hDrawCallContext begin called without end");
        SetScissor(0.f, 0.f, 1.f, 1.f);
        renderer_ = renderer;
        calls_ = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDrawCallContext::SubmitDrawCall( hMaterialInstance* mat, 
                                           hIndexBuffer* ib, 
                                           hVertexBuffer* vb,
                                           hUint32 primCount, 
                                           hBool transparent /*= hFalse*/,
                                           hFloat vsDepth /*= 0.f*/,
                                           hUint32 startVtx /*= 0*/, 
                                           PrimitiveType type /*= PRIMITIVETYPE_TRILIST*/ )
    {
        if (calls_ == MAX_DC) Submit();
        //TODO: support passes by looping here
        hDrawCall* dc = &dcs_[calls_++];
        dc->sortKey_        = hBuildRenderSortKey(camID_, layerID_, transparent, vsDepth, mat->GetMatKey(), 0);
        dc->matInstance_    = mat;
        dc->indexBuffer_    = ib;
        dc->vertexBuffer_   = vb;
        dc->scissor_        = scissor_;
        dc->primCount_      = primCount;
        dc->startVertex_    = startVtx;
        dc->stride_         = vb->GetStride();
        dc->primType_       = type;
        dc->immediate_      = hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDrawCallContext::SubmitDrawCallInline( hMaterialInstance* mat, 
                                                 hByte* ibOut, hUint32 ibSize,
                                                 hByte* vbOut, hUint32 vbSize,
                                                 hUint16 primCount, 
                                                 hUint16 stride,
                                                 hBool transparent /*= hFalse*/,
                                                 hFloat vsDepth /*= 0.f*/,
                                                 PrimitiveType type /*= PRIMITIVETYPE_TRILIST*/ )
    {
        if (calls_ == MAX_DC) Submit();
        hDrawCall* dc = &dcs_[calls_++];
        dc->sortKey_        = hBuildRenderSortKey(camID_, layerID_, transparent, vsDepth, mat->GetMatKey(), 0);
        dc->matInstance_    = mat;
        dc->imIBBuffer_     = (hByte*)renderer_->AllocTempRenderMemory(ibSize);
        dc->ibSize_         = ibSize;
        dc->imVBBuffer_     = (hByte*)renderer_->AllocTempRenderMemory(vbSize);
        dc->vbSize_         = vbSize;
        dc->primCount_      = primCount;
        dc->startVertex_    = 0;
        dc->scissor_        = scissor_;
        dc->stride_         = stride;
        dc->primType_       = type;
        dc->immediate_      = hTrue;

        if (ibOut) hMemCpy(dc->imIBBuffer_, ibOut, ibSize);
        hMemCpy(dc->imVBBuffer_, vbOut, vbSize);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDrawCallContext::End()
    {
        Submit();
        renderer_ = NULL;
        calls_ = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDrawCallContext::Submit()
    {
        if (calls_)
            renderer_->SubmitDrawCallBlock(dcs_.GetBuffer(), calls_);
        calls_ = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* hDrawCallContext::Map( hVertexBuffer* vb, hUint32 size )
    {
        void* ret = renderer_->AllocTempRenderMemory(size);
        hRenderResourceUpdateCmd cmd;
        cmd.flags_ = hRenderResourceUpdateCmd::eMapTypeVtxBuffer;
        cmd.size_ = size;
        cmd.data_ = ret;
        cmd.vb_ = vb;
        renderer_->SumbitResourceUpdateCommand(cmd);
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* hDrawCallContext::Map( hIndexBuffer* ib, hUint32 size )
    {
        void* ret = renderer_->AllocTempRenderMemory(size);
        hRenderResourceUpdateCmd cmd;
        cmd.flags_ = hRenderResourceUpdateCmd::eMapTypeIdxBuffer;
        cmd.size_ = size;
        cmd.data_ = ret;
        cmd.ib_ = ib;
        renderer_->SumbitResourceUpdateCommand(cmd);
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDrawCallContext::SetScissor(hUint32 left, hUint32 top, hUint32 right, hUint32 bottom)
    {
        scissor_.left_ = left;
        scissor_.top_ = top;
        scissor_.right_ = right;
        scissor_.bottom_ = bottom;
    }

}