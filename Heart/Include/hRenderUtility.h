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
#include "hVector.h"
#include "hResource.h"

namespace Heart
{
	class hVec3;
	class hMatrix;

	class hRenderer;
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
		hRenderer* renderer, 
		hResourceHandle< hIndexBuffer >* outIdxBuf,
		hResourceHandle< hVertexBuffer >* outVtxBuf );
	extern void		BuildConeMesh(
		hUint16 segments, 
		hFloat radius,
		hFloat depth,
		hRenderer* renderer, 
		hResourceHandle< hIndexBuffer >* outIdxBuf,
		hResourceHandle< hVertexBuffer >* outVtxBuf );
	extern void		BuildDebugSphereMesh( 
		const hVec3& centre,
		hUint16 segments, 
		hUint16 rings, 
		hFloat radius, 
		hUint16* startIdx,
		hUint16* startVtx,
		hColour colour,
		hMatrix* vp,
		hResourceHandle< hIndexBuffer >* outIdxBuf, 
		hResourceHandle< hVertexBuffer >* outVtxBuf );
	extern void		BuildDebugConeMesh(
		const hMatrix* transform,
		hUint16 segments, 
		hFloat radius, 
		hFloat lenght, 
		hUint16* startIdx, 
		hUint16* startVtx, 
		hColour colour, 
		hMatrix* vp, 
		hResourceHandle< hIndexBuffer >* outIdxBuf, 
		hResourceHandle< hVertexBuffer >* outVtxBuf );
	extern void		BuildFullscreenQuadMesh(
		hRenderer* renderer, 
		hResourceHandle< hIndexBuffer >* outIdxBuf,
		hResourceHandle< hVertexBuffer >* outVtxBuf );

}																					
}

#endif // HRRENDERUTILITY_H__