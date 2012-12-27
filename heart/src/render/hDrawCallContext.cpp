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
}