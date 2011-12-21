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
	hResourceHandle< hMaterial >		debugMat_;
	hResourceHandle< hIndexBuffer >		sphereIdxBuf_;
	hResourceHandle< hVertexBuffer >	sphereVtxBuf_;
	hResourceHandle< hVertexBuffer >	lineVtxBuf_;//< aabb's and screen rects go here
	hBool											debugDrawEnable_ = hFalse;
}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void	CreateAABBPoints( hVec4* points, const hAABB* aabb, const hMatrix& vp );
	bool	CheckLoaded()
	{
		bool loaded = true;
		loaded &= debugMat_.IsLoaded();
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
		if ( debugMat_.HasData() )
		{
			debugMat_.Release();
		}

		if ( sphereIdxBuf_.HasData() )
		{
			sphereIdxBuf_.Release();
		}

		if ( sphereVtxBuf_.HasData() )
		{
			sphereVtxBuf_.Release();
		}

		if ( lineVtxBuf_.HasData() )
		{
			lineVtxBuf_.Release();
		}
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
		hMatrix::orthoProjOffCentre( &op, 0, (hFloat)renderer->Width(), (hFloat)renderer->Height(), 0, 0.0f, 1.0f );

		renderer->NewRenderCommand< Cmd::SetMaterial >( debugMat_ );

		renderer->NewRenderCommand< Cmd::SetWorldMatrix >( &IdentityMatrix );
		renderer->NewRenderCommand< Cmd::SetViewMatrix >( &IdentityMatrix );
		renderer->NewRenderCommand< Cmd::SetProjectionMatrix >( &IdentityMatrix );

		if ( nSphereCommands_ > 0 || nConeCommands_ > 0 )
		{
			sphereIdxBuf_->Lock();
			sphereVtxBuf_->Lock();

			hUint16 sphereIdx = 0;
			hUint16 sphereVtx = 0;

			for ( hUint32 i = 0; i < nSphereCommands_; ++i )
			{
				Utility::BuildDebugSphereMesh( 
					sphereCommands_[i].centre_, 
					8, 
					6, 
					sphereCommands_[i].radius_, 
					&sphereIdx, 
					&sphereVtx, 
					sphereCommands_[i].colour_, 
					&vp, 
					&sphereIdxBuf_, 
					&sphereVtxBuf_ );
			}

			for ( hUint32 i = 0; i < nConeCommands_; ++i )
			{
				Utility::BuildDebugConeMesh(
					&coneCommands_[i].xf_, 
					8, 
					coneCommands_[i].radius_, 
					coneCommands_[i].lenght_, 
					&sphereIdx, 
					&sphereVtx, 
					coneCommands_[i].colour_, 
					&vp, 
					&sphereIdxBuf_, 
					&sphereVtxBuf_ );
			}

			sphereIdxBuf_->Unlock();
			sphereVtxBuf_->Unlock();

			renderer->NewRenderCommand< Cmd::SetStreams >( sphereIdxBuf_, sphereVtxBuf_ );
			renderer->NewRenderCommand< Cmd::DrawPrimative >( sphereIdx / 3 );
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
				hMatrix::mult( points[i2], &op, points[i2] );
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
			renderer->NewRenderCommand< Cmd::DrawVertexStream >( lineVtxBuf_, PRIMITIVETYPE_LINELIST );
		}

		ResetCommands();

		camera->Near( prevNear );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void CreateAABBPoints( hVec4* points, const hAABB* aabb, const hMatrix& vp )
	{
		////////////////////////////////////////////////////////////////////////
		//  aabb top
		// line 1
		// top left, far(+Z)
		points[ 0 ].x = aabb->c.x - aabb->r[ 0 ];
		points[ 0 ].y = aabb->c.y + aabb->r[ 1 ];
		points[ 0 ].z = aabb->c.z + aabb->r[ 2 ];
		points[ 0 ].w = 1.0f;

		//top left, close(-Z)
		points[ 1 ].x = aabb->c.x - aabb->r[ 0 ];
		points[ 1 ].y = aabb->c.y + aabb->r[ 1 ];
		points[ 1 ].z = aabb->c.z - aabb->r[ 2 ];
		points[ 1 ].w = 1.0f;

		//line 2
		//top left, close
		points[ 2 ].x = aabb->c.x - aabb->r[ 0 ];
		points[ 2 ].y = aabb->c.y + aabb->r[ 1 ];
		points[ 2 ].z = aabb->c.z - aabb->r[ 2 ];
		points[ 2 ].w = 1.0f;

		//top right, close
		points[ 3 ].x = aabb->c.x + aabb->r[ 0 ];
		points[ 3 ].y = aabb->c.y + aabb->r[ 1 ];
		points[ 3 ].z = aabb->c.z - aabb->r[ 2 ];
		points[ 3 ].w = 1.0f;

		//line 3
		//top right, close
		points[ 4 ].x = aabb->c.x + aabb->r[ 0 ];
		points[ 4 ].y = aabb->c.y + aabb->r[ 1 ];
		points[ 4 ].z = aabb->c.z - aabb->r[ 2 ];
		points[ 4 ].w = 1.0f;

		//top right, far
		points[ 5 ].x = aabb->c.x + aabb->r[ 0 ];
		points[ 5 ].y = aabb->c.y + aabb->r[ 1 ];
		points[ 5 ].z = aabb->c.z + aabb->r[ 2 ];
		points[ 5 ].w = 1.0f;

		//line 4
		//top right, far
		points[ 6 ].x = aabb->c.x + aabb->r[ 0 ];
		points[ 6 ].y = aabb->c.y + aabb->r[ 1 ];
		points[ 6 ].z = aabb->c.z + aabb->r[ 2 ];
		points[ 6 ].w = 1.0f;

		//top left, far
		points[ 7 ].x = aabb->c.x - aabb->r[ 0 ];
		points[ 7 ].y = aabb->c.y + aabb->r[ 1 ];
		points[ 7 ].z = aabb->c.z + aabb->r[ 2 ];
		points[ 7 ].w = 1.0f;

		// aabb top
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		// aabb bottom
		//line 1
		//bottom left, far(+Z)
		points[ 8 ].x = aabb->c.x - aabb->r[ 0 ];
		points[ 8 ].y = aabb->c.y - aabb->r[ 1 ];
		points[ 8 ].z = aabb->c.z + aabb->r[ 2 ];
		points[ 8 ].w = 1.0f;

		//bottom left, close(-Z)
		points[ 9 ].x = aabb->c.x - aabb->r[ 0 ];
		points[ 9 ].y = aabb->c.y - aabb->r[ 1 ];
		points[ 9 ].z = aabb->c.z - aabb->r[ 2 ];
		points[ 9 ].w = 1.0f;

		//line 2
		//bottom left, close
		points[ 10 ].x = aabb->c.x - aabb->r[ 0 ];
		points[ 10 ].y = aabb->c.y - aabb->r[ 1 ];
		points[ 10 ].z = aabb->c.z - aabb->r[ 2 ];
		points[ 10 ].w = 1.0f;

		//bottom right, close
		points[ 11 ].x = aabb->c.x + aabb->r[ 0 ];
		points[ 11 ].y = aabb->c.y - aabb->r[ 1 ];
		points[ 11 ].z = aabb->c.z - aabb->r[ 2 ];
		points[ 11 ].w = 1.0f;

		//line 3
		//bottom right, close
		points[ 12 ].x = aabb->c.x + aabb->r[ 0 ];
		points[ 12 ].y = aabb->c.y - aabb->r[ 1 ];
		points[ 12 ].z = aabb->c.z - aabb->r[ 2 ];
		points[ 12 ].w = 1.0f;

		//bottom right, far
		points[ 13 ].x = aabb->c.x + aabb->r[ 0 ];
		points[ 13 ].y = aabb->c.y - aabb->r[ 1 ];
		points[ 13 ].z = aabb->c.z + aabb->r[ 2 ];
		points[ 13 ].w = 1.0f;

		//line 4
		//bottom right, far
		points[ 14 ].x = aabb->c.x + aabb->r[ 0 ];
		points[ 14 ].y = aabb->c.y - aabb->r[ 1 ];
		points[ 14 ].z = aabb->c.z + aabb->r[ 2 ];
		points[ 14 ].w = 1.0f;

		//bottom left, far
		points[ 15 ].x = aabb->c.x - aabb->r[ 0 ];
		points[ 15 ].y = aabb->c.y - aabb->r[ 1 ];
		points[ 15 ].z = aabb->c.z + aabb->r[ 2 ];
		points[ 15 ].w = 1.0f;

		// aabb bottom
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		// aabb sides
		//line 1
		//top right, far
		points[ 16 ].x = aabb->c.x + aabb->r[ 0 ];
		points[ 16 ].y = aabb->c.y + aabb->r[ 1 ];
		points[ 16 ].z = aabb->c.z + aabb->r[ 2 ];
		points[ 16 ].w = 1.0f;

		//bottom right, far
		points[ 17 ].x = aabb->c.x + aabb->r[ 0 ];
		points[ 17 ].y = aabb->c.y - aabb->r[ 1 ];
		points[ 17 ].z = aabb->c.z + aabb->r[ 2 ];
		points[ 17 ].w = 1.0f;

		//line 2
		//top right, close
		points[ 18 ].x = aabb->c.x + aabb->r[ 0 ];
		points[ 18 ].y = aabb->c.y + aabb->r[ 1 ];
		points[ 18 ].z = aabb->c.z - aabb->r[ 2 ];
		points[ 18 ].w = 1.0f;

		//botom right, close
		points[ 19 ].x = aabb->c.x + aabb->r[ 0 ];
		points[ 19 ].y = aabb->c.y - aabb->r[ 1 ];
		points[ 19 ].z = aabb->c.z - aabb->r[ 2 ];
		points[ 19 ].w = 1.0f;

		//line 3
		//top left, close
		points[ 20 ].x = aabb->c.x - aabb->r[ 0 ];
		points[ 20 ].y = aabb->c.y + aabb->r[ 1 ];
		points[ 20 ].z = aabb->c.z - aabb->r[ 2 ];
		points[ 20 ].w = 1.0f;

		//bottom left, close
		points[ 21 ].x = aabb->c.x - aabb->r[ 0 ];
		points[ 21 ].y = aabb->c.y - aabb->r[ 1 ];
		points[ 21 ].z = aabb->c.z - aabb->r[ 2 ];
		points[ 21 ].w = 1.0f;

		//line 4
		//top left, far
		points[ 22 ].x = aabb->c.x - aabb->r[ 0 ];
		points[ 22 ].y = aabb->c.y + aabb->r[ 1 ];
		points[ 22 ].z = aabb->c.z + aabb->r[ 2 ];
		points[ 22 ].w = 1.0f;

		//bottom left, far
		points[ 23 ].x = aabb->c.x - aabb->r[ 0 ];
		points[ 23 ].y = aabb->c.y - aabb->r[ 1 ];
		points[ 23 ].z = aabb->c.z + aabb->r[ 2 ];
		points[ 23 ].w = 1.0f;

		// aabb sides
		////////////////////////////////////////////////////////////////////////

		for ( hUint32 i = 0; i < 24; ++i )
		{
			hMatrix::mult( points[i], &vp, points[i] );
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