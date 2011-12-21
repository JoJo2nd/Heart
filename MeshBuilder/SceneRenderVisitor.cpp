/********************************************************************

	filename: 	SceneRenderVisitor.cpp	
	
	Copyright (c) 23:1:2011 James Moran
	
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

#include "stdafx.h"
#include "SceneRenderVisitor.h"
#include "SceneNodeMeshCollection.h"
#include "SceneNodeCamera.h"
#include "SceneNodeLight.h"
#include "SceneNodeLocator.h"
#include "DataMesh.h"
#include "DataMaterial.h"

namespace HScene
{


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RenderVisitor::Visit( Locator* pVisit )
	{
		pD3D_->DrawLocatorPrimative( &pVisit->globalMatrix_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RenderVisitor::Visit( MeshCollection* pVisit )
	{
		LPDIRECT3DDEVICE9 pD3DD = pD3D_->pD3DDevice_;

		for ( u32 i = 0; i < pVisit->nMeshes(); ++i )
		{
			//Get the mesh data from the scene attached to the node.
			Data::Mesh* pmesh = pVisit->pMesh( i );
			Data::Material* pmat = pVisit->pMeshMaterial( i );
			pD3D_->DrawIndexedPrimative( 
				pmat, 
				&pVisit->globalMatrix_, 
				pmesh->VertexFlags(), 
				pmesh->DrawIndexCount(), 
				pmesh->DrawVertexCount(), 
				pmesh->VertexStride(), 
				pmesh->DrawIndex(), 
				pmesh->DrawVertex() );

		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RenderVisitor::Visit( Camera* pVisit )
	{
		pD3D_->DrawCameraPrimative( &pVisit->globalMatrix_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RenderVisitor::Visit( Light* pVisit )
	{
		pD3D_->DrawLightPrimative( &pVisit->globalMatrix_ );
		if ( pVisit->lightType_ == LightType_Point )
		{
			pD3D_->DrawSpherePrimative( &pVisit->globalMatrix_, pVisit->minRadius_ );
			pD3D_->DrawSpherePrimative( &pVisit->globalMatrix_, pVisit->maxRadius_ );
		}
		else if ( pVisit->lightType_ == LightType_Spot )
		{
			pD3D_->DrawConePrimative( &pVisit->globalMatrix_, pVisit->falloff_, pVisit->innerCone_ );
		}
	}

}