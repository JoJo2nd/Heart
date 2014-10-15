/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef HRRENDERUTILITY_H__
#define HRRENDERUTILITY_H__

#include "base/hTypes.h"
#include "math/hVec2.h"
#include "math/hVec3.h"

namespace Heart
{
    
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
		hIndexBuffer** outIdxBuf,
		hVertexBuffer** outVtxBuf ); 
    void HEART_API buildConeMesh(hUint16 segments, hFloat radius, hFloat depth, hVertexBuffer** outVtxBuf);
    void HEART_API buildTessellatedQuadMesh(hFloat width, hFloat height, hUint hdivs, hUint vdivs, hIndexBuffer** outIdxBuf, hVertexBuffer** outVtxBuf);
    hVertexBuffer* HEART_API buildDebugCubeMesh(hVertexBuffer** retVB);

    //////////////////////////////////////////////////////////////////////////
    // Materials /////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    hMaterial* HEART_API buildDebugFontMaterial(hMaterial* ddrawmat);   
    hMaterial* HEART_API buildDebugPosColUVMaterial(hMaterial* ddrawmat);    
    hMaterial* HEART_API buildDebugPosColMaterial(hMaterial* ddrawmat);    
    hMaterial* HEART_API buildDebugPosColUVAlphaMaterial(hMaterial* ddrawmat);
    hMaterial* HEART_API buildDebugPosColAlphaMaterial(hMaterial* ddrawmat);
    hFont* HEART_API createDebugFont(hFont* outfont, hTexture** outtex);
    void HEART_API destroyDebugFont(hFont* font, hTexture* tex);
    void HEART_API setCameraParameters(hRenderSubmissionCtx* ctx, hRendererCamera* camera);
    void HEART_API submitDrawCalls(hRenderSubmissionCtx* ctx, hRendererCamera* camera, const hDrawCall* dcs, hUint dcn, hUint32 clearflags);
    void HEART_API sortDrawCalls(hDrawCall* dcs, hUint dcn);
}
}

#endif // HRRENDERUTILITY_H__