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

namespace Heart
{

	class hRenderSubmissionCtx;
	class hIndexBuffer;
	class hVertexBuffer;

namespace RenderUtility
{
	extern hFloat	ComputeGaussianCurve( hFloat step, hFloat power );
	extern void		ComputeBlurWeights( hFloat blurPower, hUint32 samples, hFloat* pOutWeights );
	extern void		ComputeBlurOffsets( hFloat dx, hFloat dy, hUint32 samples, Heart::hVec2* pOutSamples );
    inline hUint16  GetSphereMeshVertexCount( hUint16 segments, hUint16 rings ) { return (hUint16)((rings + 1) * (segments+1)) + 1; }
    inline hUint16  GetSphereMeshIndexCount( hUint16 segments, hUint16 rings ) { return (hUint16)(6 * rings * (segments+1)) + 1; }
	extern void		BuildSphereMesh( 
		hUint16 segments, 
		hUint16 rings, 
		hFloat radius,
		hRenderSubmissionCtx* ctx, 
		hIndexBuffer* outIdxBuf,
		hVertexBuffer* outVtxBuf );
	extern void		BuildConeMesh(
		hUint16 segments, 
		hFloat radius,
		hFloat depth,
		hRenderSubmissionCtx* ctx, 
		hIndexBuffer* outIdxBuf,
		hVertexBuffer* outVtxBuf );
	extern void		BuildDebugSphereMesh( 
		const hVec3& centre,
		hUint16 segments, 
		hUint16 rings, 
		hFloat radius, 
		hUint16* startIdx,
		hUint16* startVtx,
		hColour colour,
		hMatrix* vp,
		hIndexBuffer* outIdxBuf, 
		hVertexBuffer* outVtxBuf );
	extern void		BuildDebugConeMesh(
		const hMatrix* transform,
		hUint16 segments, 
		hFloat radius, 
		hFloat lenght, 
		hUint16* startIdx, 
		hUint16* startVtx, 
		hColour colour, 
		hMatrix* vp, 
		hIndexBuffer* outIdxBuf, 
		hVertexBuffer* outVtxBuf );
	extern void		BuildFullscreenQuadMesh(
		hRenderSubmissionCtx* ctx, 
		hIndexBuffer* outIdxBuf,
		hVertexBuffer* outVtxBuf );

}																					
}

#endif // HRRENDERUTILITY_H__