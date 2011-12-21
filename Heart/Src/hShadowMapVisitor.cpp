/********************************************************************
	created:	2010/12/15
	created:	15:12:2010   17:34
	filename: 	GameShadowRenderVisitor.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hShadowMapVisitor.h"
#include "hSceneNodeBase.h"
#include "hSceneNodeCamera.h"
#include "hSceneNodeLocator.h"
#include "hSceneNodeMesh.h"
#include "hRenderTargetTexture.h"
#include "hResourceManager.h"

namespace Heart
{


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hShadowMapVisitor::hShadowMapVisitor() :
		cameraNode_( NULL )
		,renderer_( NULL )
		,sceneGraph_( NULL )
		,shadowTarget_( NULL )
		,depthTarget_( NULL )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hShadowMapVisitor::~hShadowMapVisitor()
	{
		Destroy();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hShadowMapVisitor::Initialise( Heart::hRenderer* prenderer, hResourceManager* manager )
	{
		renderer_ = prenderer;
// 		manager->GetResource( "engine/materials/shadowwrite.mat", shadowMaterial_ );
// 		hcAssert( shadowMaterial_.IsLoaded() );
// 		baisParam_ = shadowMaterial_->GetShaderParameter( "zBais" );
// 		hcAssert( baisParam_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hShadowMapVisitor::Destroy()
	{
		if ( shadowMaterial_.HasData() )
		{
			shadowMaterial_.Release();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hShadowMapVisitor::PreVisit( Heart::hSceneGraph* pSceneGraph )
	{
		using namespace Heart;

		hcAssert( renderer_ );

		nShadowCasters_ = 0;
		//Build frustum for light tests
		hVec3 eye( lightMatrix_.m41, lightMatrix_.m42, lightMatrix_.m43 );
		hVec3 up( lightMatrix_.m21, lightMatrix_.m22, lightMatrix_.m23 );
		hVec3 at( lightMatrix_.m31, lightMatrix_.m32, lightMatrix_.m33 );
		lightFrustum_.UpdateFromCamera( eye, eye+at, up, lightFOV_, 1, 0.01f, lightFalloff_, false );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hShadowMapVisitor::PostVisit( Heart::hSceneGraph* pSceneGraph )
	{
		if ( nShadowCasters_ == 0 )
		{
			return;
		}

		renderer_->NewRenderCommand< Cmd::BeginDebuggerEvent >( "Shadow Map Render" );

		hViewport vp = { 0, 0, shadowTarget_->Width(), shadowTarget_->Height() };
	 	hMatrix lv;
	 	hMatrix lp;
	 	hVec3 tFrustPoints[ 8 ];
	 	hVec3 minV, maxV;
	 	hMatrix ldirm, lproj;
	 
	 	// build a matrix looking in the lights direction
		hMatrix::inverse( &lightMatrix_, &ldirm );
	
		hFloat zBais = (lightFalloff_/500.0f)*0.001f;//is .5% good enough, need config?
		zBais = zBais < 0.0001f ? 0.0001f : zBais;

	 	//build a projection matrix with these Z values
		//Matrix::perspective( &lproj, minV.x, maxV.x, minV.y, maxV.y, minV.z - (zBais*100.0f), maxV.z + (zBais*100.0f) );
		hMatrix::perspectiveFOV( &lproj, lightFOV_, 1, /*minV.z - */(zBais*100.0f), lightFalloff_ + (zBais*100.0f) );
	 
		lp = lproj;
	 	lv = ldirm;
	 
 	 	//build the shadow projection matrix 
 	 	//(inverse view matrix)*( light view matrix )*( projection matrix )
 	 	hMatrix iv, tmp;
 	 	hMatrix::inverse( cameraNode_->GetViewMatrix(), &iv );
 	 	hMatrix::mult( &iv, &lv, &tmp );
 	 	hMatrix::mult( &tmp, &lproj, &shadowMatrix_ );
 	
		renderer_->NewRenderCommand< Cmd::SetRenderTarget >( 0, shadowTarget_ );
		renderer_->NewRenderCommand< Cmd::SetDepthBuffer >( depthTarget_ );
 	 	renderer_->NewRenderCommand< Cmd::ClearScreen >( hColour( 1.0f, 1.0f, 1.0f, 1.0f ) );
 	 	renderer_->NewRenderCommand< Cmd::SetViewport >( vp );
 	 	renderer_->NewRenderCommand< Cmd::SetViewMatrix >( &ldirm );
 	 	renderer_->NewRenderCommand< Cmd::SetProjectionMatrix >( &lproj );
		renderer_->NewRenderCommand< Cmd::SetMaterial >( shadowMaterial_ );
 	 
		renderer_->NewRenderCommand< Cmd::SetMaterialFloatParameter >( shadowMaterial_, baisParam_, zBais );
 	 
 	 	//render into the shadow map
 	 	for ( hUint32 i = 0; i < nShadowCasters_; ++i )
 	 	{
			if ( shadowCasters_[i].castShadows_ )
			{
				const hMesh* mesh = shadowCasters_[ i ].meshData_;

				renderer_->NewRenderCommand< Cmd::SetStreams >( mesh->GetIndexBuffer(), mesh->GetVertexBuffer() );
				renderer_->NewRenderCommand< Cmd::SetWorldMatrix >( &shadowCasters_[ i ].matrix_ );
				renderer_->NewRenderCommand< Cmd::DrawPrimative >( mesh->GetPrimativeCount() );
			}
	 	}

		renderer_->NewRenderCommand< Cmd::SetRenderTarget >( 0, (hRenderTargetTexture*)NULL );
		renderer_->NewRenderCommand< Cmd::SetDepthBuffer >( (hRenderTargetTexture*)NULL );

		renderer_->NewRenderCommand< Cmd::EndDebuggerEvent >();
		renderer_->NewRenderCommand< Cmd::IncrementRenderStatsPass >();
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hShadowMapVisitor::Visit( hSceneNodeMesh& visit )
	{
		if ( lightFrustum_.TestAABB( *visit.GetGlobalAABB() ) )
		{
			//store for later
			hUint32 c = visit.GetMeshCount();
			if ( (nShadowCasters_ + c) < MAX_SHADOW_CASTERS )
			{
				for ( hUint32 i = 0; i < c; ++i )
				{
					shadowCasters_[nShadowCasters_].aabb_ = *visit.GetGlobalAABB();
					shadowCasters_[nShadowCasters_].meshData_ = visit.GetMesh(i);
					shadowCasters_[nShadowCasters_].matrix_ = *visit.GetGlobalMatrix();
					shadowCasters_[nShadowCasters_].castShadows_ = visit.GetCastShadows();
					++nShadowCasters_;
				}
			}
		}
		else
		{
			AbortChildVisit( hTrue );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hShadowMapVisitor::Visit( hSceneNodeLocator& visit )
	{
		if ( !lightFrustum_.TestAABB( *visit.GetGlobalAABB() ))
		{
			AbortChildVisit( hTrue );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hShadowMapVisitor::Visit( hSceneNodeCamera& visit )
	{
		if ( !lightFrustum_.TestAABB( *visit.GetGlobalAABB() ))
		{
			AbortChildVisit( hTrue );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSunShadowMapVisitor::Visit( hSceneNodeMesh& visit )
	{
		if ( cameraNode_->GetViewFrustum()->TestMovingAABB( *visit.GetGlobalAABB(), lightDir_ ) )
		{
			//store for later
			hUint32 c = visit.GetMeshCount();
			if ( (nShadowCasters_ + c) < MAX_SHADOW_CASTERS )
			{
				for ( hUint32 i = 0; i < c; ++i )
				{
					shadowCasters_[nShadowCasters_].aabb_ = *visit.GetLocalAABB();
					shadowCasters_[nShadowCasters_].meshData_ = visit.GetMesh(i);
					shadowCasters_[nShadowCasters_].matrix_ = *visit.GetGlobalMatrix();
					shadowCasters_[nShadowCasters_].castShadows_ = visit.GetCastShadows();
					++nShadowCasters_;
				}
			}
		}
		else
		{
			AbortChildVisit( hTrue );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSunShadowMapVisitor::Visit( hSceneNodeLocator& visit )
	{
		if ( !cameraNode_->GetViewFrustum()->TestMovingAABB( *visit.GetGlobalAABB(), lightDir_ ))
		{
			AbortChildVisit( hTrue );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSunShadowMapVisitor::Visit( hSceneNodeCamera& visit )
	{
		if ( !cameraNode_->GetViewFrustum()->TestMovingAABB( *visit.GetGlobalAABB(), lightDir_ ))
		{
			AbortChildVisit( hTrue );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSunShadowMapVisitor::PreVisit( Heart::hSceneGraph* pSceneGraph )
	{

		hcAssert( renderer_ );

		nShadowCasters_ = 0;
		// store the light direction for aabb tests
		hVec3 at = hVec3( lightMatrix_.m31, lightMatrix_.m32, lightMatrix_.m33 );
		cameraEye_ = hMatrix::GetTranslation( cameraNode_->GetGlobalMatrix() );
		lightDir_ = at*1000.0f;

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSunShadowMapVisitor::PostVisit( Heart::hSceneGraph* pSceneGraph )
	{
		if ( nShadowCasters_ == 0 )
		{
			return;
		}

		renderer_->NewRenderCommand< Cmd::BeginDebuggerEvent >( "Shadow Map Render" );

		hViewport vp = { 0, 0, shadowTarget_->Width(), shadowTarget_->Height() };
		hMatrix lv;
		hMatrix lp;
		hVec3 teye( lightMatrix_.m41, lightMatrix_.m42, lightMatrix_.m43 );
		hVec3 tat ( lightMatrix_.m31, lightMatrix_.m32, lightMatrix_.m33 );
		hVec3 tup ( lightMatrix_.m21, lightMatrix_.m22, lightMatrix_.m23 );
		hVec3 tFrustPoints[ 8 ];
		hVec3 minV, maxV;
		hMatrix ldirm, lproj;
		hViewFrustum* pFrust = cameraNode_->GetViewFrustum();

		minV.x = FLT_MAX;
		minV.y = FLT_MAX;
		minV.z = FLT_MAX;

		maxV.x = -FLT_MAX;
		maxV.y = -FLT_MAX;
		maxV.z = -FLT_MAX;

		// build a matrix looking in the lights direction
		hMatrix::lookAt( &ldirm, ZeroVector3, tat, tup );

		//find the min & max Z for the othro projection matrix
		//find the bounds for the clip & projection matrix
		for ( hUint32 i = 0; i < nShadowCasters_; ++i )
		{
			Heart::hAABB& aabb = shadowCasters_[ i ].aabb_;
			hVec3 v[8],xfv[8];

			v[ 0 ].x = aabb.c.x + aabb.r[ 0 ];
			v[ 0 ].y = aabb.c.y + aabb.r[ 1 ];
			v[ 0 ].z = aabb.c.z + aabb.r[ 2 ];

			v[ 1 ].x = aabb.c.x - aabb.r[ 0 ];
			v[ 1 ].y = aabb.c.y + aabb.r[ 1 ];
			v[ 1 ].z = aabb.c.z + aabb.r[ 2 ];

			v[ 2 ].x = aabb.c.x + aabb.r[ 0 ];
			v[ 2 ].y = aabb.c.y - aabb.r[ 1 ];
			v[ 2 ].z = aabb.c.z + aabb.r[ 2 ];

			v[ 3 ].x = aabb.c.x - aabb.r[ 0 ];
			v[ 3 ].y = aabb.c.y - aabb.r[ 1 ];
			v[ 3 ].z = aabb.c.z + aabb.r[ 2 ];

			v[ 4 ].x = aabb.c.x + aabb.r[ 0 ];
			v[ 4 ].y = aabb.c.y + aabb.r[ 1 ];
			v[ 4 ].z = aabb.c.z - aabb.r[ 2 ];

			v[ 5 ].x = aabb.c.x - aabb.r[ 0 ];
			v[ 5 ].y = aabb.c.y + aabb.r[ 1 ];
			v[ 5 ].z = aabb.c.z - aabb.r[ 2 ];

			v[ 6 ].x = aabb.c.x + aabb.r[ 0 ];
			v[ 6 ].y = aabb.c.y - aabb.r[ 1 ];
			v[ 6 ].z = aabb.c.z - aabb.r[ 2 ];

			v[ 7 ].x = aabb.c.x - aabb.r[ 0 ];
			v[ 7 ].y = aabb.c.y - aabb.r[ 1 ];
			v[ 7 ].z = aabb.c.z - aabb.r[ 2 ];

			for ( hUint32 iV = 0; iV < 8; ++iV )
			{
				hMatrix::mult( v[ iV ], &ldirm, xfv[ iV ] );

				maxV.x = hMax( xfv[ iV ].x, maxV.x );
				maxV.y = hMax( xfv[ iV ].y, maxV.y );
				maxV.z = hMax( xfv[ iV ].z, maxV.z );

				minV.x = hMin( xfv[ iV ].x, minV.x );
				minV.y = hMin( xfv[ iV ].y, minV.y );
				minV.z = hMin( xfv[ iV ].z, minV.z );
			}
		}

		hFloat zBais = ((maxV.z-minV.z)/500.0f)*0.001f;//is .5% good enough, need config?
		zBais = zBais < 0.0001f ? 0.0001f : zBais;

		//build a projection matrix with these Z values
		hMatrix::orthoProjOffCentre( &lproj, minV.x, maxV.x, minV.y, maxV.y, minV.z - (zBais*100.0f), maxV.z + (zBais*100.0f) );

		hMatrix clipm;
		hMatrix::identity( &clipm );

		clipm.m11 = 2 / ( maxV.x - minV.x );
		clipm.m22 = 2 / ( maxV.y - minV.y );
		clipm.m14 = -0.5f*( maxV.x + minV.x )*clipm.m11;
		clipm.m24 = -0.5f*( maxV.y + minV.y )*clipm.m22;


		hMatrix::mult( &clipm, &lproj, &lp );
		lv = ldirm;

		//build the shadow projection matrix 
		//(inverse view matrix)*( light view matrix )*( projection matrix )
		hMatrix iv, tmp;
		hMatrix::inverse( cameraNode_->GetViewMatrix(), &iv );
		hMatrix::mult( &iv, &lv, &tmp );
		hMatrix::mult( &tmp, &lproj, &shadowMatrix_ );

		renderer_->NewRenderCommand< Cmd::SetRenderTarget >( 0, shadowTarget_ );
		renderer_->NewRenderCommand< Cmd::SetDepthBuffer >( depthTarget_ );
		renderer_->NewRenderCommand< Cmd::ClearScreen >( hColour( 1.0f, 1.0f, 1.0f, 1.0f ) );
		renderer_->NewRenderCommand< Cmd::SetViewport >( vp );
		renderer_->NewRenderCommand< Cmd::SetViewMatrix >( &ldirm );
		renderer_->NewRenderCommand< Cmd::SetProjectionMatrix >( &lproj );
		renderer_->NewRenderCommand< Cmd::SetMaterial >( shadowMaterial_ );

		renderer_->NewRenderCommand< Cmd::SetMaterialFloatParameter >( shadowMaterial_, baisParam_, zBais );

		//render into the shadow map
		for ( hUint32 i = 0; i < nShadowCasters_; ++i )
		{
			if ( shadowCasters_[i].castShadows_ )
			{
				const hMesh* mesh = shadowCasters_[ i ].meshData_;

				renderer_->NewRenderCommand< Cmd::SetStreams >( mesh->GetIndexBuffer(), mesh->GetVertexBuffer() );
				renderer_->NewRenderCommand< Cmd::SetWorldMatrix >( &shadowCasters_[ i ].matrix_ );
				renderer_->NewRenderCommand< Cmd::DrawPrimative >( mesh->GetPrimativeCount() );
			}
		}

		renderer_->NewRenderCommand< Cmd::SetRenderTarget >( 0, (hRenderTargetTexture*)NULL );
		renderer_->NewRenderCommand< Cmd::SetDepthBuffer >( (hRenderTargetTexture*)NULL );

		renderer_->NewRenderCommand< Cmd::EndDebuggerEvent >();
		renderer_->NewRenderCommand< Cmd::IncrementRenderStatsPass >();
	}

}