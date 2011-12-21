/********************************************************************

	filename: 	ActionImportScene.cpp	
	
	Copyright (c) 17:1:2011 James Moran
	
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
#include "Resource.h"
#include "ActionImportScene.h"
#include "LogWindow.h"
#include "SceneGraph.h"
#include "ImportSceneDialog.h"
#include "assimp.h"
#include "aiPostProcess.h"
#include "AssImpWrapper.h"
#include "wx/wx.h"
#include "wx/treectrl.h"
#include "SceneDatabase.h"

extern void				UpdateTreeControlView( wxTreeCtrl*, HScene::SceneGraph* );

namespace Action
{


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ImportScene::Execute()
	{
		UI::Log::WriteLog( "Attempting to load file %s ...", filename_.c_str() );
		u32 flags = 
			aiFlags_								|
//			aiProcess_CalcTangentSpace				| 
			aiProcess_GenSmoothNormals				| 
			aiProcess_JoinIdenticalVertices			| 
			aiProcess_ImproveCacheLocality			| 
			aiProcess_LimitBoneWeights				| 
			aiProcess_RemoveRedundantMaterials      | 
			aiProcess_SplitLargeMeshes				| 
			aiProcess_Triangulate					| 
			aiProcess_GenUVCoords                   | 
			aiProcess_SortByPType                   | 
			aiProcess_FindDegenerates               | 
			aiProcess_FindInvalidData               | 
			aiProcess_MakeLeftHanded			;
		if ( aiFlags_ & aiProcess_FindInstances )
		{
			UI::Log::WriteInfo( "aiProcess_FindInstances is Set" );
		}
		if ( aiFlags_ & aiProcess_JoinIdenticalVertices )
		{
			flags |= aiProcess_JoinIdenticalVertices;
			UI::Log::WriteInfo( "aiProcess_JoinIdenticalVertices is Set" );
		}
		if ( aiFlags_ & aiProcess_OptimizeMeshes )
		{
			UI::Log::WriteInfo( "aiProcess_OptimizeMeshes is Set" );
		}
		if ( aiFlags_ & aiProcess_OptimizeGraph )
		{
			UI::Log::WriteInfo( "aiProcess_OptimizeGraph is Set" );
		}

		const aiScene* paiScene = aiImportFile( filename_.c_str(), flags );
		if ( paiScene && (paiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != AI_SCENE_FLAGS_INCOMPLETE )
		{
			UI::Log::WriteLog( "Loaded %s", filename_.c_str() );
	
			HScene::ImportToScene( pScene_, paiScene, pAddParent_ );

			UpdateTreeControlView( pSceneView_, pScene_ );

			pSceneView_->ExpandAll();

		}
		else
		{
			UI::Log::WriteError( "Could Not Load %s", filename_.c_str() );
		}

		aiReleaseImport( paiScene );

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ImportScene::Undo()
	{
		UI::Log::WriteWarning( "Undo for Import scene not yet implemented" );
	}

}