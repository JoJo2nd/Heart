/********************************************************************

	filename: 	DebugRenderer.cpp	
	
	Copyright (c) 4:6:2011 James Moran
	
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

#include "Common.h"
#include "hDebugRenderer.h"
#include "hVector.h"
#include "hAABB.h"
#include "hRendererConstants.h"
#include "hRenderer.h"
#include "hRenderUtility.h"
#include "hResourceManager.h"
#include "hSceneNodeCamera.h"
#include "hMatrix.h"
#include "hVertexBuffer.h"
#include "hIndexBuffer.h"

namespace Heart
{
namespace DebugRenderer
{
namespace 
{
#define TEXT_BUFFER_SIZE			(1024*1024)
#define MAX_TEXT_COMMANDS			(256)
#define MAX_SPHERE_COMMANDS			(120)
#define MAX_CONE_COMMANDS			(120)
#define MAX_AABB_COMMANDS			(256)
#define MAX_SCREENRECT_COMMANDS		(128)

	//Private Data
	struct TextCommand
	{
		hFloat			x_;
		hFloat			y_;
		const char*		text_;
	};

	struct SphereCommand
	{
		hVec3		centre_;
		hFloat			radius_;
		hColour	colour_;
	};

	struct ConeCommand 
	{
		hMatrix	xf_;
		hFloat			lenght_;
		hFloat			radius_;
		hColour	colour_;
	};

	struct AABBCommand
	{
		hAABB		aabb_;
		hColour	colour_;
	};

	struct ScreenRectCommand
	{
		hRect	rect_;
		hColour	colour_;
	};

	hUint32											nTextCommands_;
	TextCommand										textCommands_[MAX_TEXT_COMMANDS];
	hUint32											nSphereCommands_;
	SphereCommand									sphereCommands_[MAX_SPHERE_COMMANDS];
	hUint32											nAABBCommands_;
	AABBCommand										aabbCommands_[MAX_AABB_COMMANDS];
	hUint32											nScreenRectCommands_;
	ScreenRectCommand								screenRectCommands_[MAX_SCREENRECT_COMMANDS];
	hUint32											nConeCommands_;
	ConeCommand										coneCommands_[MAX_CONE_COMMANDS];
	hMaterial*		                                debugMat_;
	hIndexBuffer*		                            sphereIdxBuf_;
	hVertexBuffer*	                                sphereVtxBuf_;
	hVertexBuffer*	                                lineVtxBuf_;//< aabb's and screen rects go here
	hBool											debugDrawEnable_ = hFalse;
}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void	CreateAABBPoints( hVec4* points, const hAABB* aabb, const hMatrix& vp );
	bool	CheckLoaded()
	{
		bool loaded = true;
		//loaded &= debugMat_.IsLoaded();
		return loaded;
	}
	void ResetCommands();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Initialise( hResourceManager* resourceManager, hRenderer* renderer )
	{
		//JM TODO:
// 		hVertexDeclaration* vtxDecl;
// 		resourceManager->GetResource( "engine/materials/debugmaterial.mat", debugMat_ );
// 
// 		renderer->GetVertexDeclaration( vtxDecl, hrVF_XYZW | hrVF_COLOR );
// 
// 		renderer->CreateVertexBuffer( lineVtxBuf_, MAX_AABB_COMMANDS*MAX_SCREENRECT_COMMANDS*32, vtxDecl, DYNAMIC, "debugVtxBuf" );
// 
// 		renderer->CreateIndexBuffer( sphereIdxBuf_, NULL, MAX_SPHERE_COMMANDS*512, DYNAMIC, PRIMITIVETYPE_TRILIST, "debugSphereIdxBuf" );
// 		renderer->CreateVertexBuffer( sphereVtxBuf_, MAX_SPHERE_COMMANDS*512, vtxDecl, DYNAMIC, "debugSphereVtxBuf" );

		ResetCommands();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Destory()
	{
// 		if ( debugMat_.HasData() )
// 		{
// 			debugMat_.Release();
// 		}
// 
// 		if ( sphereIdxBuf_.HasData() )
// 		{
// 			sphereIdxBuf_.Release();
// 		}
// 
// 		if ( sphereVtxBuf_.HasData() )
// 		{
// 			sphereVtxBuf_.Release();
// 		}
// 
// 		if ( lineVtxBuf_.HasData() )
// 		{
// 			lineVtxBuf_.Release();
// 		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RenderDebugText( hFloat x, hFloat y, const char* fmt, ... )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RenderDebugSphere( const hVec3& centre, hFloat radius, const hColour& colour )
	{
		if ( nSphereCommands_ >= MAX_SPHERE_COMMANDS )
		{
			hcWarningHigh( true, "Debug Sphere draw command list full" );
			return;
		}

		sphereCommands_[nSphereCommands_].centre_ = centre;
		sphereCommands_[nSphereCommands_].radius_ = radius;
		sphereCommands_[nSphereCommands_].colour_ = colour;
		++nSphereCommands_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RenderDebugCone( const hMatrix& transform, hFloat radius, hFloat len, const hColour& colour )
	{
		if ( nConeCommands_ >= MAX_CONE_COMMANDS )
		{
			hcWarningHigh( true, "Debug Cone draw command list full" );
			return;
		}

		coneCommands_[nConeCommands_].xf_ = transform;
		coneCommands_[nConeCommands_].lenght_ = len;
		coneCommands_[nConeCommands_].radius_ = tan(radius)*len;
		coneCommands_[nConeCommands_].colour_ = colour;
		++nConeCommands_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RenderDebugAABB( const hAABB& aabb, const hColour& colour )
	{
		if ( nAABBCommands_ >= MAX_AABB_COMMANDS )
		{
			//hcWarningHigh( true, "Debug aabb draw command list full" );
			return;
		}

		aabbCommands_[nAABBCommands_].aabb_ = aabb;
		aabbCommands_[nAABBCommands_].colour_ = colour;
		++nAABBCommands_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RenderDebugScreenQuad( const hRect& rect, const hColour& colour )
	{
		if ( nScreenRectCommands_ >= MAX_SCREENRECT_COMMANDS )
		{
			hcWarningHigh( true, "Debug screen rect draw command list full" );
			return;
		}

		screenRectCommands_[nScreenRectCommands_].rect_ = rect;
		screenRectCommands_[nScreenRectCommands_].colour_ = colour;
		++nScreenRectCommands_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RenderDebugRenderCalls( hRenderer* renderer, hSceneNodeCamera* camera )
	{
		if ( !CheckLoaded() || !debugDrawEnable_ )
		{
			ResetCommands();

			return;
		}

		lineVtxBuf_->Lock();

		hUint32 lineIndexCount = 0;
		hFloat prevNear = camera->Near();
		camera->Near( 0.0f );
		camera->UpdateCamera();
		hMatrix vp = *camera->GetViewProjectionMatrix();
		hMatrix op;
        op = hMatrixFunc::orthoProjOffCentre( 0, (hFloat)renderer->GetWidth(), (hFloat)renderer->GetHeight(), 0, 0.0f, 1.0f );

#ifdef HEART_OLD_RENDER_SUBMISSION
		renderer->NewRenderCommand< Cmd::SetMaterial >( debugMat_ );

        renderer->NewRenderCommand< Cmd::SetWorldMatrix >( &hMatrixFunc::identity() );
		renderer->NewRenderCommand< Cmd::SetViewMatrix >( &hMatrixFunc::identity() );
		renderer->NewRenderCommand< Cmd::SetProjectionMatrix >( &hMatrixFunc::identity() );
#endif // HEART_OLD_RENDER_SUBMISSION

		if ( nSphereCommands_ > 0 || nConeCommands_ > 0 )
		{
			//sphereIdxBuf_->Lock();
			//sphereVtxBuf_->Lock();

			hUint16 sphereIdx = 0;
			hUint16 sphereVtx = 0;

			for ( hUint32 i = 0; i < nSphereCommands_; ++i )
			{
// 				Utility::BuildDebugSphereMesh( 
// 					sphereCommands_[i].centre_, 
// 					8, 
// 					6, 
// 					sphereCommands_[i].radius_, 
// 					&sphereIdx, 
// 					&sphereVtx, 
// 					sphereCommands_[i].colour_, 
// 					&vp, 
// 					&sphereIdxBuf_, 
// 					&sphereVtxBuf_ );
			}

			for ( hUint32 i = 0; i < nConeCommands_; ++i )
			{
// 				Utility::BuildDebugConeMesh(
// 					&coneCommands_[i].xf_, 
// 					8, 
// 					coneCommands_[i].radius_, 
// 					coneCommands_[i].lenght_, 
// 					&sphereIdx, 
// 					&sphereVtx, 
// 					coneCommands_[i].colour_, 
// 					&vp, 
// 					&sphereIdxBuf_, 
// 					&sphereVtxBuf_ );
			}

			//sphereIdxBuf_->Unlock();
			//sphereVtxBuf_->Unlock();

#ifdef HEART_OLD_RENDER_SUBMISSION
			renderer->NewRenderCommand< Cmd::SetStreams >( sphereIdxBuf_, sphereVtxBuf_ );
			renderer->NewRenderCommand< Cmd::DrawPrimative >( sphereIdx / 3 );
#endif
		}

		for ( hUint32 i = 0; i < nAABBCommands_; ++i )
		{
			hVec4 points[24];

			CreateAABBPoints( points, &aabbCommands_[i].aabb_, vp );

			for ( hUint32 i2 = 0; i2 < 24; ++i2 )
			{
				lineVtxBuf_->SetElement( lineIndexCount, hrVE_XYZW, points[i2] );
				lineVtxBuf_->SetElement( lineIndexCount, hrVE_COLOR, (hUint32)aabbCommands_[i].colour_ );
				++lineIndexCount;
			}
		}

		for ( hUint32 i = 0; i < nScreenRectCommands_; ++i )
		{
			const hRect& r = screenRectCommands_[i].rect_;
			hVec4 points[] =
			{
				hVec4( (hFloat)r.left_ , (hFloat)r.top_   , 0.0f, 1.0f ),
				hVec4( (hFloat)r.right_, (hFloat)r.top_   , 0.0f, 1.0f ),
				hVec4( (hFloat)r.right_, (hFloat)r.bottom_, 0.0f, 1.0f ),
				hVec4( (hFloat)r.left_ , (hFloat)r.bottom_, 0.0f, 1.0f ),
			};

			for ( hUint32 i2 = 0; i2 < 4; ++i2 )
			{
				points[i2] = points[i2] * op;
			}

			lineVtxBuf_->SetElement( lineIndexCount, hrVE_XYZW, points[0] );
			lineVtxBuf_->SetElement( lineIndexCount, hrVE_COLOR, (hUint32)screenRectCommands_[i].colour_ );
			++lineIndexCount;
			lineVtxBuf_->SetElement( lineIndexCount, hrVE_XYZW, points[1] );
			lineVtxBuf_->SetElement( lineIndexCount, hrVE_COLOR, (hUint32)screenRectCommands_[i].colour_ );
			++lineIndexCount;

			lineVtxBuf_->SetElement( lineIndexCount, hrVE_XYZW, points[1]);
			lineVtxBuf_->SetElement( lineIndexCount, hrVE_COLOR, (hUint32)screenRectCommands_[i].colour_ );
			++lineIndexCount;
			lineVtxBuf_->SetElement( lineIndexCount, hrVE_XYZW, points[2] );
			lineVtxBuf_->SetElement( lineIndexCount, hrVE_COLOR, (hUint32)screenRectCommands_[i].colour_ );
			++lineIndexCount;

			lineVtxBuf_->SetElement( lineIndexCount, hrVE_XYZW, points[2] );
			lineVtxBuf_->SetElement( lineIndexCount, hrVE_COLOR, (hUint32)screenRectCommands_[i].colour_ );
			++lineIndexCount;
			lineVtxBuf_->SetElement( lineIndexCount, hrVE_XYZW, points[3] );
			lineVtxBuf_->SetElement( lineIndexCount, hrVE_COLOR, (hUint32)screenRectCommands_[i].colour_ );
			++lineIndexCount;

			lineVtxBuf_->SetElement( lineIndexCount, hrVE_XYZW, points[3] );
			lineVtxBuf_->SetElement( lineIndexCount, hrVE_COLOR, (hUint32)screenRectCommands_[i].colour_ );
			++lineIndexCount;
			lineVtxBuf_->SetElement( lineIndexCount, hrVE_XYZW, points[0] );
			lineVtxBuf_->SetElement( lineIndexCount, hrVE_COLOR, (hUint32)screenRectCommands_[i].colour_ );
			++lineIndexCount;
		}
		
		lineVtxBuf_->Unlock();

		if ( nScreenRectCommands_ > 0 || nAABBCommands_ > 0 )
		{
#ifdef HEART_OLD_RENDER_SUBMISSION
			renderer->NewRenderCommand< Cmd::DrawVertexStream >( lineVtxBuf_, PRIMITIVETYPE_LINELIST );
#endif // HEART_OLD_RENDER_SUBMISSION
		}

		ResetCommands();

		camera->Near( prevNear );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void CreateAABBPoints( hVec4* points, const hAABB* aabb, const hMatrix& vp )
	{
        hCPUVec3 aabbc = aabb->c_;
        hCPUVec3 aabbr = aabb->r_;
        hCPUVec4 tmpPoints[24];

		////////////////////////////////////////////////////////////////////////
		//  aabb top
		// line 1
		// top left, far(+Z)
		tmpPoints[ 0 ].x = aabbc.x - aabbr.x;
		tmpPoints[ 0 ].y = aabbc.y + aabbr.y;
		tmpPoints[ 0 ].z = aabbc.z + aabbr.z;
		tmpPoints[ 0 ].w = 1.0f;

		//top left, close(-Z)
		tmpPoints[ 1 ].x = aabbc.x - aabbr.x;
		tmpPoints[ 1 ].y = aabbc.y + aabbr.y;
		tmpPoints[ 1 ].z = aabbc.z - aabbr.z;
		tmpPoints[ 1 ].w = 1.0f;

		//line 2
		//top left, close
		tmpPoints[ 2 ].x = aabbc.x - aabbr.x;
		tmpPoints[ 2 ].y = aabbc.y + aabbr.y;
		tmpPoints[ 2 ].z = aabbc.z - aabbr.z;
		tmpPoints[ 2 ].w = 1.0f;

		//top right, close
		tmpPoints[ 3 ].x = aabbc.x + aabbr.x;
		tmpPoints[ 3 ].y = aabbc.y + aabbr.y;
		tmpPoints[ 3 ].z = aabbc.z - aabbr.z;
		tmpPoints[ 3 ].w = 1.0f;

		//line 3
		//top right, close
		tmpPoints[ 4 ].x = aabbc.x + aabbr.x;
		tmpPoints[ 4 ].y = aabbc.y + aabbr.y;
		tmpPoints[ 4 ].z = aabbc.z - aabbr.z;
		tmpPoints[ 4 ].w = 1.0f;

		//top right, far
		tmpPoints[ 5 ].x = aabbc.x + aabbr.x;
		tmpPoints[ 5 ].y = aabbc.y + aabbr.y;
		tmpPoints[ 5 ].z = aabbc.z + aabbr.z;
		tmpPoints[ 5 ].w = 1.0f;

		//line 4
		//top right, far
		tmpPoints[ 6 ].x = aabbc.x + aabbr.x;
		tmpPoints[ 6 ].y = aabbc.y + aabbr.y;
		tmpPoints[ 6 ].z = aabbc.z + aabbr.z;
		tmpPoints[ 6 ].w = 1.0f;

		//top left, far
		tmpPoints[ 7 ].x = aabbc.x - aabbr.x;
		tmpPoints[ 7 ].y = aabbc.y + aabbr.y;
		tmpPoints[ 7 ].z = aabbc.z + aabbr.z;
		tmpPoints[ 7 ].w = 1.0f;

		// aabb top
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		// aabb bottom
		//line 1
		//bottom left, far(+Z)
		tmpPoints[ 8 ].x = aabbc.x - aabbr.x;
		tmpPoints[ 8 ].y = aabbc.y - aabbr.y;
		tmpPoints[ 8 ].z = aabbc.z + aabbr.z;
		tmpPoints[ 8 ].w = 1.0f;

		//bottom left, close(-Z)
		tmpPoints[ 9 ].x = aabbc.x - aabbr.x;
		tmpPoints[ 9 ].y = aabbc.y - aabbr.y;
		tmpPoints[ 9 ].z = aabbc.z - aabbr.z;
		tmpPoints[ 9 ].w = 1.0f;

		//line 2
		//bottom left, close
		tmpPoints[ 10 ].x = aabbc.x - aabbr.x;
		tmpPoints[ 10 ].y = aabbc.y - aabbr.y;
		tmpPoints[ 10 ].z = aabbc.z - aabbr.z;
		tmpPoints[ 10 ].w = 1.0f;

		//bottom right, close
		tmpPoints[ 11 ].x = aabbc.x + aabbr.x;
		tmpPoints[ 11 ].y = aabbc.y - aabbr.y;
		tmpPoints[ 11 ].z = aabbc.z - aabbr.z;
		tmpPoints[ 11 ].w = 1.0f;

		//line 3
		//bottom right, close
		tmpPoints[ 12 ].x = aabbc.x + aabbr.x;
		tmpPoints[ 12 ].y = aabbc.y - aabbr.y;
		tmpPoints[ 12 ].z = aabbc.z - aabbr.z;
		tmpPoints[ 12 ].w = 1.0f;

		//bottom right, far
		tmpPoints[ 13 ].x = aabbc.x + aabbr.x;
		tmpPoints[ 13 ].y = aabbc.y - aabbr.y;
		tmpPoints[ 13 ].z = aabbc.z + aabbr.z;
		tmpPoints[ 13 ].w = 1.0f;

		//line 4
		//bottom right, far
		tmpPoints[ 14 ].x = aabbc.x + aabbr.x;
		tmpPoints[ 14 ].y = aabbc.y - aabbr.y;
		tmpPoints[ 14 ].z = aabbc.z + aabbr.z;
		tmpPoints[ 14 ].w = 1.0f;

		//bottom left, far
		tmpPoints[ 15 ].x = aabbc.x - aabbr.x;
		tmpPoints[ 15 ].y = aabbc.y - aabbr.y;
		tmpPoints[ 15 ].z = aabbc.z + aabbr.z;
		tmpPoints[ 15 ].w = 1.0f;

		// aabb bottom
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		// aabb sides
		//line 1
		//top right, far
		tmpPoints[ 16 ].x = aabbc.x + aabbr.x;
		tmpPoints[ 16 ].y = aabbc.y + aabbr.y;
		tmpPoints[ 16 ].z = aabbc.z + aabbr.z;
		tmpPoints[ 16 ].w = 1.0f;

		//bottom right, far
		tmpPoints[ 17 ].x = aabbc.x + aabbr.x;
		tmpPoints[ 17 ].y = aabbc.y - aabbr.y;
		tmpPoints[ 17 ].z = aabbc.z + aabbr.z;
		tmpPoints[ 17 ].w = 1.0f;

		//line 2
		//top right, close
		tmpPoints[ 18 ].x = aabbc.x + aabbr.x;
		tmpPoints[ 18 ].y = aabbc.y + aabbr.y;
		tmpPoints[ 18 ].z = aabbc.z - aabbr.z;
		tmpPoints[ 18 ].w = 1.0f;

		//botom right, close
		tmpPoints[ 19 ].x = aabbc.x + aabbr.x;
		tmpPoints[ 19 ].y = aabbc.y - aabbr.y;
		tmpPoints[ 19 ].z = aabbc.z - aabbr.z;
		tmpPoints[ 19 ].w = 1.0f;

		//line 3
		//top left, close
		tmpPoints[ 20 ].x = aabbc.x - aabbr.x;
		tmpPoints[ 20 ].y = aabbc.y + aabbr.y;
		tmpPoints[ 20 ].z = aabbc.z - aabbr.z;
		tmpPoints[ 20 ].w = 1.0f;

		//bottom left, close
		tmpPoints[ 21 ].x = aabbc.x - aabbr.x;
		tmpPoints[ 21 ].y = aabbc.y - aabbr.y;
		tmpPoints[ 21 ].z = aabbc.z - aabbr.z;
		tmpPoints[ 21 ].w = 1.0f;

		//line 4
		//top left, far
		tmpPoints[ 22 ].x = aabbc.x - aabbr.x;
		tmpPoints[ 22 ].y = aabbc.y + aabbr.y;
		tmpPoints[ 22 ].z = aabbc.z + aabbr.z;
		tmpPoints[ 22 ].w = 1.0f;

		//bottom left, far
		tmpPoints[ 23 ].x = aabbc.x - aabbr.x;
		tmpPoints[ 23 ].y = aabbc.y - aabbr.y;
		tmpPoints[ 23 ].z = aabbc.z + aabbr.z;
		tmpPoints[ 23 ].w = 1.0f;

		// aabb sides
		////////////////////////////////////////////////////////////////////////

		for ( hUint32 i = 0; i < 24; ++i )
		{
			points[i] = hMatrixFunc::mult( (hVec4)tmpPoints[i], vp );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ResetCommands()
	{
		nAABBCommands_ = 0;
		nScreenRectCommands_ = 0;
		nSphereCommands_ = 0;
		nConeCommands_ = 0;
		nTextCommands_ = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void EnableDebugDrawing( hBool val )
	{
		debugDrawEnable_ = val;
		hcPrintf( "Debug Drawing is %s", val ? "Enabled" : "Disabled" );
	}

}
}