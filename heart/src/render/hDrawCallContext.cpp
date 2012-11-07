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
        renderer_ = renderer;
        SetScissor(0.f, 0.f, renderer_->GetWidth(), renderer_->GetHeight());
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
        if (calls_ == MAX_DC) Flush();
        //TODO: support passes by looping here?
        hDrawCall* dc = &dcs_[calls_++];
        dc->sortKey_        = hBuildRenderSortKey(camID_, layerID_, transparent, vsDepth, mat->GetMatKey(), 0);
        dc->matInstance_    = mat;
        dc->indexBuffer_    = ib;
        dc->vertexBuffer_[0] = vb;
        dc->vertexBuffer_[1] = NULL;
        dc->vertexBuffer_[2] = NULL;
        dc->vertexBuffer_[3] = NULL;
        dc->vertexBuffer_[4] = NULL;
        dc->scissor_        = scissor_;
        dc->primCount_      = primCount;
        dc->startVertex_    = startVtx;
        dc->primType_       = type;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDrawCallContext::End()
    {
        Flush();
        renderer_ = NULL;
        calls_ = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDrawCallContext::Flush()
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