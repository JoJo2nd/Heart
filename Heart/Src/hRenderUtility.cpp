/********************************************************************
	created:	2009/12/31
	created:	31:12:2009   9:38
	filename: 	Utility.cpp	
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
namespace Utility
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
								hRenderer* renderer, 
								hResourceHandle< hIndexBuffer >* outIdxBuf, 
								hResourceHandle< hVertexBuffer >* outVtxBuf )
	{
/*
		hUint16 vtxCnt = (hUint16)((rings + 1) * (segments+1)) + 1;
		hUint16 idxCnt = (hUint16)(6 * rings * (segments+1)) + 1;
		hVertexDeclaration* vtxDecl;

		renderer->GetVertexDeclaration( vtxDecl, hrVF_XYZ | hrVF_NORMAL );
		renderer->CreateIndexBuffer( *outIdxBuf, NULL, idxCnt, 0, PRIMITIVETYPE_TRILIST, "SphereMeshIB" );
		renderer->CreateVertexBuffer( *outVtxBuf, vtxCnt, vtxDecl, 0, "SphereMeshVB" );

		(*outIdxBuf)->Lock();
		(*outVtxBuf)->Lock();

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
				hVec3 n,v( x0, y0, z0 );
				hVec3::normalise( v, n );//needs flipping, but shouldn't

				// Add one vertex to the strip which makes up the sphere
				(*outVtxBuf)->SetElement( vtxIdx, hrVE_XYZ, v );
	// 			*pVertex++ = x0;
	// 			*pVertex++ = y0;
	// 			*pVertex++ = z0;

				//Normal
				(*outVtxBuf)->SetElement( vtxIdx, hrVE_NORMAL, n );
	//			Vector3 vNormal = Vector3(x0, y0, z0).normalisedCopy();
	// 			*pVertex++ = vNormal.x;
	// 			*pVertex++ = vNormal.y;
	// 			*pVertex++ = vNormal.z;

				//UV
	// 			*pVertex++ = (float) seg / (float) nSegments;
	// 			*pVertex++ = (float) ring / (float) nRings;

				if ( ring != rings ) 
				{
					// each vertex (except the last) has six indices pointing to it
					(*outIdxBuf)->SetIndex( idxIdx++, vtxIdx );
					(*outIdxBuf)->SetIndex( idxIdx++, vtxIdx+segments+1 );
					(*outIdxBuf)->SetIndex( idxIdx++, vtxIdx+segments );
					
					(*outIdxBuf)->SetIndex( idxIdx++, vtxIdx+1 );
					(*outIdxBuf)->SetIndex( idxIdx++, vtxIdx+segments+1 );				
					(*outIdxBuf)->SetIndex( idxIdx++, vtxIdx );

	// 				*pIndices++ = wVerticeIndex + nSegments + 1;
	// 				*pIndices++ = wVerticeIndex;               
	// 				*pIndices++ = wVerticeIndex + nSegments;
	// 				*pIndices++ = wVerticeIndex + nSegments + 1;
	// 				*pIndices++ = wVerticeIndex + 1;
	// 				*pIndices++ = wVerticeIndex;
				}
				++vtxIdx;
			}
		}

		//Vec3 n,v( 0.0f, -radius, 0.0f );
		//Vec3::normalise( v, n );//needs flipping, but shouldn't
		//(*outVtxBuf)->SetElement( vtxIdx, hrVE_XYZ, v );
		//(*outVtxBuf)->SetElement( vtxIdx, hrVE_NORMAL, n );

		(*outIdxBuf)->Unlock();
		(*outVtxBuf)->Unlock();
*/
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	extern void BuildDebugSphereMesh( const hVec3& centre,
									hUint16 segments, 
									hUint16 rings, 
									hFloat radius, 
									hUint16* startIdx,
									hUint16* startVtx,
									hColour colour,
									hMatrix* vp,
									hResourceHandle< hIndexBuffer >* outIdxBuf, 
									hResourceHandle< hVertexBuffer >* outVtxBuf )
	{
/*
		hUint16 vtxCnt = (hUint16)((rings + 1) * (segments+1)) + 1;
		hUint16 idxCnt = (hUint16)(6 * rings * (segments+1)) + 1;
		hUint32 dwColour = (hUint32)colour;

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
				hVec4 v( x0+centre.x, y0+centre.y, z0+centre.z, 1.0f ), ov;

				hMatrix::mult( v, vp, ov );

				(*outVtxBuf)->SetElement( vtxIdx, hrVE_XYZW, ov );
				(*outVtxBuf)->SetElement( vtxIdx, hrVE_COLOR, dwColour );

				if ( ring != rings ) 
				{
					// each vertex (except the last) has six indices pointing to it
					(*outIdxBuf)->SetIndex( idxIdx++, vtxIdx );
					(*outIdxBuf)->SetIndex( idxIdx++, vtxIdx+segments+1 );
					(*outIdxBuf)->SetIndex( idxIdx++, vtxIdx+segments );

					(*outIdxBuf)->SetIndex( idxIdx++, vtxIdx+1 );
					(*outIdxBuf)->SetIndex( idxIdx++, vtxIdx+segments+1 );				
					(*outIdxBuf)->SetIndex( idxIdx++, vtxIdx );
				}
				++vtxIdx;
			}
		}

		*startIdx = idxIdx;
		*startVtx = vtxIdx;
*/
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	extern void BuildFullscreenQuadMesh( 
		hRenderer* renderer, 
		hResourceHandle< hIndexBuffer >* outIdxBuf, 
		hResourceHandle< hVertexBuffer >* outVtxBuf )
	{
		hVertexDeclaration* vtxDecl;
		hUint16 quadIdx[] =
		{
			0,2,1,2,3,1
		};
		hFloat wo2 = renderer->Width() / 2.0f;
		hFloat ho2 = renderer->Height() / 2.0f;

		renderer->GetVertexDeclaration( vtxDecl, hrVF_XYZ | hrVF_1UV  );

		renderer->CreateIndexBuffer( (*outIdxBuf), quadIdx, 6, 0, PRIMITIVETYPE_TRILIST );

		renderer->CreateVertexBuffer( (*outVtxBuf), 4, vtxDecl, 0 );

		(*outVtxBuf)->Lock();

		(*outVtxBuf)->SetElement( 0, hrVE_XYZ, hVec3( -wo2,  ho2, 0.25f ) );
		(*outVtxBuf)->SetElement( 0, hrVE_1UV, hVec2( 0.0f, 0.0f ) );

		(*outVtxBuf)->SetElement( 1, hrVE_XYZ, hVec3(  wo2,  ho2, 0.25f ) );
		(*outVtxBuf)->SetElement( 1, hrVE_1UV, hVec2( 1.0f, 0.0f ) );

		(*outVtxBuf)->SetElement( 2, hrVE_XYZ, hVec3( -wo2, -ho2, 0.25f ) );
		(*outVtxBuf)->SetElement( 2, hrVE_1UV, hVec2( 0.0f, 1.0f ) );

		(*outVtxBuf)->SetElement( 3, hrVE_XYZ, hVec3(  wo2, -ho2, 0.25f ) );
		(*outVtxBuf)->SetElement( 3, hrVE_1UV, hVec2( 1.0f, 1.0f ) );

		(*outVtxBuf)->Unlock();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void BuildConeMesh(	hUint16 segments, 
					    hFloat radius, 
						hFloat depth, 
						hRenderer* renderer, 
						hResourceHandle< hIndexBuffer >* outIdxBuf, 
						hResourceHandle< hVertexBuffer >* outVtxBuf )
	{
		if ( segments < 4)
		{
			segments = 4;
		}

		hUint16 vtxCnt = (hUint16)(segments+2);
		hUint16 idxCnt = (hUint16)(segments*6);
		hVertexDeclaration* vtxDecl;

		renderer->GetVertexDeclaration( vtxDecl, hrVF_XYZ | hrVF_NORMAL );
		renderer->CreateIndexBuffer( *outIdxBuf, NULL, idxCnt, 0, PRIMITIVETYPE_TRILIST, "ConeMeshIB" );
		renderer->CreateVertexBuffer( *outVtxBuf, vtxCnt, vtxDecl, 0, "ConeMeshVB" );

		hUint16 iidx = 0;
		hUint16 vidx = 0;
		(*outIdxBuf)->Lock();
		(*outVtxBuf)->Lock();

		hVec3* segmentPts = (hVec3*)alloca( sizeof(hVec3)*(segments+2) );
		hFloat dSegAngle = (2 * hmPI / segments);

		for ( hUint32 i = 0; i < segments; ++i )
		{
			float x = radius * sinf( dSegAngle*i );
			float y = radius * cosf( dSegAngle*i );

			segmentPts[i] = hVec3( x, y, depth );
			(*outVtxBuf)->SetElement( vidx++, hrVE_XYZ, segmentPts[i] );
		}
		segmentPts[segments] = hVec3( 0.0f, 0.0f, 0.0f );//Tip of cone
		(*outVtxBuf)->SetElement( vidx++, hrVE_XYZ, segmentPts[segments] );
		segmentPts[segments+1] = hVec3( 0.0f, 0.0f, depth );// centre base of cone
		(*outVtxBuf)->SetElement( vidx++, hrVE_XYZ, segmentPts[segments+1] );

		//Create the Cone
		for ( hUint16 i = 0; i < segments; ++i )
		{
			(*outIdxBuf)->SetIndex( iidx++, segments );//Tip
			(*outIdxBuf)->SetIndex( iidx++, (i+1)%segments  );//Base 1
			(*outIdxBuf)->SetIndex( iidx++, i );//Base 2
		}

		//Create the Base
		for ( hUint16 i = 0; i < segments; ++i )
		{
			(*outIdxBuf)->SetIndex( iidx++, segments+1 );//Base Centre
			(*outIdxBuf)->SetIndex( iidx++, i );//Base 1
			(*outIdxBuf)->SetIndex( iidx++, (i+1)%segments );//Base 2
		}

		(*outIdxBuf)->Unlock();
		(*outVtxBuf)->Unlock();
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
								   hResourceHandle< hIndexBuffer >* outIdxBuf, 
								   hResourceHandle< hVertexBuffer >* outVtxBuf )
	{
		if ( segments < 4)
		{
			segments = 4;
		}

		hUint32 dwColour = (hUint32)colour;

		hUint16 iidx = *startIdx;
		hUint16 vidx = *startVtx;

		hVec4* segmentPts = (hVec4*)alloca( sizeof(hVec4)*(segments+2) );
		hFloat dSegAngle = (2 * hmPI / segments);

		for ( hUint32 i = 0; i < segments; ++i )
		{
			float x = radius * sinf( dSegAngle*i );
			float y = radius * cosf( dSegAngle*i );

			segmentPts[i] = hVec4( x, y, lenght, 1.0f );
		}
		segmentPts[segments]   = hVec4( 0.0f, 0.0f, 0.0f, 1.0f );//Tip of cone
		segmentPts[segments+1] = hVec4( 0.0f, 0.0f, lenght, 1.0f );// centre base of cone

		hVec4 v;
		hMatrix wvp;
		//hMatrix::mult( transform, vp, &wvp );
        wvp = (*transform) * (*vp);

		for ( hUint16 i = 0; i < segments+2; ++i )
		{
			//hMatrix::mult( segmentPts[i], &wvp, v );
            v = segmentPts[i] * wvp;
			(*outVtxBuf)->SetElement( vidx, hrVE_XYZW, v );
			(*outVtxBuf)->SetElement( vidx++, hrVE_COLOR, dwColour );
		}

		//Create the Cone
		for ( hUint16 i = 0; i < segments; ++i )
		{
			(*outIdxBuf)->SetIndex( iidx++, (segments)+(*startVtx) );//Tip
			(*outIdxBuf)->SetIndex( iidx++, i+(*startVtx) );//Base 1
			(*outIdxBuf)->SetIndex( iidx++, ((i+1)%segments)+(*startVtx) );//Base 2
		}

		//Create the Base
		for ( hUint16 i = 0; i < segments; ++i )
		{
			(*outIdxBuf)->SetIndex( iidx++, (segments+1)+(*startVtx) );//Base Centre
			(*outIdxBuf)->SetIndex( iidx++, i+(*startVtx) );//Base 1
			(*outIdxBuf)->SetIndex( iidx++, ((i+1)%segments)+(*startVtx) );//Base 2
		}

		*startIdx = iidx;
		*startVtx = vidx;
	}

}
}