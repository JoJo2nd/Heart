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

namespace Heart
{
    class hFont;

    struct hDebugLine
    {
        hVec3 p1, p2;
        hColour colour;
    };

    struct hDebugTextString
    {
        hColour colour;
        hVec3   position;
        hUint   txtBufOffset;
    };

    struct hDebugPrimsSet
    {
        void reset();

        hVector< hDebugLine >       lines_;
        hVector< hDebugTextString > strings_;
        hVector< hChar >            txtBuffer_;
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
        void drawText(const hVec3& screenpos, const hChar* buffer, const hColour& colour, hDebugSet set);
        void end();

        static hDebugDraw* it();

    private:
        
        friend class hDebugDrawRenderer;

        hDebugDrawRenderer* renderer_;
        hDebugPrimsSet debugPrims_[eDebugSet_Max];
    };

    class hDebugDrawRenderer
    {
    public:

        hDebugDrawRenderer();
        ~hDebugDrawRenderer();

        void initialiseResources(hRenderer* renderer);
        void setMainViewCameraIndex(hUint camIndex);

        void destroyResources();
        void render(hRenderer* renderer, hRenderSubmissionCtx* ctx);
        void append(hDebugDraw* debugdraw);

        static hDebugDrawRenderer* it();
    private:

        static const hUint s_maxDebugPrims = 10000;

        hMutex              critSection_;
        hRenderTargetView*  colourView_;
        hDepthStencilView*  depthView_;
        hDebugPrimsSet      debugPrims_[eDebugSet_Max];
        hMaterial*          debugPosColMat_;
        hMaterial*          debugPosColUVMat_;
        hVertexBuffer*      posColBuffer_;
        hVertexBuffer*      posColUVBuffer_;
        hMaterial*          lineMaterial_;
        hFont*              debugFont_;
        hTexture*           debugFontTex_;
        hShaderResourceView* debugFontSRV_;
        hdInputLayout*      inputlayout_;
        hRenderBuffer*      viewParameters_;
        hRenderBuffer*      perDrawParameters_;
        hRenderCommands     posColRdrCmds_;
        hRenderCommands     posColUVRdrCmds_;
        hRenderCommands     tmpRdrCmds_;
        hBool               resourcesCreated_;
    };
}

#endif // HDEBUGDRAW_H__