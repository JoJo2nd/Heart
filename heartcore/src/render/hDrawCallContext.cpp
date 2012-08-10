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
        Submit();
        //TODO: support passes by looping here
        hDrawCall* dc = dcs_[calls_++];
        dc->sortKey_        = hBuildRenderSortKey(camID_, layerID_, transparent, vsDepth, mat->GetMatKey(), 0);
        dc->matInstance_    = mat;
        dc->indexBuffer_    = ib;
        dc->vertexBuffer_   = vb;
        dc->primCount_      = primCount;
        dc->startVertex_    = startVtx;
        dc->primType_       = type;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDrawCallContext::SubmitDrawCallInline( hMaterialInstance* mat, 
                                                 hByte** ibOut, hUint32 ibSize,
                                                 hByte** vbOut, hUint32 vbSize,
                                                 hUint16** primCount, 
                                                 hBool transparent /*= hFalse*/,
                                                 hFloat vsDepth /*= 0.f*/,
                                                 PrimitiveType type /*= PRIMITIVETYPE_TRILIST*/ )
    {
        Submit();
        hDrawCall* dc = dcs_[calls_++];
        *ibOut = (hByte*)renderer_->AllocTempRenderMemory(ibSize);
        *vbOut = (hByte*)renderer_->AllocTempRenderMemory(vbSize);
        *primCount = &dc->primCount_;
        dc->sortKey_        = hBuildRenderSortKey(camID_, layerID_, transparent, vsDepth, mat->GetMatKey(), 0);
        dc->matInstance_    = mat;
        dc->imIBBuffer_     = *ibOut;
        dc->ibSize_         = ibSize;
        dc->imVBBuffer_     = *vbOut;
        dc->vbSize_         = vbSize;
        dc->primCount_      = 0;//to be set later by caller
        dc->startVertex_    = 0;
        dc->primType_       = type;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDrawCallContext::SubmitDrawCallInline( hMaterialInstance* mat, 
                                                 hByte** vbOut, hUint32 vbSize,
                                                 hUint16** primCount, 
                                                 hBool transparent /*= hFalse*/,
                                                 hFloat vsDepth /*= 0.f*/,
                                                 PrimitiveType type /*= PRIMITIVETYPE_TRILIST*/ )
    {
        Submit();
        hDrawCall* dc = dcs_[calls_++];
        *vbOut = (hByte*)renderer_->AllocTempRenderMemory(vbSize);
        *primCount = &dc->primCount_;
        dc->sortKey_        = hBuildRenderSortKey(camID_, layerID_, transparent, vsDepth, mat->GetMatKey(), 0);
        dc->matInstance_    = mat;
        dc->imIBBuffer_     = NULL;
        dc->ibSize_         = 0;
        dc->imVBBuffer_     = *vbOut;
        dc->vbSize_         = vbSize;
        dc->primCount_      = 0;//to be set later by caller
        dc->startVertex_    = 0;
        dc->primType_       = type;
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
        if (calls_ == MAX_DC)
        {
            renderer_->SubmitDrawCallBlock(dcs_->GetBuffer(), calls_);
        }
    }

}