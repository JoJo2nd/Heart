/********************************************************************
	created:	2009/12/31
	created:	31:12:2009   9:30
	filename: 	Utility.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HRRENDERUTILITY_H__
#define HRRENDERUTILITY_H__

#include "hTypes.h"
#include "hVec3.h"
#include "hMatrix.h"
#include "hResource.h"

namespace Heart
{

	class hRenderSubmissionCtx;
	class hIndexBuffer;
	class hVertexBuffer;

namespace Utility
{
	extern hFloat	ComputeGaussianCurve( hFloat step, hFloat power );
	extern void		ComputeBlurWeights( hFloat blurPower, hUint32 samples, hFloat* pOutWeights );
	extern void		ComputeBlurOffsets( hFloat dx, hFloat dy, hUint32 samples, Heart::hVec2* pOutSamples );
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