/********************************************************************

    filename:   hViews.h  
    
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
#pragma once

#ifndef HVIEWS_H__
#define HVIEWS_H__

namespace Heart
{

class hTexture;
class hRenderBuffer;

hFUNCTOR_TYPEDEF(void(*)(class hShaderResourceView*), hShaderResourceViewZeroRefProc);
class HEART_DLLEXPORT hShaderResourceView : public hIReferenceCounted,
                                            public hdShaderResourceView
{
public:
    hShaderResourceView(hShaderResourceViewZeroRefProc relproc)
        : zeroRefProc_(relproc)
        , refType_(eRenderResourceType_Invalid)
        , refTex_(NULL)
        //, refCB_(NULL)
    {
    }

private:
    friend class hRenderer;

    HEART_PRIVATE_COPY(hShaderResourceView);

    void bindTexture(hTexture* tex);
    void bindConstBlock(hRenderBuffer* cb);
    virtual void OnZeroRef();

    hShaderResourceViewZeroRefProc zeroRefProc_;
    hRenderResourceType refType_;
    union {
        hTexture*                   refTex_;
        hRenderBuffer*    refCB_;
    };
};

hFUNCTOR_TYPEDEF(void(*)(class hRenderTargetView*), hRenderTargetViewZeroRefProc);
class HEART_DLLEXPORT hRenderTargetView : public hIReferenceCounted,
                                          public hdRenderTargetView
{
public:
    hRenderTargetView(hRenderTargetViewZeroRefProc zeroproc)
        : zeroRefProc_(zeroproc)
        , refTex_(NULL)
    {
    }

    const hTexture* getTarget() const { return refTex_; }

private:
    friend class hRenderer;

    HEART_PRIVATE_COPY(hRenderTargetView);

    void bindTexture(hTexture* tex);
    virtual void OnZeroRef();

    hRenderTargetViewZeroRefProc    zeroRefProc_;
    hTexture*       refTex_;
};

hFUNCTOR_TYPEDEF(void(*)(class hDepthStencilView*), hDepthStencilViewZeroRefProc);
class HEART_DLLEXPORT hDepthStencilView : public hIReferenceCounted,
                                          public hdDepthStencilView
{
public:
    hDepthStencilView(hDepthStencilViewZeroRefProc zeroproc)
        : zeroRefProc_(zeroproc)
        , refTex_(NULL)
    {
    }

private:
    friend class hRenderer;

    HEART_PRIVATE_COPY(hDepthStencilView);

    void bindTexture(hTexture* tex);
    virtual void OnZeroRef();

    hDepthStencilViewZeroRefProc    zeroRefProc_;
    hTexture*       refTex_;
};
}

#endif // HVIEWS_H__