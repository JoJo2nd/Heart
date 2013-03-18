/********************************************************************

    filename:   hRenderStateBlock.h  
    
    Copyright (c) 14:3:2013 James Moran
    
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

#ifndef HRENDERSTATEBLOCK_H__
#define HRENDERSTATEBLOCK_H__

namespace Heart
{
    class HEART_DLLEXPORT hBlendState : public hdBlendState,
                                        public hIReferenceCounted,
                                        public hMapElement< hUint32, hBlendState >
    {
    public:
        hFUNCTOR_TYPEDEF(void(*)(hBlendState*), hZeroRefProc);
        hBlendState(hZeroRefProc zeroproc)
            : zeroProc_(zeroproc)
        {}
    private:
        void OnZeroRef() {
            zeroProc_(this);
        }
        hZeroRefProc zeroProc_;
    };

    class HEART_DLLEXPORT hRasterizerState : public hdRasterizerState,
                                             public hIReferenceCounted,
                                             public hMapElement< hUint32, hRasterizerState >
    {
    public:
        hFUNCTOR_TYPEDEF(void(*)(hRasterizerState*), hZeroRefProc);
        hRasterizerState(hZeroRefProc zeroproc)
            : zeroProc_(zeroproc)
        {}
    private:
        void OnZeroRef() {
            zeroProc_(this);
        }
        hZeroRefProc zeroProc_;
    };

    struct HEART_DLLEXPORT hDepthStencilState : public hdDepthStencilState,
                                                public hIReferenceCounted,
                                                public hMapElement< hUint32, hDepthStencilState >
    {
    public:
        hFUNCTOR_TYPEDEF(void(*)(hDepthStencilState*), hZeroRefProc);
        hDepthStencilState(hZeroRefProc zeroproc)
            : zeroProc_(zeroproc)
        {}
    private:
        void OnZeroRef() {
            zeroProc_(this);
        }
        hZeroRefProc zeroProc_;
    };

    struct HEART_DLLEXPORT hSamplerState : public hdSamplerState,
                                           public hIReferenceCounted,
                                           public hMapElement< hUint32, hSamplerState >
    {
    public:
        hFUNCTOR_TYPEDEF(void(*)(hSamplerState*), hZeroRefProc);
        hSamplerState(hZeroRefProc zeroproc)
            : zeroProc_(zeroproc)
        {}
    private:
        void OnZeroRef() {
            zeroProc_(this);
        }
        hZeroRefProc zeroProc_;
    };
}

#endif // HRENDERSTATEBLOCK_H__