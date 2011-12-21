/********************************************************************

	filename: 	ActionExportScene.h	
	
	Copyright (c) 6:3:2011 James Moran
	
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

#ifndef ACTIONEXPORTSCENE_H__
#define ACTIONEXPORTSCENE_H__

#include "ActionStack.h"
#include "SceneGraph.h"
#include <string>
#include "wx/wx.h"
#include "tinyxml.h"

namespace Data
{
	class Mesh;
	class Material;
}

namespace Action
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class ExportScene : public ICommand
	{ 
	public:
		ExportScene( HScene::SceneGraph* pSceneToExport, wxString* destPath, wxString* name );
		~ExportScene();

		void		Execute();

		void ExportMeshList( std::list< Data::Mesh* >& usedMeshes, const wxString& meshDir, const wxString& materialDir, std::list< Data::Material* >& usedMaterials );
		void		Undo();

	private:

		void					VisitScene( HScene::SceneGraph* pScene, TiXmlElement* pRoot, std::list< Data::Mesh* >* pMeshList );

		HScene::SceneGraph*		pScene_;
		wxString				destPath_;
		wxString				destName_;
	};
}

#endif // ACTIONEXPORTSCENE_H__