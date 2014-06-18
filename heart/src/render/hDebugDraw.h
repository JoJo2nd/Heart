/********************************************************************

    filename:   hDebugDraw.h  
    
    Copyright (c) 27:6:2013 James Moran
    
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

#ifndef HDEBUGDRAW_H__
#define HDEBUGDRAW_H__

#include "base/hArray.h"
#include "base/hRendererConstants.h"
#include "base/hTypes.h" 
#include "math/hVec3.h"
#include "pal/dx11/hWin32DX11.h"
#include "pal/hMutex.h"
#include <vector>

namespace Heart
{
    class hFont;

    struct hDebugLine
    {
        hVec3 p1, p2;
        hColour colour;
    };

    struct hDebugTriPosCol
    {
        hVec3   pos;
        hColour colour;
    };

    struct hDebugTextString
    {
        hColour colour;
        hVec3   position;
        hUint   txtBufOffset;
    };

    class hShaderResourceView;

    struct hDebugTexQuad
    {
        hShaderResourceView* srv;
        hVec3 position;
        hVec3 width;
        hVec3 height;
        hUint startvtx;
    };

    struct hDebugPrimsSet
    {
        void reset();
        void clear();

        std::vector< hDebugLine >       lines_;
        std::vector< hDebugTriPosCol >  tris_;
        std::vector< hDebugTextString > strings_;
        std::vector< hDebugTexQuad >    texQuads_;
        std::vector< hChar >            txtBuffer_;
    };

    enum hDebugSet
    {
        eDebugSet_2DNoDepth,
        eDebugSet_3DDepth,

        eDebugSet_Max,
    };

    class hDebugDraw
    {
    public:
        hDebugDraw();
        ~hDebugDraw();

        void begin();
        void drawLines(hDebugLine* points, hUint32 linecount, hDebugSet set);
        void drawTris(const hVec3* tris, hUint tricount, const hColour& colour, hDebugSet set);
        void drawText(const hVec3& screenpos, const hChar* buffer, const hColour& colour, hUint textLimit=0);
        void drawTexturedQuad(const hVec3& screenpos, hFloat width, hFloat height, hShaderResourceView* texturesrv);
        void end();

        static hDebugDraw* it();

    private:
        
        friend class hDebugDrawRenderer;

        hDebugDrawRenderer* renderer_;
        hDebugPrimsSet debugPrims_[eDebugSet_Max];
    };

    class hDepthStencilView;
    class hFont;
    class hIndexBuffer;
    class hMaterial;
    class hRenderBuffer;
    
    class hRenderSubmissionCtx;
    class hRenderTargetView;
    class hShaderResourceView;
    class hTexture;
    class hVertexBuffer;

    class hDebugDrawRenderer
    {
    public:

        hDebugDrawRenderer();
        ~hDebugDrawRenderer();

        void initialiseResources();
        void destroyResources();
        void setMainViewCameraIndex(hUint camIndex);

        void render(hRenderSubmissionCtx* ctx);
        void append(hDebugDraw* debugdraw);

        static hDebugDrawRenderer* it();
    private:

        static const hUint s_maxDebugPrims = 20000;

        hMutex              critSection_;
        hRenderTargetView*  colourView_;
        hDepthStencilView*  depthView_;
        hDebugPrimsSet      debugPrims_[eDebugSet_Max];

        hMaterial*          debugPosColMat_;
        hMaterial*          debugPosColUVMat_;
        hMaterial*          debugFontMat_;
        hMaterial*          debugPosColAlphaMat_;
        hMaterial*          debugPosColUVAlphaMat_;

        hVertexBuffer*      posColBuffer_;
        hVertexBuffer*      posColUVBuffer_;

        hFont*              debugFont_;
        hTexture*           debugFontTex_;
        hShaderResourceView* debugFontSRV_;

        hVector<hdInputLayout*> inputlayout_;
        hRenderBuffer*      viewParameters_;
        hRenderBuffer*      perDrawParameters_;

        hRenderCommands     posColRdrLineCmds_;
        hRenderCommands     posColRdrCmds_;
        hRenderCommands     posColUVRdrCmds_;
        hRenderCommands     textRdrCmds_;
        hRenderCommands     posColAlphaRdrCmds_;
        hRenderCommands     posColAlphaRdrLineCmds_;
        hRenderCommands     posColUVAlphaRdrCmds_;

        hBool               resourcesCreated_;
    };
}

#endif // HDEBUGDRAW_H__