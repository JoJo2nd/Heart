/********************************************************************

	filename: 	hRenderUtility.h	
	
	Copyright (c) 1:4:2012 James Moran
	
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

#ifndef HRRENDERUTILITY_H__
#define HRRENDERUTILITY_H__

#include "base/hTypes.h"
#include "math/hVec2.h"
#include "math/hVec3.h"

namespace Heart
{
    class hRenderer;
	class hRenderSubmissionCtx;
	class hIndexBuffer;
	class hVertexBuffer;
    class hMaterial;
    class hTexture;
    class hFont;
    class hRendererCamera;
    struct hDrawCall;

namespace hRenderUtility
{

	extern hFloat	ComputeGaussianCurve( hFloat step, hFloat power );
	extern void		ComputeBlurWeights( hFloat blurPower, hUint32 samples, hFloat* pOutWeights );
	extern void		ComputeBlurOffsets( hFloat dx, hFloat dy, hUint32 samples, Heart::hVec2* pOutSamples );
    //////////////////////////////////////////////////////////////////////////
    // Debug mesh shapes /////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    inline hUint16  GetSphereMeshVertexCount( hUint16 segments, hUint16 rings ) { return (hUint16)((rings + 1) * (segments+1)) + 1; }
    inline hUint16  GetSphereMeshIndexCount( hUint16 segments, hUint16 rings ) { return (hUint16)(6 * rings * (segments+1)) + 1; }
	
    void HEART_API buildSphereMesh( 
		hUint16 segments, 
		hUint16 rings, 
		hFloat radius,
		hRenderer* rndr, 
		hIndexBuffer** outIdxBuf,
		hVertexBuffer** outVtxBuf );
	 
    void HEART_API buildConeMesh(hUint16 segments, hFloat radius, hFloat depth,
    hRenderer* rndr, hVertexBuffer** outVtxBuf);
	 
    void HEART_API buildTessellatedQuadMesh(hFloat width, hFloat height, hUint hdivs, hUint vdivs, 
    hRenderer* rndr, hIndexBuffer** outIdxBuf, hVertexBuffer** outVtxBuf);
    
    hVertexBuffer* HEART_API buildDebugCubeMesh(hRenderer* rndr, hVertexBuffer** retVB);
    //////////////////////////////////////////////////////////////////////////
    // Materials /////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    hMaterial* HEART_API buildDebugFontMaterial(hRenderer* rndr, hMaterial* ddrawmat);
    
    hMaterial* HEART_API buildDebugPosColUVMaterial(hRenderer* rndr, hMaterial* ddrawmat);
    
    hMaterial* HEART_API buildDebugPosColMaterial(hRenderer* rndr, hMaterial* ddrawmat);
    
    hMaterial* HEART_API buildDebugPosColUVAlphaMaterial(hRenderer* rndr, hMaterial* ddrawmat);
    
    hMaterial* HEART_API buildDebugPosColAlphaMaterial(hRenderer* rndr, hMaterial* ddrawmat);
    
    hFont* HEART_API createDebugFont(hRenderer* rndr, hFont* outfont, hTexture** outtex);
    
    void HEART_API destroyDebugFont(hRenderer* rndr, hFont* font, hTexture* tex);
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    void HEART_API setCameraParameters(hRenderSubmissionCtx* ctx, hRendererCamera* camera);
    
    void HEART_API submitDrawCalls(hRenderSubmissionCtx* ctx, hRendererCamera* camera, const hDrawCall* dcs, hUint dcn, hUint32 clearflags);
    
    void HEART_API sortDrawCalls(hDrawCall* dcs, hUint dcn);
}
}

#endif // HRRENDERUTILITY_H__