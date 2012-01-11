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

#include "hTypes.h"
#include "hMemory.h"
#include "hMath.h"

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
    class hIndexBuffer;
    struct hIndexBufferMapInfo;
    class hVertexBuffer;
    struct hVertexBufferMapInfo;
    class hMesh;
    struct hSamplerParameter;
    class hShaderProgram;
    class hRenderViewport;

    //Should these be in there own file?
    struct hViewportShaderConstants
    {
        hMatrix view_;
        hMatrix viewInverse_;
        hMatrix viewInverseTranspose_;
        hMatrix projection_;
        hMatrix projectionInverse_;
    };

    struct hInstanceConstants
    {
        hMatrix worldViewProj_;
        hMatrix worldView_;
        hMatrix world_;
    };

    class hRenderSubmissionCtx
    {
    public:

        hRenderSubmissionCtx()
        {}
        ~hRenderSubmissionCtx() 
        {}

        void    SetViewport( hRenderViewport* viewport );

        hdRenderCommandBuffer SaveToCommandBuffer();
        void	SetIndexStream( hIndexBuffer* pIIBuf );
        void	SetVertexStream( hUint32 stream, hVertexBuffer* vtxBuf );
        void    SetVertexDeclaration( hVertexDeclaration* vtxDecl );
        void	SetRenderTarget( hUint32 idx , hTexture* pTarget );
        void	SetViewport( const hViewport& viewport );
        void	SetScissorRect( const ScissorRect& scissor );
        void    SetPixelShader( hShaderProgram* ps );
        void    SetVertexShader( hShaderProgram* vs );
        void    SetConstantBuffer( hdParameterConstantBlock* constBuffer );
        void	ClearTarget( hBool clearColour, hColour& colour, hBool clearZ, hFloat z );
        void    SetPrimitiveType( PrimitiveType primType );
        void	DrawPrimitive( hUint32 nPrimatives, hUint32 startVertex );
        void    DrawIndexedPrimitive( hUint32 nPrimatives, hUint32 startVertex );
        void    RunCommandBuffer( hdRenderCommandBuffer cmdBuf );
        void    Map( hIndexBuffer* ib, hIndexBufferMapInfo* outInfo );
        void    Unmap( hIndexBufferMapInfo* outInfo );
        void    Map( hVertexBuffer* ib, hVertexBufferMapInfo* outInfo );
        void    Unmap( hVertexBufferMapInfo* outInfo );
        void    Map( hTexture* ib, hUint32 level, hTextureMapInfo* outInfo );
        void    Unmap( hTextureMapInfo* outInfo );

        //Debug
        void	EnableDebugDrawing( hBool val );
        void	RenderDebugText( hFloat x, hFloat y, const char* fmt, ... );
        void	RenderDebugSphere( const hVec3& centre, hFloat radius, const hColour& colour );
        void	RenderDebugAABB( const hAABB& aabb, const hColour& colour );
        void	RenderDebugScreenQuad( const hRect& rect, const hColour& colour );
        void	RenderDebugCone( const hMatrix& transform, hFloat radius, hFloat len, const hColour& colour );
        hdRenderCommandBuffer SaveDebugCallsToCommandBuffer();

    private:

        friend class hRenderer; 

        //Debug
        void	InitialiseDebugInterface( hRenderer* renderer );

        const hViewportShaderConstants* viewportConstants_;
        hInstanceConstants              instanceConstants_;
        hdRenderSubmissionCtx           impl_;
        hdRenderSubmissionCtx           debug_;
    };

}

#endif // HRENDERSUBMISSIONCONTEXT_H__