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
		hcAssert( renderer_ );

		nShadowCasters_ = 0;
		//Build frustum for light tests
		hVec3 eye = lightMatrix_.r[3];
        hVec3 up = lightMatrix_.r[1];
		hVec3 at = lightMatrix_.r[2];
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

#ifdef HEART_OLD_RENDER_SUBMISSION
		renderer_->NewRenderCommand< Cmd::BeginDebuggerEvent >( "Shadow Map Render" );
#endif // HEART_OLD_RENDER_SUBMISSION

        hViewport vp( 0, 0, shadowTarget_->Width(), shadowTarget_->Height() );
	 	hMatrix lv;
	 	hMatrix lp;
	 	hVec3 tFrustPoints[ 8 ];
	 	hVec3 minV, maxV;
	 	hMatrix ldirm, lproj;
	 
	 	// build a matrix looking in the lights direction
		ldirm = hMatrixFunc::inverse( lightMatrix_ );
	
		hFloat zBais = (lightFalloff_/500.0f)*0.001f;//is .5% good enough, need config?
		zBais = zBais < 0.0001f ? 0.0001f : zBais;

	 	//build a projection matrix with these Z values
		//Matrix::perspective( &lproj, minV.x, maxV.x, minV.y, maxV.y, minV.z - (zBais*100.0f), maxV.z + (zBais*100.0f) );
		lproj = hMatrixFunc::perspectiveFOV( lightFOV_, 1, /*minV.z - */(zBais*100.0f), lightFalloff_ + (zBais*100.0f) );
	 
		lp = lproj;
	 	lv = ldirm;
	 
 	 	//build the shadow projection matrix 
 	 	//(inverse view matrix)*( light view matrix )*( projection matrix )
 	 	hMatrix iv, tmp;
 	 	iv = hMatrixFunc::inverse( *cameraNode_->GetViewMatrix() );
 	 	//hMatrixFunc::mult( &iv, &lv, &tmp );
 	 	//hMatrixFunc::mult( &tmp, &lproj, &shadowMatrix_ );
 	 	shadowMatrix_ = iv * lv * lproj;
 	
#ifdef HEART_OLD_RENDER_SUBMISSION
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
#endif // HEART_OLD_RENDER_SUBMISSION
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
		hVec3 at = lightMatrix_.r[2];//hVec3( lightMatrix_.m31, lightMatrix_.m32, lightMatrix_.m33 );
		cameraEye_ = cameraNode_->GetGlobalMatrix()->r[3];//hMatrixFunc::GetTranslation( *cameraNode_->GetGlobalMatrix() );
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

#ifdef HEART_OLD_RENDER_SUBMISSION
		renderer_->NewRenderCommand< Cmd::BeginDebuggerEvent >( "Shadow Map Render" );
#endif // HEART_OLD_RENDER_SUBMISSION

		hViewport vp( 0, 0, shadowTarget_->Width(), shadowTarget_->Height() );
		hMatrix lv;
		hMatrix lp;
		hVec3 teye = lightMatrix_.r[3];//( lightMatrix_.m41, lightMatrix_.m42, lightMatrix_.m43 );
		hVec3 tat  = lightMatrix_.r[2];//( lightMatrix_.m31, lightMatrix_.m32, lightMatrix_.m33 );
		hVec3 tup  = lightMatrix_.r[1];//( lightMatrix_.m21, lightMatrix_.m22, lightMatrix_.m23 );
		hVec3 tFrustPoints[ 8 ];
		hCPUVec3 minV( -FLT_MAX, -FLT_MAX, -FLT_MAX ), maxV( FLT_MAX, FLT_MAX, FLT_MAX );
		hMatrix ldirm, lproj;
		hViewFrustum* pFrust = cameraNode_->GetViewFrustum();

// 		minV.x = FLT_MAX;
// 		minV.y = FLT_MAX;
// 		minV.z = FLT_MAX;
// 
// 		maxV.x = -FLT_MAX;
// 		maxV.y = -FLT_MAX;
// 		maxV.z = -FLT_MAX;

		// build a matrix looking in the lights direction
        ldirm = hMatrixFunc::LookAt( hVec3Func::zeroVector(), tat, tup );

		//find the min & max Z for the othro projection matrix
		//find the bounds for the clip & projection matrix
		for ( hUint32 i = 0; i < nShadowCasters_; ++i )
		{
			hCPUVec3 aabbc = shadowCasters_[ i ].aabb_.c_;
            hCPUVec3 aabbr = shadowCasters_[ i ].aabb_.r_;
			hCPUVec3 v[8],xfv[8];

			v[ 0 ].x = aabbc.x + aabbr.x;
			v[ 0 ].y = aabbc.y + aabbr.y;
			v[ 0 ].z = aabbc.z + aabbr.z;

			v[ 1 ].x = aabbc.x - aabbr.x;
			v[ 1 ].y = aabbc.y + aabbr.y;
			v[ 1 ].z = aabbc.z + aabbr.z;

			v[ 2 ].x = aabbc.x + aabbr.x;
			v[ 2 ].y = aabbc.y - aabbr.y;
			v[ 2 ].z = aabbc.z + aabbr.z;

			v[ 3 ].x = aabbc.x - aabbr.x;
			v[ 3 ].y = aabbc.y - aabbr.y;
			v[ 3 ].z = aabbc.z + aabbr.z;

			v[ 4 ].x = aabbc.x + aabbr.x;
			v[ 4 ].y = aabbc.y + aabbr.y;
			v[ 4 ].z = aabbc.z - aabbr.z;

			v[ 5 ].x = aabbc.x - aabbr.x;
			v[ 5 ].y = aabbc.y + aabbr.y;
			v[ 5 ].z = aabbc.z - aabbr.z;

			v[ 6 ].x = aabbc.x + aabbr.x;
			v[ 6 ].y = aabbc.y - aabbr.y;
			v[ 6 ].z = aabbc.z - aabbr.z;

			v[ 7 ].x = aabbc.x - aabbr.x;
			v[ 7 ].y = aabbc.y - aabbr.y;
			v[ 7 ].z = aabbc.z - aabbr.z;

			for ( hUint32 iV = 0; iV < 8; ++iV )
			{
				xfv[ iV ] = hMatrixFunc::mult( (hVec3)v[ iV ], ldirm );

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
		lproj = hMatrixFunc::orthoProjOffCentre( minV.x, maxV.x, minV.y, maxV.y, minV.z - (zBais*100.0f), maxV.z + (zBais*100.0f) );

		hMatrix clipm;
		clipm = hMatrixFunc::identity();
/*
		clipm.m11 = 2 / ( maxV.x - minV.x );
		clipm.m22 = 2 / ( maxV.y - minV.y );
		clipm.m14 = -0.5f*( maxV.x + minV.x )*clipm.m11;
		clipm.m24 = -0.5f*( maxV.y + minV.y )*clipm.m22;


		hMatrix::mult( &clipm, &lproj, &lp );
*/
		lv = ldirm;

		//build the shadow projection matrix 
		//(inverse view matrix)*( light view matrix )*( projection matrix )
		hMatrix iv, tmp;
		iv = hMatrixFunc::inverse( *cameraNode_->GetViewMatrix() );
		tmp = hMatrixFunc::mult( iv, lv );
		shadowMatrix_ = hMatrixFunc::mult( tmp, lproj );

#ifdef HEART_OLD_RENDER_SUBMISSION
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
#endif // HEART_OLD_RENDER_SUBMISSION
	}

}