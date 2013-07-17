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
    struct hDebugLine
    {
        hVec3 p1, p2;
    };

    struct hDebugPrimsSet
    {
        void reset();

        hVector< hDebugLine > lines_;
    };

    enum hDebugSet
    {
        eDebugSet_2DDepth,
        eDebugSet_2DNoDepth,
        eDebugSet_3DDepth,
        eDebugSet_3DNoDepth,

        eDebugSet_Max,
    };

    class hDebugDraw
    {
    public:
        hDebugDraw();
        ~hDebugDraw();

        void begin();
        void drawLines(hVec3* points, hUint32 linecount, hDebugSet set);
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
        void destroyResources();
        void debugRenderSetup(hRenderTargetView* colour, hDepthStencilView* depth);
        void render(hRenderSubmissionCtx* ctx);
        void append(hDebugDraw* debugdraw);

        static hDebugDrawRenderer* it();
    private:

        static const hUint s_maxDebugPrims = 10000;

        hMutex              critSection_;
        hRenderTargetView*  colourView_;
        hDepthStencilView*  depthView_;
        hDebugPrimsSet      debugPrims_[eDebugSet_Max];
        hMaterial*          debugMat_;
        hVertexBuffer*      linesBuffer_;
        hMaterial*          lineMaterial_;
        hBool               resourcesCreated_;
    };
}

#endif // HDEBUGDRAW_H__