/********************************************************************

    filename: 	hRenderSubmissionContext.h	
    
    Copyright (c) 3:1:2012 James Moran
    
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
#ifndef HRENDERSUBMISSIONCONTEXT_H__
#define HRENDERSUBMISSIONCONTEXT_H__

namespace Heart
{
    struct hViewport;
    class hTexture;
    struct hTextureMapInfo;
    class hTextureBase;
    class hRenderTargetTexture;
    class DepthSurface;
    class hVertexDeclaration;
    class hMaterialInstance;
    class hMaterialTechnique;
    class hIndexBuffer;
    struct hIndexBufferMapInfo;
    class hVertexBuffer;
    struct hVertexBufferMapInfo;
    class hMesh;
    class hMaterial;
    struct hSamplerParameter;
    class hShaderProgram;
    class hRendererCamera;
    class hRenderer;

    //Should these be in there own file?
    struct hViewportShaderConstants
    {
        hMatrix view_;
        hMatrix viewInverse_;
        hMatrix viewInverseTranspose_;
        hMatrix projection_;
        hMatrix projectionInverse_;
        hMatrix viewProjection_;
        hMatrix viewProjectionInverse_;
        hFloat  viewportSize_[4]; //x,y,0,0
    };

    struct hInstanceConstants
    {
        hMatrix world_;
    };

    struct hConstBlockMapInfo
    {
        hdParameterConstantBlock* cb;
        void* ptr;
    };

    class hRenderInputStreams;

    class HEART_DLLEXPORT hRenderSubmissionCtx
    {
    public:

        hRenderSubmissionCtx()
            : renderer_(NULL)
            , viewportConstants_(NULL)
        {}
        ~hRenderSubmissionCtx();

        void    Initialise( hRenderer* renderer );

        hdRenderCommandBuffer SaveToCommandBuffer() { return impl_.SaveToCommandBuffer(); }
        void                  RunCommandBuffer(hdRenderCommandBuffer cmdBuf);
        //Raw Functions

        void    SetInputStreams(hdRenderStreamsObject* streams) { impl_.SetInputStreams(streams); }
        void    SetRenderInputObject(hdRenderInputObject* rnInput) { impl_.SetRenderInputObject(rnInput); }
        void    SetRenderTarget(hUint32 idx , hTexture* pTarget);
        void    SetDepthTarget(hTexture* depth);
        void    SetViewport(const hViewport& viewport);
        void    SetScissorRect(const hScissorRect& scissor);
        void    SetMaterialPass(hMaterialTechniquePass* pass);
        void    SetRenderStateBlock(hdBlendState* st) { impl_.SetRenderStateBlock( st ); }
        void    SetRenderStateBlock(hdDepthStencilState* st) { impl_.SetRenderStateBlock( st ); }
        void    SetRenderStateBlock(hdRasterizerState* st) { impl_.SetRenderStateBlock( st ); }
        void    ClearTarget(hBool clearColour, const hColour& colour, hBool clearZ, hFloat z);
        void    DrawPrimitive(hUint32 nPrimatives, hUint32 startVertex);
        void    DrawIndexedPrimitive(hUint32 nPrimatives, hUint32 startVertex);
        void    DrawIndexedPrimitiveInstanced(hUint instanceCount, hUint32 nPrimatives, hUint32 startVertex);
        void    Map(hIndexBuffer* ib, hIndexBufferMapInfo* outInfo);
        void    Unmap(hIndexBufferMapInfo* outInfo);
        void    Map(hVertexBuffer* ib, hVertexBufferMapInfo* outInfo);
        void    Unmap(hVertexBufferMapInfo* outInfo);
        void    Map(hTexture* ib, hUint32 level, hTextureMapInfo* outInfo);
        void    Unmap(hTextureMapInfo* outInfo);
        void    Map(hdParameterConstantBlock* cb, hConstBlockMapInfo* outInfo);
        void    Unmap(hConstBlockMapInfo* info);
        void    Update(hdParameterConstantBlock* cb) { impl_.Update(cb); }

    private:

        friend class hRenderer; 
           
        hRenderer*                      renderer_;
        hUint32                         techniqueMask_;
        hMaterialInstance*              currentMaterial_;
        hMaterialTechnique*             currentTechnique_;
        const hViewportShaderConstants* viewportConstants_;
        hdParameterConstantBlock*       viewportConstantsBlock_;
        hInstanceConstants*             instanceConstants_;
        hdParameterConstantBlock*       instanceConstantsBlock_; 
        hdRenderSubmissionCtx           impl_;
    };

}

#endif // HRENDERSUBMISSIONCONTEXT_H__