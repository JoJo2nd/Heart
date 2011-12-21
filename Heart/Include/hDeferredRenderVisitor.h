/********************************************************************

	filename: 	DeferredRenderVisitor.h	
	
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

#ifndef DEFERREDRENDERVISITOR_H__
#define DEFERREDRENDERVISITOR_H__

#include "hSceneGraphVisitorBase.h"
#include "hShadowMapVisitor.h"
#include "hResource.h"
#include "hSceneNodeBase.h"
#include "hMatrix.h"
#include "hMesh.h"

namespace Heart
{
	class hRenderer;
	class hRenderTargetTexture;
	class hMesh;
	class hMaterial;

	class hDeferredRenderVisitor : public hSceneGraphVisitorEngine
	{
	public:

		typedef hResourceHandle< hRenderTargetTexture > RenderTargetRes;
		typedef hResourceHandle< hMaterial >			MaterialRes;
		typedef hResourceHandle< hIndexBuffer >			IndexBufferRes;
		typedef hResourceHandle< hVertexBuffer >		VertexBufferRes;
		typedef hResourceHandle< hMesh >				MeshRes;

		hDeferredRenderVisitor() 
			: meshLimit_( DEFAULT_RENDER_INSTANCES )
			, meshes_( NULL )
		{
		}
		virtual ~hDeferredRenderVisitor();

		ENGINE_DEFINE_VISITOR();

		void Initialise( hRenderer* renderer, hResourceManager* manager );
		void Destroy();
		void SetCamera( hSceneNodeCamera* camera ) { camera_ = camera; }
		void PreVisit( hSceneGraph* pSceneGraph );
		void PostVisit( hSceneGraph*	pSceneGraph );
		void Visit( hSceneNodeMesh& visit );
		void Visit( hSceneNodeLocator& visit );
		void Visit( hSceneNodeCamera& visit );
		void Visit( hSceneNodeLight& visit );
		void RenderGBuffers( hRenderer* renderer, hSceneGraph* pSceneGraph );
		void RenderLightPasses( hRenderer* renderer, hSceneGraph* pSceneGraph );

	protected:

		struct LightNodeData
		{
			hMatrix			matrix_;//in world space
			hColour			diffuse_;
			hFloat			minRadius_;
			hFloat			maxRadius_;
			hFloat			power_;
			hFloat			exp_;
			hFloat			spotFalloff_;
			hFloat			spotAngleRad_;
			hBool			shadowCaster_;
			hBool			inside_;
			LightType		type_;
		};

		struct RenderInstance
		{
			hMatrix			matrix_;//in world space
			//Mesh classes are, currently, quite lite so just copy across
			hMesh			meshData_;//< to send to the renderer
			hMaterial*		material_;//< to sort with?, better to take a key?
		};

		struct Geom
		{
			IndexBufferRes			idxBuf_;
			VertexBufferRes			vtxBuf_;
			hUint32					nPrims_;
		};

		void						BuildScissorForPointLight( LightNodeData* light, ScissorRect* out );

		static const hUint32		MAX_LIGHTS = 128;
		static const hUint32		DEFAULT_RENDER_INSTANCES = 2048;

		hSceneNodeCamera*			camera_;
		hUint32						nLights_;
		LightNodeData				lights_[MAX_LIGHTS];
		hUint32						nMeshes_;
		hUint32						meshLimit_;
		RenderInstance*				meshes_;
		hSunShadowMapVisitor		sunShadowVisitor_;
		hShadowMapVisitor			shadowVisitor_;
		MaterialRes					pointLight1stMat_;
		MaterialRes					pointLight2ndMat_;
		MaterialRes					spotLight1stMat_;
		MaterialRes					spotLight2ndMat_;
		MaterialRes					spotLightShadow2ndMat_;
		MaterialRes					pointLightInsideMat_;
		MaterialRes					dirLightMat_;
		MaterialRes					dirLightMatWShadow_;
		const hShaderParameter*		dirShadowMatrixParm_;
		const hShaderParameter*		spotShadowMatrixParm_;
		const hShaderParameter*		inverseCamProjParam_;
		const hShaderParameter*		inverseCamProjPointParam_;
		Geom						pointLightGeom_;
		Geom						dirLightGeom_;
		Geom						spotLightGeom_;
		RenderTargetRes				renderTargets_[4];
		RenderTargetRes				shadowTarget_;
		RenderTargetRes				shadowDepthTarget_;
		RenderTargetRes				depthTarget_;

	};
}

#endif // DEFERREDRENDERVISITOR_H__
