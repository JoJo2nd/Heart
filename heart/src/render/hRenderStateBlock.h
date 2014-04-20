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
    hFUNCTOR_TYPEDEF(void(*)(class hBlendState*), hBlendZeroRefProc);
    class HEART_DLLEXPORT hBlendState : public hdBlendState,
                                        public hIReferenceCounted,
                                        public hMapElement< hUint32, hBlendState >
    {
    public:
        hBlendState(hBlendZeroRefProc zeroproc)
            : zeroProc_(zeroproc)
        {}
    private:
        void OnZeroRef() {
            zeroProc_(this);
        }
        hBlendZeroRefProc zeroProc_;
    };

    hFUNCTOR_TYPEDEF(void(*)(class hRasterizerState*), hRasterizerZeroRefProc);
    class HEART_DLLEXPORT hRasterizerState : public hdRasterizerState,
                                             public hIReferenceCounted,
                                             public hMapElement< hUint32, hRasterizerState >
    {
    public:
        hRasterizerState(hRasterizerZeroRefProc zeroproc)
            : zeroProc_(zeroproc)
        {}
    private:
        void OnZeroRef() {
            zeroProc_(this);
        }
        hRasterizerZeroRefProc zeroProc_;
    };

    hFUNCTOR_TYPEDEF(void(*)(class hDepthStencilState*), hDepthStencilZeroRefProc);
    class HEART_DLLEXPORT hDepthStencilState : public hdDepthStencilState,
                                                public hIReferenceCounted,
                                                public hMapElement< hUint32, hDepthStencilState >
    {
    public:
        hDepthStencilState(hDepthStencilZeroRefProc zeroproc)
            : zeroProc_(zeroproc)
        {}
    private:
        void OnZeroRef() {
            zeroProc_(this);
        }
        hDepthStencilZeroRefProc zeroProc_;
    };

    hFUNCTOR_TYPEDEF(void(*)(class hSamplerState*), hSamplerZeroRefProc);
    class HEART_DLLEXPORT hSamplerState : public hdSamplerState,
                                           public hIReferenceCounted,
                                           public hMapElement< hUint32, hSamplerState >
    {
    public:
        hSamplerState(hSamplerZeroRefProc zeroproc)
            : zeroProc_(zeroproc)
        {}
    private:
        void OnZeroRef() {
            zeroProc_(this);
        }
        hSamplerZeroRefProc zeroProc_;
    };
}

#endif // HRENDERSTATEBLOCK_H__