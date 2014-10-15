/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef HRENDERSUBMISSIONCONTEXT_H__
#define HRENDERSUBMISSIONCONTEXT_H__

#include "base/hTypes.h"
#include "math/hMatrix.h"

namespace Heart {
namespace hRender {
class hContext;

void    setViewport(hContext* ctx, const hViewport& viewport);
void    setScissorRect(hContext* ctx, const hScissorRect& scissor);
void    clearColour(hContext* ctx, const hColour& colour);
void    clearDepth(hContext* ctx, hFloat z);
void    drawPrimitive(hContext* ctx, hUint32 nPrimatives, hUint32 startVertex);
void    drawIndexedPrimitive(hContext* ctx, hUint32 nPrimatives, hUint32 startVertex);
void    drawIndexedPrimitiveInstanced(hContext* ctx, hUint instanceCount, hUint32 nPrimatives, hUint32 startVertex);

}
#if 0
    class hRenderSubmissionCtx
    {
    public:

        hRenderSubmissionCtx() {}
        ~hRenderSubmissionCtx();

        void    Initialise();

        hdRenderCommandBuffer saveToCommandBuffer() { return impl_.SaveToCommandBuffer(); }
        void    runCommandBuffer(hdRenderCommandBuffer cmdBuf);
        void    runRenderCommands(hRCmd* cmds);
        void    setTargets(hUint32 n, hRenderTargetView** targets, hDepthStencilView* depth);
        void    SetViewport(const hViewport& viewport);
        void    SetScissorRect(const hScissorRect& scissor);
        void    setRenderStateBlock(hdBlendState* st) {
            impl_.setRenderStateBlock( st );
        }
        void    setRenderStateBlock(hdDepthStencilState* st) {
            impl_.setRenderStateBlock( st );
        }
        void    setRenderStateBlock(hdRasterizerState* st) {
            impl_.setRenderStateBlock( st );
        }
        void    clearColour(hRenderTargetView* view, const hColour& colour);
        void    clearDepth(hDepthStencilView* view, hFloat z);
        void    DrawPrimitive(hUint32 nPrimatives, hUint32 startVertex);
        void    DrawIndexedPrimitive(hUint32 nPrimatives, hUint32 startVertex);
        void    DrawIndexedPrimitiveInstanced(hUint instanceCount, hUint32 nPrimatives, hUint32 startVertex);
        void    dispatch(hUint x, hUint y, hUint z) { impl_.dispatch(x, y, z); }
        // mapping
        void    Map(hIndexBuffer* ib, hIndexBufferMapInfo* outInfo);
        void    Unmap(hIndexBufferMapInfo* outInfo);
        void    Map(hVertexBuffer* ib, hVertexBufferMapInfo* outInfo);
        void    Unmap(hVertexBufferMapInfo* outInfo);
        void    Map(hTexture* ib, hUint32 level, hTextureMapInfo* outInfo);
        void    Unmap(hTextureMapInfo* outInfo);
        void    Map(hRenderBuffer* cb, hRenderBufferMapInfo* outInfo);
        void    Unmap(hRenderBufferMapInfo* info);

        void    appendRenderStats(hRenderFrameStats* outstats) { impl_.appendRenderStats(outstats); }
        void    resetStats() { impl_.resetStats(); }
        const hRenderFrameStats* getRenderStats() const { return impl_.getRenderStats(); }

    private:

        hdRenderSubmissionCtx           impl_;
    };
#endif
}

#endif // HRENDERSUBMISSIONCONTEXT_H__