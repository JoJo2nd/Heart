/********************************************************************

	filename: 	hShadowMapVisitor.h	
	
	Copyright (c) 1:4:2012 James Moran
	
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

#ifndef SHADOWMAPVISITOR_H__
#define SHADOWMAPVISITOR_H__

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

#if 0
		typedef Heart::hResourceHandle< hMaterial > MaterialRes;
		typedef Heart::hResourceHandle< hRenderTargetTexture > RenderTargetRes;
#endif

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

#if 0
		MaterialRes								shadowMaterial_;
#endif
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