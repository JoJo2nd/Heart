/********************************************************************
	created:	2009/12/31
	created:	31:12:2009   9:38
	filename: 	RenderUtility.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hRenderUtility.h"
#include "hMathUtil.h"
#include <math.h>
#include "hVector.h"
#include "hRenderer.h"
#include "hIndexBuffer.h"
#include "hVertexBuffer.h"
#include "hResource.h"
#include "hMatrix.h"
#include "hRendererConstants.h"

namespace Heart
{
namespace RenderUtility
{

	hFloat ComputeGaussianCurve( hFloat step, hFloat power )
	{
		hFloat theta = power;

		return (hFloat)( (1.0 / sqrt( 2 * hmPI * theta ) ) * exp(-(step * step) / (2 * theta * theta)));
	}

	void ComputeBlurWeights( hFloat blurPower, hUint32 samples, hFloat* pOutWeights )
	{
		// The first sample always has a zero offset.
		pOutWeights[0] = ComputeGaussianCurve( 0.0f, blurPower );

		// Maintain a sum of all the weighting values.
		float totalWeights = pOutWeights[0];

		// Add pairs of additional sample taps, positioned
		// along a line in both directions from the center.
		for ( hUint32 i = 0; i < samples / 2; ++i )
		{
			// Store weights for the positive and negative taps.
			hFloat weight = ComputeGaussianCurve( i + 1.0f, blurPower );

			pOutWeights[i * 2 + 1] = weight;
			pOutWeights[i * 2 + 2] = weight;

			totalWeights += weight * 2;
		}

		// Normalize the list of sample weightings, so they will always sum to one.
		for (hUint32 i = 0; i < samples; i++)
		{
			pOutWeights[i] /= totalWeights;
		}
	}

	void ComputeBlurOffsets( hFloat dx, hFloat dy, hUint32 samples, hVec2* pOutSamples )
	{
		// The first sample always has a zero offset.
        pOutSamples[0] = hVec2Func::zeroVector();

		// Add pairs of additional sample taps, positioned
		// along a line in both directions from the center.
		for ( hUint32 i = 0; i < samples / 2; ++i )
		{
			// To get the maximum amount of blurring from a limited number of
			// pixel shader samples, we take advantage of the bilinear filtering
			// hardware inside the texture fetch unit. If we position our texture
			// coordinates exactly halfway between two texels, the filtering unit
			// will average them for us, giving two samples for the price of one.
			// This allows us to step in units of two texels per sample, rather
			// than just one at a time. The 1.5 offset kicks things off by
			// positioning us nicely in between two texels.
			hFloat sampleOffset = i * 2 + 1.5f;

			hVec2 delta = hVec2( dx, dy ) * sampleOffset;

			// Store texture coordinate offsets for the positive and negative taps.
			pOutSamples[i * 2 + 1] = delta;
			pOutSamples[i * 2 + 2] = -delta;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	extern void BuildSphereMesh( hUint16 segments, 
								 hUint16 rings, 
								 hFloat radius, 
								 hRenderSubmissionCtx* ctx, 
								 hIndexBuffer* idxBuf, 
								 hVertexBuffer* vtxBuf )
	{
		hUint16 vtxCnt = GetSphereMeshVertexCount( segments, rings );//(hUint16)((rings + 1) * (segments+1)) + 1;
		hUint16 idxCnt = GetSphereMeshIndexCount( segments, rings );//(hUint16)(6 * rings * (segments+1)) + 1;

        hIndexBufferMapInfo  ibMap;
        hVertexBufferMapInfo vbMap;
        ctx->Map( idxBuf, &ibMap );
        ctx->Map( vtxBuf, &vbMap );

        hFloat* vtx      = (hFloat*)vbMap.ptr_;
        hUint16* indices = (hUint16*)ibMap.ptr_;

		hFloat dRingAngle = (hmPI / rings);
		hFloat dSegAngle = (2 * hmPI / segments);

		hUint16 idxIdx = 0;
		hUint16 vtxIdx = 0;

		// Generate the group of rings for the sphere
		for( hUint32 ring = 0; ring <= rings; ring++ ) 
		{
			float r0 = radius * sinf (ring * dRingAngle);
			float y0 = radius * cosf (ring * dRingAngle);

			// Generate the group of segments for the current ring
			for( hUint32 seg = 0; seg <= segments; seg++) 
			{
				float x0 = r0 * sinf(seg * dSegAngle);
				float z0 = r0 * cosf(seg * dSegAngle);
				hVec3 v( x0, y0, z0 );
				hCPUVec3 vNormal = hVec3Func::normaliseFast( v );//needs flipping, but shouldn't

				// Add one vertex to the strip which makes up the sphere
	 			*vtx++ = x0;
	 			*vtx++ = y0;
	 			*vtx++ = z0;

// 				//Normal
// 	 			*vtx++ = vNormal.x;
// 	 			*vtx++ = vNormal.y;
// 	 			*vtx++ = vNormal.z;
// 
// 				//UV
// 	 			*vtx++ = (float) seg / (float) segments;
// 	 			*vtx++ = (float) ring / (float) rings;

				if ( ring != rings ) 
				{
					// each vertex (except the last) has six indices pointing to it
	 				*indices++ = vtxIdx + segments + 1;
	 				*indices++ = vtxIdx;               
	 				*indices++ = vtxIdx + segments;
	 				*indices++ = vtxIdx + segments + 1;
	 				*indices++ = vtxIdx + 1;
	 				*indices++ = vtxIdx;
				}
				++vtxIdx;
			}
		}

        ctx->Unmap( &ibMap );
        ctx->Unmap( &vbMap );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	extern void BuildDebugSphereMesh( const hCPUVec3& centre,
									hUint16 segments, 
									hUint16 rings, 
									hFloat radius, 
									hUint16* startIdx,
									hUint16* startVtx,
									hColour colour,
									hMatrix* vp,
									hUint16* outIdxBuf, 
									void* outVtxBuf )
	{

        struct Vertex
        {
            hCPUVec4    post;
            hUint32     colour;
        };

		hUint16 vtxCnt = (hUint16)((rings + 1) * (segments+1)) + 1;
		hUint16 idxCnt = (hUint16)(6 * rings * (segments+1)) + 1;
		hUint32 dwColour = (hUint32)colour;
        Vertex* vtx = (Vertex*)outVtxBuf;

		hFloat dRingAngle = (hmPI / rings);
		hFloat dSegAngle = (2 * hmPI / segments);

		hUint16 idxIdx = *startIdx;
		hUint16 vtxIdx = *startVtx;

		// Generate the group of rings for the sphere
		for( hUint32 ring = 0; ring <= rings; ring++ ) 
		{
			float r0 = radius * sinf (ring * dRingAngle);
			float y0 = radius * cosf (ring * dRingAngle);

			// Generate the group of segments for the current ring
			for( hUint32 seg = 0; seg <= segments; seg++) 
			{
				float x0 = r0 * sinf(seg * dSegAngle);
				float z0 = r0 * cosf(seg * dSegAngle);
				hVec4 v( x0+centre.x, y0+centre.y, z0+centre.z, 1.0f );

				vtx[vtxIdx].post   = hMatrixFunc::mult( v, *vp );
                vtx[vtxIdx].colour = dwColour;

				if ( ring != rings ) 
				{
					// each vertex (except the last) has six indices pointing to it
                    outIdxBuf[idxIdx++] = vtxIdx;
                    outIdxBuf[idxIdx++] = vtxIdx+segments+1;
                    outIdxBuf[idxIdx++] = vtxIdx+segments;

                    outIdxBuf[idxIdx++] = vtxIdx+1;
                    outIdxBuf[idxIdx++] = vtxIdx+segments+1;
                    outIdxBuf[idxIdx++] = vtxIdx;

				}
				++vtxIdx;
			}
		}

		*startIdx = idxIdx;
		*startVtx = vtxIdx;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	extern void BuildFullscreenQuadMesh( 
        hFloat width,
        hFloat height,
		hRenderSubmissionCtx* ctx,  
		hIndexBuffer* idxBuf, 
		hVertexBuffer* vtxBuf )
	{
		hUint16 quadIdx[] =
		{
			0,2,1,2,3,1
		};
		hFloat wo2 = width / 2.0f;
		hFloat ho2 = height / 2.0f;
        struct Vertex
        {
            hCPUVec3 pos;
            hCPUVec2 uv;
        };

        hIndexBufferMapInfo ibMapInfo;
        ctx->Map( idxBuf, &ibMapInfo );

        hMemCpy( ibMapInfo.ptr_, quadIdx, sizeof(quadIdx) );

        ctx->Unmap( &ibMapInfo );

        hVertexBufferMapInfo vbMapInfo;
        ctx->Map( vtxBuf, &vbMapInfo );

        Vertex* vtx = (Vertex*)vbMapInfo.ptr_;

        vtx[0].pos = hCPUVec3( -wo2, ho2,  0.25f );
        vtx[0].uv  = hCPUVec2( 0.f, 0.f );

        vtx[0].pos = hCPUVec3(  wo2, ho2,  0.25f );
        vtx[0].uv  = hCPUVec2( 1.f, 0.f );

        vtx[0].pos = hCPUVec3( -wo2, -ho2, 0.25f );
        vtx[0].uv  = hCPUVec2( 0.f, 1.f );

        vtx[0].pos = hCPUVec3(  wo2, -ho2, 0.25f );
        vtx[0].uv  = hCPUVec2( 1.f, 1.f );

        ctx->Unmap( &vbMapInfo );
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void BuildConeMesh(	hUint16 segments, 
					    hFloat radius, 
						hFloat depth, 
                        hRenderSubmissionCtx* ctx, 
						hIndexBuffer* outIdxBuf, 
						hVertexBuffer* outVtxBuf )
	{
		if ( segments < 4)
		{
			segments = 4;
		}

        struct Vertex
        {
            hCPUVec3 pos;
            hCPUVec3 normal;
        };

		hUint16 vtxCnt = (hUint16)(segments+2);
		hUint16 idxCnt = (hUint16)(segments*6);


		hUint16 iidx = 0;
		hUint16 vidx = 0;
        hIndexBufferMapInfo  ibMap;
        hVertexBufferMapInfo vbMap;
        ctx->Map( outIdxBuf, &ibMap );
        ctx->Map( outVtxBuf, &vbMap );
        hUint16* idx = (hUint16*)ibMap.ptr_;
        Vertex* vtx = (Vertex*)vbMap.ptr_;

		hVec3* segmentPts = (hVec3*)hAlloca( sizeof(hVec3)*(segments+2) );
		hFloat dSegAngle = (2 * hmPI / segments);

		for ( hUint32 i = 0; i < segments; ++i )
		{
			float x = radius * sinf( dSegAngle*i );
			float y = radius * cosf( dSegAngle*i );

            vtx->pos = hCPUVec3( x, y, depth );
            ++vtx;
		}
        //Tip of cone
        vtx->pos = hCPUVec3( 0.0f, 0.0f, 0.0f );
        ++vtx;
        // centre base of cone
        vtx->pos = hCPUVec3( 0.0f, 0.0f, depth );
        ++vtx;

		//Create the Cone
		for ( hUint16 i = 0; i < segments; ++i )
		{
            idx[iidx++] = segments;
            idx[iidx++] = (i+1)%segments;
            idx[iidx++] = i;
		}

		//Create the Base
		for ( hUint16 i = 0; i < segments; ++i )
		{
            idx[iidx++] = segments+1;
            idx[iidx++] = i;
            idx[iidx++] = (i+1)%segments;
		}

 		ctx->Unmap( &ibMap );
        ctx->Unmap( &vbMap );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	extern void BuildDebugConeMesh( const hMatrix* transform,
								   hUint16 segments, 
								   hFloat radius, 
								   hFloat lenght, 
								   hUint16* startIdx, 
								   hUint16* startVtx, 
								   hColour colour, 
								   hMatrix* vp, 
								   hUint16* outIdxBuf, 
								   void* outVtxBuf )
	{
		if ( segments < 4)
		{
			segments = 4;
		}

        struct Vertex
        {
            hVec4    post;
            hUint32  colour;
        };

		hUint32 dwColour = (hUint32)colour;

		hUint16 iidx = *startIdx;
		hUint16 vidx = *startVtx;
        Vertex* vtx = (Vertex*)outVtxBuf;
        //TODO: check alignment here...
		hVec4* segmentPts = (hVec4*)hAlloca( sizeof(hVec4)*(segments+2) );
		hFloat dSegAngle = (2 * hmPI / segments);

		for ( hUint32 i = 0; i < segments; ++i )
		{
			float x = radius * sinf( dSegAngle*i );
			float y = radius * cosf( dSegAngle*i );

			segmentPts[i] = hVec4( x, y, lenght, 1.0f );
		}
		segmentPts[segments]   = hVec4( 0.0f, 0.0f, 0.0f, 1.0f );//Tip of cone
		segmentPts[segments+1] = hVec4( 0.0f, 0.0f, lenght, 1.0f );// centre base of cone


		hMatrix wvp = (*transform) * (*vp);

		for ( hUint16 i = 0; i < segments+2; ++i )
		{
            vtx->post   = segmentPts[i] * wvp;
            vtx->colour = dwColour;
		}

		//Create the Cone
		for ( hUint16 i = 0; i < segments; ++i )
		{
            outIdxBuf[iidx++] = (segments)+(*startVtx);//Tip
            outIdxBuf[iidx++] = i+(*startVtx);//Base 1
            outIdxBuf[iidx++] = ((i+1)%segments)+(*startVtx);//Base 2
        }

		//Create the Base
		for ( hUint16 i = 0; i < segments; ++i )
		{
            outIdxBuf[iidx++] = (segments+1)+(*startVtx);//Base Centre
            outIdxBuf[iidx++] = i+(*startVtx);//Base 1
            outIdxBuf[iidx++] = ((i+1)%segments)+(*startVtx);//Base 2
		}

		*startIdx = iidx;
		*startVtx = vidx;
	}

}
}