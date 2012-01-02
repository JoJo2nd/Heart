/********************************************************************

	filename: 	DeferredRenderVisitor.cpp	
	
	Copyright (c) 23:5:2011 James Moran
	
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
#include "hDeferredRenderVisitor.h"
#include "hRenderTargetTexture.h"
#include "hSceneGraph.h"
#include "hSceneNodeMesh.h"
#include "hSceneNodeLocator.h"
#include "hSceneNodeCamera.h"
#include "hSceneNodeLight.h"
#include "hRenderUtility.h"

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hDeferredRenderVisitor::~hDeferredRenderVisitor()
	{
		Destroy();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hDeferredRenderVisitor::PreVisit( hSceneGraph* pSceneGraph )
	{
		hcAssert( camera_ );

		nLights_ = 0;
		nMeshes_ = 0;

		if ( !meshes_ )
		{
			meshes_ = (RenderInstance*)hGeneralHeap.alignAlloc( sizeof(RenderInstance)*meshLimit_, 16, __FILE__, __LINE__ );
			new ( meshes_ ) RenderInstance[meshLimit_];
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hDeferredRenderVisitor::PostVisit( hSceneGraph* pSceneGraph )
	{
		//TODO: sort lights by type
		//TODO: sort meshes by material
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hDeferredRenderVisitor::Visit( hSceneNodeMesh& visit )
	{
		if ( !camera_->GetViewFrustum()->TestAABB( *visit.GetGlobalAABB() ) )
		{
			AbortChildVisit( hTrue );
			return;
		}

		hUint32 c = visit.GetMeshCount();
		if ( (nMeshes_ + c) < meshLimit_ )
		{
			for ( hUint32 i = 0; i < c; ++i )
			{
				hMesh* m = visit.GetMesh(i);
				meshes_[nMeshes_].material_ = m->GetMaterial();
				meshes_[nMeshes_].meshData_ = *m;
				meshes_[nMeshes_].matrix_ = *visit.GetGlobalMatrix();
				++nMeshes_;
			}
		}

		Heart::DebugRenderer::RenderDebugAABB( *visit.GetGlobalAABB(), WHITE );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hDeferredRenderVisitor::Visit( hSceneNodeLocator& visit )
	{
		if ( !camera_->GetViewFrustum()->TestAABB( *visit.GetGlobalAABB() ) )
		{
			AbortChildVisit( hTrue );
			return;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hDeferredRenderVisitor::Visit( hSceneNodeCamera& visit )
	{
		if ( !camera_->GetViewFrustum()->TestAABB( *visit.GetGlobalAABB() ) )
		{
			AbortChildVisit( hTrue );
			return;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hDeferredRenderVisitor::Visit( hSceneNodeLight& visit )
	{
		if ( !camera_->GetViewFrustum()->TestAABB( *visit.GetGlobalAABB() ) && visit.GetLightType() != LightType_DIRECTION )
		{
			AbortChildVisit( hTrue );
			return;
		}

		if ( nLights_ < MAX_LIGHTS )
		{
			lights_[nLights_].matrix_		= *visit.GetGlobalMatrix();
			lights_[nLights_].type_			= visit.GetLightType();
			lights_[nLights_].power_		= visit.GetLightPower();
			lights_[nLights_].exp_			= visit.GetSpecularExponent();
			lights_[nLights_].diffuse_		= visit.GetDiffuse();
			lights_[nLights_].shadowCaster_ = visit.GetCastShadows();
			lights_[nLights_].inside_		= hAABB::PointWithinAABBSphere( hMatrixFunc::getTranslation( *camera_->GetGlobalMatrix() ), *visit.GetGlobalAABB() );

			if ( visit.GetLightType() == LightType_POINT )
			{
				lights_[nLights_].minRadius_	= visit.GetMinRadius();
				lights_[nLights_].maxRadius_	= visit.GetMaxRadius();

				Heart::DebugRenderer::RenderDebugSphere( hMatrixFunc::getTranslation( *visit.GetGlobalMatrix() ), visit.GetMinRadius(), visit.GetDiffuse() );
				Heart::DebugRenderer::RenderDebugSphere( hMatrixFunc::getTranslation( *visit.GetGlobalMatrix() ), visit.GetMaxRadius(), visit.GetDiffuse() );
				Heart::DebugRenderer::RenderDebugAABB( *visit.GetGlobalAABB(), visit.GetDiffuse() );
			}
			else if ( visit.GetLightType() == LightType_SPOT ) 
			{
				lights_[nLights_].minRadius_	= cos( visit.GetInnerAngleRad() );
				lights_[nLights_].maxRadius_	= cos( visit.GetOuterAngleRad() );
				lights_[nLights_].spotFalloff_	= visit.GetSpotFalloffDist();
				lights_[nLights_].spotAngleRad_ = visit.GetOuterAngleRad();

				Heart::DebugRenderer::RenderDebugCone( *visit.GetGlobalMatrix(), visit.GetInnerAngleRad(), visit.GetSpotFalloffDist(), visit.GetDiffuse() );
				Heart::DebugRenderer::RenderDebugCone( *visit.GetGlobalMatrix(), visit.GetOuterAngleRad(), visit.GetSpotFalloffDist(), visit.GetDiffuse() );
				Heart::DebugRenderer::RenderDebugAABB( *visit.GetGlobalAABB(), visit.GetDiffuse() );
			}

			++nLights_;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hDeferredRenderVisitor::RenderGBuffers( hRenderer* renderer, hSceneGraph* pSceneGraph )
	{

		hcAssert( camera_ );

		//cameraEye_ = Matrix::GetTranslation( pCameraNode_->GetGlobalMatrix() );

#ifdef HEART_OLD_RENDER_SUBMISSION
		renderer->NewRenderCommand< Cmd::BeginDebuggerEvent >( "G Buffer Pass" );

		renderer->NewRenderCommand< Cmd::SetRenderTarget >( 0, renderTargets_[0] );
		renderer->NewRenderCommand< Cmd::SetRenderTarget >( 1, renderTargets_[1] );
		renderer->NewRenderCommand< Cmd::SetRenderTarget >( 2, renderTargets_[2] );
		//renderer->NewRenderCommand< Cmd::SetRenderTarget >( 3, pRenderTargets_[3] );
		renderer->NewRenderCommand< Cmd::SetDepthBuffer >( depthTarget_ );
		renderer->NewRenderCommand< Cmd::ClearScreen >( hColour( 1.0f, 1.0f, 1.0f, 1.0f ) );

		renderer->NewRenderCommand< Cmd::SetViewport >( camera_->GetViewport() );
		renderer->NewRenderCommand< Cmd::SetViewMatrix >( camera_->GetViewMatrix() );
		renderer->NewRenderCommand< Cmd::SetProjectionMatrix >( camera_->GetProjectionMatrix() );


		for ( hUint32 i = 0; i < nMeshes_; ++i )
		{
			renderer->NewRenderCommand< Cmd::SetMaterial >( meshes_[i].material_ );
			renderer->NewRenderCommand< Cmd::SetStreams >( meshes_[i].meshData_.GetIndexBuffer(), meshes_[i].meshData_.GetVertexBuffer() );
			renderer->NewRenderCommand< Cmd::SetWorldMatrix >( &meshes_[i].matrix_ );
			renderer->NewRenderCommand< Cmd::DrawPrimative >( meshes_[i].meshData_.GetPrimativeCount() );
		}

		renderer->NewRenderCommand< Cmd::SetRenderTarget >( 0, (hRenderTargetTexture*)NULL );
		renderer->NewRenderCommand< Cmd::SetRenderTarget >( 1, (hRenderTargetTexture*)NULL );
		renderer->NewRenderCommand< Cmd::SetRenderTarget >( 2, (hRenderTargetTexture*)NULL );
		//renderer->NewRenderCommand< Cmd::SetRenderTarget >( 3, (RenderTargetTexture*)NULL );
		renderer->NewRenderCommand< Cmd::SetDepthBuffer >( (hRenderTargetTexture*)NULL );

		renderer->NewRenderCommand< Cmd::EndDebuggerEvent >();
#endif // HEART_OLD_RENDER_SUBMISSION
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hDeferredRenderVisitor::RenderLightPasses( hRenderer* renderer, hSceneGraph* pSceneGraph )
	{

		hMatrix proj,iproj;
		iproj = hMatrixFunc::inverse( *camera_->GetProjectionMatrix() );
#ifdef HEART_OLD_RENDER_SUBMISSION
		renderer->NewRenderCommand< Cmd::SetMaterialFloat4x4Parameter >( dirLightMatWShadow_, inverseCamProjParam_, (hFloat*)&iproj );
		renderer->NewRenderCommand< Cmd::SetMaterialFloat4x4Parameter >( pointLightInsideMat_, inverseCamProjPointParam_, (hFloat*)&iproj );

		//Clear Final Target
		renderer->NewRenderCommand< Cmd::ClearScreen >( hColour( 0.0f, 0.0f, 0.0f, 1.0f ) );
		//TODO: Render Baked Light Pass
		renderer->NewRenderCommand< Cmd::BeginDebuggerEvent >( "Baked Light Pass" );
		renderer->NewRenderCommand< Cmd::EndDebuggerEvent >();
#endif // HEART_OLD_RENDER_SUBMISSION
		//Render Light Pass (x nLights_)
		for ( hUint32 i = 0; i < nLights_; ++i )
		{
			if ( lights_[i].type_ == LightType_DIRECTION )
			{
				if ( lights_[i].shadowCaster_ )
				{
#ifdef HEART_OLD_RENDER_SUBMISSION
					renderer->NewRenderCommand< Cmd::BeginDebuggerEvent >( "Dir Light Shadow Pass" );
#endif // HEART_OLD_RENDER_SUBMISSION
					//Create a Shadow Map of the scene.
					//Means another visit of the scene -_-
					sunShadowVisitor_.SetLightMatrix( &lights_[i].matrix_ );
					sunShadowVisitor_.SetCameraNode( camera_ );
					sunShadowVisitor_.SetDepthTarget( shadowDepthTarget_ );
					sunShadowVisitor_.SetShadowTarget( shadowTarget_ );
					pSceneGraph->VisitScene( &sunShadowVisitor_, hSceneGraph::TOP_DOWN, false );

					proj =hMatrixFunc::orthoProj( (hFloat)renderer->Width(), (hFloat)renderer->Height(), 0.0f, 10.0f );

                    hVec3 wvDir,dir( hMatrixFunc::getRow( lights_[i].matrix_, 2 ) );
					hMatrix wvi,wvit;
					wvi = hMatrixFunc::inverse( *camera_->GetViewMatrix() );
					wvit = hMatrixFunc::transpose( wvi );
					wvDir = hMatrixFunc::multNormal( dir, wvit );
					wvDir = hVec3Func::normaliseFast( wvDir );

#ifdef HEART_OLD_RENDER_SUBMISSION
 					renderer->NewRenderCommand< Cmd::SetMaterialFloat4x4Parameter >( dirLightMatWShadow_, dirShadowMatrixParm_, (hFloat*)sunShadowVisitor_.GetShadowMatrix() );
					renderer->NewRenderCommand< Cmd::SetDirectionLight >( wvDir, lights_[i].power_, lights_[i].diffuse_, WHITE, lights_[i].exp_ );

                    renderer->NewRenderCommand< Cmd::SetWorldMatrix >( &hMatrixFunc::identity() );
					renderer->NewRenderCommand< Cmd::SetViewMatrix >( &hMatrixFunc::identity() );
					renderer->NewRenderCommand< Cmd::SetProjectionMatrix >( &proj );

					renderer->NewRenderCommand< Cmd::SetMaterial >( dirLightMatWShadow_ );
					renderer->NewRenderCommand< Cmd::SetStreams >( dirLightGeom_.idxBuf_, dirLightGeom_.vtxBuf_ );
					renderer->NewRenderCommand< Cmd::DrawPrimative >( 2 );

					renderer->NewRenderCommand< Cmd::EndDebuggerEvent >();
#endif // HEART_OLD_RENDER_SUBMISSION

				}
				else
				{
#ifdef HEART_OLD_RENDER_SUBMISSION
					renderer->NewRenderCommand< Cmd::BeginDebuggerEvent >( "Dir Light Pass" );

					renderer->NewRenderCommand< Cmd::SetDepthBuffer >( depthTarget_ );
#endif // HEART_OLD_RENDER_SUBMISSION

					proj = hMatrixFunc::orthoProj( (hFloat)renderer->Width(), (hFloat)renderer->Height(), 0.0f, 10.0f );

					hVec3 wvDir,dir(hMatrixFunc::getTranslation( lights_[i].matrix_ ) );
					hMatrix wvi,wvit;
					wvi = hMatrixFunc::inverse( *camera_->GetViewMatrix() );
					wvit = hMatrixFunc::transpose( wvi );
					wvDir = hMatrixFunc::multNormal( dir, wvit );
					wvDir = hVec3Func::normaliseFast( wvDir );

#ifdef HEART_OLD_RENDER_SUBMISSION
					renderer->NewRenderCommand< Cmd::SetDirectionLight >( wvDir, lights_[i].power_, lights_[i].diffuse_, WHITE, lights_[i].exp_ );

                    renderer->NewRenderCommand< Cmd::SetWorldMatrix >( &hMatrixFunc::identity() );
					renderer->NewRenderCommand< Cmd::SetViewMatrix >( &hMatrixFunc::identity() );
					renderer->NewRenderCommand< Cmd::SetProjectionMatrix >( &proj );

					renderer->NewRenderCommand< Cmd::SetMaterial >( dirLightMat_ );
					renderer->NewRenderCommand< Cmd::SetStreams >( dirLightGeom_.idxBuf_, dirLightGeom_.vtxBuf_ );
					renderer->NewRenderCommand< Cmd::DrawPrimative >( 2 );

					renderer->NewRenderCommand< Cmd::EndDebuggerEvent >();
#endif // HEART_OLD_RENDER_SUBMISSION
				}
			}
			else if ( lights_[i].type_ == LightType_POINT )
			{
				hVec3 vspos;
				vspos = hMatrixFunc::mult( hMatrixFunc::getTranslation( lights_[i].matrix_ ), *camera_->GetViewMatrix() );

				hMatrix scale,world;
				scale = hMatrixFunc::scale( lights_[i].maxRadius_*1.01f, lights_[i].maxRadius_*1.01f, lights_[i].maxRadius_*1.01f );

				world = hMatrixFunc::mult( scale, lights_[i].matrix_ );
		
#ifdef HEART_OLD_RENDER_SUBMISSION
				renderer->NewRenderCommand< Cmd::BeginDebuggerEvent >( "Point Light Pass" );

				renderer->NewRenderCommand< Cmd::SetDepthBuffer >( depthTarget_ );
				renderer->NewRenderCommand< Cmd::SetWorldMatrix >( &world );
				renderer->NewRenderCommand< Cmd::SetViewMatrix >( camera_->GetViewMatrix() );
				renderer->NewRenderCommand< Cmd::SetProjectionMatrix >( camera_->GetProjectionMatrix() );
				renderer->NewRenderCommand< Cmd::SetPointLight >( 
					vspos,
					lights_[i].minRadius_, lights_[i].maxRadius_, lights_[i].power_, lights_[i].diffuse_, WHITE, lights_[i].exp_ );

				renderer->NewRenderCommand< Cmd::SetStreams >( pointLightGeom_.idxBuf_, pointLightGeom_.vtxBuf_ );

				//light it up! :P
				renderer->NewRenderCommand< Cmd::SetMaterial >( pointLightInsideMat_ );
				renderer->NewRenderCommand< Cmd::DrawPrimative >( pointLightGeom_.nPrims_ );

				renderer->NewRenderCommand< Cmd::EndDebuggerEvent >();
#endif // HEART_OLD_RENDER_SUBMISSION
			}
			else if ( lights_[i].type_ == LightType_SPOT )
			{
				hVec3 vspos,vsdir;
				vspos = hMatrixFunc::mult( hMatrixFunc::getTranslation( lights_[i].matrix_ ), *camera_->GetViewMatrix() );
                vsdir = hMatrixFunc::multNormal( (hVec3)hMatrixFunc::getRow( lights_[i].matrix_, 2 ), *camera_->GetViewMatrix() );
				vsdir = hVec3Func::normaliseFast( vsdir );

				hMatrix scale,world;
				hFloat xyscale = tan( lights_[i].spotAngleRad_ )*lights_[i].spotFalloff_;
				scale = hMatrixFunc::scale( xyscale*2.0f, xyscale*2.0f, lights_[i].spotFalloff_ );

				world = hMatrixFunc::mult( scale, lights_[i].matrix_ );

				if ( lights_[i].shadowCaster_ )
				{
#ifdef HEART_OLD_RENDER_SUBMISSION
					renderer->NewRenderCommand< Cmd::BeginDebuggerEvent >( "Spot Light Pass" );
#endif // HEART_OLD_RENDER_SUBMISSION
					//Create a Shadow Map of the scene.
					//Means another visit of the scene -_-
					shadowVisitor_.SetLightMatrix( &lights_[i].matrix_ );
					shadowVisitor_.SetLightParameters( lights_[i].spotAngleRad_*2, lights_[i].spotFalloff_ );
					shadowVisitor_.SetCameraNode( camera_ );
					shadowVisitor_.SetDepthTarget( shadowDepthTarget_ );
					shadowVisitor_.SetShadowTarget( shadowTarget_ );
					pSceneGraph->VisitScene( &shadowVisitor_, hSceneGraph::TOP_DOWN, false );

#ifdef HEART_OLD_RENDER_SUBMISSION
					renderer->NewRenderCommand< Cmd::SetDepthBuffer >( depthTarget_ );
					renderer->NewRenderCommand< Cmd::SetWorldMatrix >( &world );
					renderer->NewRenderCommand< Cmd::SetViewMatrix >( camera_->GetViewMatrix() );
					renderer->NewRenderCommand< Cmd::SetProjectionMatrix >( camera_->GetProjectionMatrix() );
					renderer->NewRenderCommand< Cmd::SetStreams >( spotLightGeom_.idxBuf_, spotLightGeom_.vtxBuf_ );
					renderer->NewRenderCommand< Cmd::SetMaterialFloat4x4Parameter >( spotLightShadow2ndMat_, spotShadowMatrixParm_, (hFloat*)shadowVisitor_.GetShadowMatrix() );
					renderer->NewRenderCommand< Cmd::SetSpotLight >( 
						vspos, vsdir,
						1.0f-lights_[i].minRadius_, 
						1.0f-lights_[i].maxRadius_, 
						lights_[i].spotFalloff_, 
						lights_[i].power_, 
						lights_[i].diffuse_, 
						WHITE, 
						lights_[i].exp_ );

					//light it up! :P
					//renderer->NewRenderCommand< Cmd::SetMaterial >( spotLight1stMat_ );
					//renderer->NewRenderCommand< Cmd::DrawPrimative >( spotLightGeom_.nPrims_ );

					renderer->NewRenderCommand< Cmd::SetMaterial >( spotLightShadow2ndMat_ );
					renderer->NewRenderCommand< Cmd::DrawPrimative >( spotLightGeom_.nPrims_ );

					renderer->NewRenderCommand< Cmd::EndDebuggerEvent >();
#endif // HEART_OLD_RENDER_SUBMISSION
				}
				else
				{

#ifdef HEART_OLD_RENDER_SUBMISSION
					renderer->NewRenderCommand< Cmd::BeginDebuggerEvent >( "Spot Light Pass" );

					renderer->NewRenderCommand< Cmd::SetWorldMatrix >( &world );
					renderer->NewRenderCommand< Cmd::SetViewMatrix >( camera_->GetViewMatrix() );
					renderer->NewRenderCommand< Cmd::SetProjectionMatrix >( camera_->GetProjectionMatrix() );
					renderer->NewRenderCommand< Cmd::SetStreams >( spotLightGeom_.idxBuf_, spotLightGeom_.vtxBuf_ );
					renderer->NewRenderCommand< Cmd::SetSpotLight >( 
						vspos, vsdir,
						1.0f-lights_[i].minRadius_, 
						1.0f-lights_[i].maxRadius_, 
						lights_[i].spotFalloff_, 
						lights_[i].power_, 
						lights_[i].diffuse_, 
						WHITE, 
						lights_[i].exp_ );

					//light it up! :P
					//renderer->NewRenderCommand< Cmd::SetMaterial >( spotLight1stMat_ );
					//renderer->NewRenderCommand< Cmd::DrawPrimative >( spotLightGeom_.nPrims_ );

					renderer->NewRenderCommand< Cmd::SetMaterial >( spotLight2ndMat_ );
					renderer->NewRenderCommand< Cmd::DrawPrimative >( spotLightGeom_.nPrims_ );

					renderer->NewRenderCommand< Cmd::EndDebuggerEvent >();
#endif // HEART_OLD_RENDER_SUBMISSION
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void hDeferredRenderVisitor::Initialise( hRenderer* renderer, hResourceManager* manager )
	{
// 		manager->GetResource( "engine/materials/pointlight1st.mat", pointLight1stMat_ );
// 		manager->GetResource( "engine/materials/pointlight2nd.mat", pointLight2ndMat_ );
// 		manager->GetResource( "engine/materials/pointlightinside.mat", pointLightInsideMat_ );
// 		manager->GetResource( "engine/materials/directionlight.mat", dirLightMat_ );
// 		manager->GetResource( "engine/materials/directionlightshadow.mat", dirLightMatWShadow_ );
// 		manager->GetResource( "engine/materials/spotlight1st.mat", spotLight1stMat_ );
// 		manager->GetResource( "engine/materials/spotlight2nd.mat", spotLight2ndMat_ );
// 		manager->GetResource( "engine/materials/spotlightshadow2nd.mat", spotLightShadow2ndMat_ );
// 
// 		//These are "required" resources so should be loaded by the engine
// 		hcAssert( pointLight1stMat_.IsLoaded() );
// 		hcAssert( pointLight2ndMat_.IsLoaded() );
// 		hcAssert( pointLightInsideMat_.IsLoaded() );
// 		hcAssert( dirLightMat_.IsLoaded() );
// 		hcAssert( dirLightMatWShadow_.IsLoaded() );
// 		hcAssert( spotLight1stMat_.IsLoaded() );
// 		hcAssert( spotLight2ndMat_.IsLoaded() );
// 		hcAssert( spotLightShadow2ndMat_.IsLoaded() );
// 
// 		dirShadowMatrixParm_ = dirLightMatWShadow_->GetShaderParameter( "shadowMatrix" );
// 		spotShadowMatrixParm_ = spotLightShadow2ndMat_->GetShaderParameter( "shadowMatrix" );
// 		inverseCamProjParam_ = dirLightMatWShadow_->GetShaderParameter( "invProj" );
// 		inverseCamProjPointParam_ = pointLightInsideMat_->GetShaderParameter( "invProj" );
// 
// 		hcAssert( dirShadowMatrixParm_ );
// 		hcAssert( spotShadowMatrixParm_ );
// 		hcAssert( inverseCamProjParam_ );
// 		hcAssert( inverseCamProjPointParam_ );
// 
// 		hUint32 w = renderer->Width();
// 		hUint32 h = renderer->Height();
// 		hUint32 sw = 1024;
// 		hUint32 sh = 1024;
// 		renderer->CreateRenderTarget( renderTargets_[0], w, h, TFORMAT_ARGB8, "Deferred01" );
// 		renderer->CreateRenderTarget( renderTargets_[1], w, h, TFORMAT_GR16F, "Deferred02" );
// 		renderer->CreateRenderTarget( renderTargets_[2], w, h, TFORMAT_R32F,  "Deferred03" );
// 		renderer->CreateRenderTarget( renderTargets_[3], w, h, TFORMAT_ARGB8, "Deferred04" );
// 		renderer->CreateRenderTarget( depthTarget_, w, h, TFORMAT_D24S8F, "DeferredDepth" );
// 		renderer->CreateRenderTarget( shadowTarget_, sw, sh, TFORMAT_R32F, "ShadowTarget" );
// 		renderer->CreateRenderTarget( shadowDepthTarget_, sw, sh, TFORMAT_D24S8F, "ShadowDepth" );
// 
// 		Utility::BuildFullscreenQuadMesh( renderer, &dirLightGeom_.idxBuf_, &dirLightGeom_.vtxBuf_ );
// 		Utility::BuildSphereMesh( 8, 8, 1, renderer, &pointLightGeom_.idxBuf_, &pointLightGeom_.vtxBuf_ );
// 		Utility::BuildConeMesh( 8, hmDegToRad( 45 ), 1, renderer, &spotLightGeom_.idxBuf_, &spotLightGeom_.vtxBuf_ );
// 		pointLightGeom_.nPrims_ = pointLightGeom_.idxBuf_->IndexCount() / 3;
// 		spotLightGeom_.nPrims_ = spotLightGeom_.idxBuf_->IndexCount() / 3;
// 
// 		renderer->NewRenderCommand< Cmd::BindTexture >( dirLightMat_, dirLightMat_->GetShaderParameter( "MRT0" ), renderTargets_[0] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( dirLightMat_, dirLightMat_->GetShaderParameter( "MRT1" ), renderTargets_[1] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( dirLightMat_, dirLightMat_->GetShaderParameter( "MRT2" ), renderTargets_[2] );
// 
// 		renderer->NewRenderCommand< Cmd::BindTexture >( pointLight2ndMat_, pointLight2ndMat_->GetShaderParameter( "MRT0" ), renderTargets_[0] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( pointLight2ndMat_, pointLight2ndMat_->GetShaderParameter( "MRT1" ), renderTargets_[1] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( pointLight2ndMat_, pointLight2ndMat_->GetShaderParameter( "MRT2" ), renderTargets_[2] );
// 
// 		renderer->NewRenderCommand< Cmd::BindTexture >( spotLight2ndMat_, spotLight2ndMat_->GetShaderParameter( "MRT0" ), renderTargets_[0] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( spotLight2ndMat_, spotLight2ndMat_->GetShaderParameter( "MRT1" ), renderTargets_[1] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( spotLight2ndMat_, spotLight2ndMat_->GetShaderParameter( "MRT2" ), renderTargets_[2] );
// 
// 		renderer->NewRenderCommand< Cmd::BindTexture >( spotLightShadow2ndMat_, spotLightShadow2ndMat_->GetShaderParameter( "MRT0" ), renderTargets_[0] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( spotLightShadow2ndMat_, spotLightShadow2ndMat_->GetShaderParameter( "MRT1" ), renderTargets_[1] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( spotLightShadow2ndMat_, spotLightShadow2ndMat_->GetShaderParameter( "MRT2" ), renderTargets_[2] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( spotLightShadow2ndMat_, spotLightShadow2ndMat_->GetShaderParameter( "ShadowMap" ), shadowTarget_ );
// 
// 		renderer->NewRenderCommand< Cmd::BindTexture >( pointLightInsideMat_, pointLightInsideMat_->GetShaderParameter( "MRT0" ), renderTargets_[0] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( pointLightInsideMat_, pointLightInsideMat_->GetShaderParameter( "MRT1" ), renderTargets_[1] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( pointLightInsideMat_, pointLightInsideMat_->GetShaderParameter( "MRT2" ), renderTargets_[2] );
// 
// 		renderer->NewRenderCommand< Cmd::BindTexture >( dirLightMatWShadow_, dirLightMatWShadow_->GetShaderParameter( "MRT0" ), renderTargets_[0] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( dirLightMatWShadow_, dirLightMatWShadow_->GetShaderParameter( "MRT1" ), renderTargets_[1] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( dirLightMatWShadow_, dirLightMatWShadow_->GetShaderParameter( "MRT2" ), renderTargets_[2] );
// 		renderer->NewRenderCommand< Cmd::BindTexture >( dirLightMatWShadow_, dirLightMatWShadow_->GetShaderParameter( "ShadowMap" ), shadowTarget_ );
// 
// 		sunShadowVisitor_.Initialise( renderer, manager );
// 		shadowVisitor_.Initialise( renderer, manager );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hDeferredRenderVisitor::Destroy()
	{
		// call release to avoid destructor calls,
		// we never really "own" a copy of a mesh, just "borrow" it
		// so don't call the destructor
		hGeneralHeap.release( meshes_ );
		meshes_ = NULL;

		sunShadowVisitor_.Destroy();
		shadowVisitor_.Destroy();

		if ( dirLightMat_.HasData() )
		{
			dirLightMat_.Release();
		}
		if ( dirLightMatWShadow_.HasData() )
		{
			dirLightMatWShadow_.Release();
		}
		if ( pointLight1stMat_.HasData() )
		{
			pointLight1stMat_.Release();
		}
		if ( pointLight2ndMat_.HasData() )
		{
			pointLight2ndMat_.Release();
		}
		if ( pointLightInsideMat_.HasData() )
		{
			pointLightInsideMat_.Release();
		}
		if ( spotLight1stMat_.HasData() )
		{
			spotLight1stMat_.Release();
		}
		if ( spotLight2ndMat_.HasData() )
		{
			spotLight2ndMat_.Release();
		}
		if ( spotLightShadow2ndMat_.HasData() )
		{
			spotLightShadow2ndMat_.Release();
		}
		for ( hUint32 i = 0; i < 4; ++i )
		{
			if ( renderTargets_[i].HasData() )
			{
				renderTargets_[i].Release();
			}
		}
		if ( shadowTarget_.HasData() )
		{
			shadowTarget_.Release();
		}
		if ( depthTarget_.HasData() )
		{
			depthTarget_.Release();
		}
		if ( shadowDepthTarget_.HasData() )
		{
			shadowDepthTarget_.Release();
		}

		if ( dirLightGeom_.idxBuf_.HasData() )
		{
			dirLightGeom_.idxBuf_.Release();
		}
		if ( dirLightGeom_.vtxBuf_.HasData() )
		{
			dirLightGeom_.vtxBuf_.Release();
		}

		if ( pointLightGeom_.idxBuf_.HasData() )
		{
			pointLightGeom_.idxBuf_.Release();
		}
		if ( pointLightGeom_.vtxBuf_.HasData() )
		{
			pointLightGeom_.vtxBuf_.Release();
		}

		if ( spotLightGeom_.idxBuf_.HasData() )
		{
			spotLightGeom_.idxBuf_.Release();
		}
		if ( spotLightGeom_.vtxBuf_.HasData() )
		{
			spotLightGeom_.vtxBuf_.Release();
		}

		dirShadowMatrixParm_ = NULL;
		inverseCamProjParam_ = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hDeferredRenderVisitor::BuildScissorForPointLight( LightNodeData* light, ScissorRect* out )
	{
		hMatrix wvp;
		hCPUVec3 sp;
		hCPUVec3 lc = hMatrixFunc::getTranslation( light->matrix_ );
		hCPUVec3 vpd( (hFloat)camera_->GetViewport().width_, (hFloat)camera_->GetViewport().height_, 0.0f );
		//Matrix::mult( &light->matrix_, camera_->GetViewProjectionMatrix(), &wvp );
		wvp = *camera_->GetViewProjectionMatrix();
		hFloat xr = light->maxRadius_;
		hFloat yr = light->maxRadius_;
		hFloat zr = light->maxRadius_;
		hVec3 p[8] = 
		{
			hVec3( lc.x+xr, lc.y+yr, lc.z+zr ),
			hVec3( lc.x-xr, lc.y+yr, lc.z+zr ),
			hVec3( lc.x+xr, lc.y-yr, lc.z+zr ),
			hVec3( lc.x-xr, lc.y-yr, lc.z+zr ),
			hVec3( lc.x+xr, lc.y+yr, lc.z-zr ),
			hVec3( lc.x-xr, lc.y+yr, lc.z-zr ),
			hVec3( lc.x+xr, lc.y-yr, lc.z-zr ),
			hVec3( lc.x-xr, lc.y-yr, lc.z-zr ),
		};

		hUint32 maxX = 0;
		hUint32 maxY = 0;
		hUint32 minX = ~0U;
		hUint32 minY = ~0U;

		for ( hUint32 i = 0; i < 8; ++i )
		{
			sp = (camera_->ProjectTo2D( p[i] ));// + vpd;
			sp.x = hMax( 0.0f, sp.x );
			sp.y = hMax( 0.0f, sp.y );
			sp.x = hMin( vpd.x, sp.x );
			sp.y = hMin( vpd.y, sp.y );
			maxX = hMax( maxX, (hUint32)sp.x );
			maxY = hMax( maxY, (hUint32)sp.y );
			minX = hMin( minX, (hUint32)sp.x );
			minY = hMin( minY, (hUint32)sp.y );
		}

		out->left_		= minX;
		out->top_		= minY;
		out->right_		= maxX;
		out->bottom_	= maxY;
	}

}
