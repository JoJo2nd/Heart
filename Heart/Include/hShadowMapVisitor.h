/********************************************************************
	created:	2010/12/15
	created:	15:12:2010   17:29
	filename: 	GameShadowRenderVisitor.h	
	author:		James
	
	purpose:	
*********************************************************************/

#ifndef SHADOWMAPVISITOR_H__
#define SHADOWMAPVISITOR_H__

#include "hSceneGraphVisitorBase.h"
#include "hMaterial.h"
#include "hVector.h"
#include "hMatrix.h"
#include "hAABB.h"
#include "hViewFrustum.h"

namespace Heart
{
	class hRenderer;
	class hRenderTargetTexture;
	class hTextureBase;
	class DepthSurface;
	class hMesh;
	class hRenderTargetTexture;
	class hRendererCamera;
	class hSceneGraph;
	class hSceneNodeBase;
	class hSceneNodeMesh;
	class hSceneNodeLocator;
	class hSceneNodeCamera;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hShadowMapVisitor : public hSceneGraphVisitorEngine
	{
	public:

		typedef Heart::hResourceHandle< hMaterial > MaterialRes;
		typedef Heart::hResourceHandle< hRenderTargetTexture > RenderTargetRes;

		hShadowMapVisitor();
		~hShadowMapVisitor();

		virtual void 							PreVisit( Heart::hSceneGraph* pSceneGraph );
		virtual void 							PostVisit( Heart::hSceneGraph* pSceneGraph );
		void		 							Initialise( Heart::hRenderer* prenderer, hResourceManager* manager );
		void									Destroy();
		void		 							SetCameraNode( Heart::hSceneNodeCamera* pcamera ) { cameraNode_ = pcamera; }
		void									SetLightMatrix( hMatrix* m ) { lightMatrix_ = *m; }
		void									SetLightParameters( hFloat fov, hFloat falloff ) { lightFOV_ = fov; lightFalloff_ = falloff; }
		const hMatrix*						GetShadowMatrix() const { return &shadowMatrix_; }
		void									SetShadowTarget( hRenderTargetTexture* val ) { shadowTarget_ = val; }
		hRenderTargetTexture*			GetShadowTarget() { return shadowTarget_; }
		void									SetDepthTarget( hRenderTargetTexture* val ) { depthTarget_ = val; }
		hRenderTargetTexture*			GetDepthTarget() { return depthTarget_; }
		virtual void 							Visit( hSceneNodeMesh& visit );
		virtual void 							Visit( hSceneNodeLocator& visit );
		virtual void 							Visit( hSceneNodeCamera& visit );

	public:

		MaterialRes								shadowMaterial_;
		const Heart::hShaderParameter*	baisParam_;
		Heart::hSceneGraph*				sceneGraph_;
		Heart::hRenderer*				renderer_;
		Heart::hSceneNodeCamera*			cameraNode_;
		hMatrix							lightMatrix_;
		hFloat									lightFOV_;
		hFloat									lightFalloff_;
		hViewFrustum								lightFrustum_;
		hRenderTargetTexture*			shadowTarget_;
		hRenderTargetTexture*			depthTarget_;

		//Use a linked list instead?
		struct ShadowCaster
		{
			hMatrix	matrix_;//in world space
			hAABB		aabb_;//< needed for AABB size
			hMesh*	meshData_;//< to send to the renderer
			bool			castShadows_;
		};

		static const hUint32		MAX_SHADOW_CASTERS = 2048;
		hVec3						lightDir_;
		hVec3						cameraEye_;
		hMatrix						shadowMatrix_;
		hUint32						nShadowCasters_;
		ShadowCaster				shadowCasters_[ MAX_SHADOW_CASTERS ];
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSunShadowMapVisitor : public hShadowMapVisitor
	{
	public:
		virtual void 			PreVisit( hSceneGraph* pSceneGraph );
		virtual void 			PostVisit( hSceneGraph* pSceneGraph );
		virtual void 			Visit( hSceneNodeMesh& visit );
		virtual void 			Visit( hSceneNodeLocator& visit );
		virtual void 			Visit( hSceneNodeCamera& visit );
	private:
	};

}

#endif // GAMESHADOWRENDERVISITOR_H__