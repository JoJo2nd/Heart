/********************************************************************

    filename:   hViews.cpp  
    
    Copyright (c) 12:3:2013 James Moran
    
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

#include "render/hViews.h"
#include "render/hTexture.h"
#include "render/hParameterConstBlock.h"

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hShaderResourceView::OnZeroRef() {
#if 0 // !!JM
        if (refTex_ && refType_!=eRenderResourceType_Buffer) {
            refTex_->DecRef();
        } else if (refCB_) {
            refCB_->DecRef();
        }
        zeroRefProc_(this);
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hShaderResourceView::bindTexture(hTexture* tex) {
#if 0 // !!JM
        if (tex) {
            tex->AddRef();
        }
        if (refTex_) {
            refTex_->DecRef();
        }
        refType_=tex->getRenderType();
        refTex_=tex;
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hShaderResourceView::bindConstBlock(hRenderBuffer* cb) {
#if 0 // !!JM
        if (cb) {
            cb->AddRef();
        }
        if (refCB_) {
            refCB_->DecRef();
        }
        refType_=eRenderResourceType_Buffer;
        refCB_=cb;
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderTargetView::OnZeroRef() {
#if 0 // !!JM
        if (refTex_) {
            refTex_->DecRef();
        }
        zeroRefProc_(this);
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderTargetView::bindTexture(hTexture* tex) {
#if 0 // !!JM
        if (refTex_) {
            refTex_->DecRef();
        }
        refTex_=tex;
        if (refTex_) {
            refTex_->AddRef();
        }
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDepthStencilView::OnZeroRef() {
#if 0 // !!JM
        if (refTex_) {
            refTex_->DecRef();
        }
        zeroRefProc_(this);
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDepthStencilView::bindTexture(hTexture* tex) {
#if 0 // !!JM
        if (refTex_) {
            refTex_->DecRef();
        }
        refTex_=tex;
        if (refTex_) {
            refTex_->AddRef();
        }
#else
        hStub();
#endif
    }

}