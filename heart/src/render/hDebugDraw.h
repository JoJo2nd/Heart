/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hArray.h"
#include "base/hRendererConstants.h"
#include "base/hTypes.h" 
#include "math/hVec3.h"
#include "pal/hMutex.h"
#include <vector>

namespace Heart {
namespace hRenderer {
    struct hCmdList;
}
#if 0
    class hFont;
    class hDebugDrawRenderer;

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

        //hVector<hdInputLayout*> inputlayout_;
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

#endif
namespace hDebugOSD {
    void begin();
    void drawText(const hVec3& screenpos, const hColour& colour, const hChar* buffer, ...);
    //void drawTexturedQuad(const hVec3& screenpos, hFloat width, hFloat height, hShaderResourceView* texturesrv);
    void end();
    void renderOnScreenDisplay(hRenderer::hCmdList* cl);
}
}
