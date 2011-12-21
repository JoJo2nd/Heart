/********************************************************************

	filename: 	ActionImportScene.h	
	
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

#ifndef ACTIONIMPORTSCENE_H__
#define ACTIONIMPORTSCENE_H__

#include "ActionStack.h"

class wxTreeCtrl;

namespace HScene
{
	class SceneGraph;
	class Node;
}

namespace Action
{
	class ImportScene : public ICommand
	{
	public:
		ImportScene( const std::string& filename, u32 aiflags, HScene::SceneGraph* pToScene, HScene::Node* pAsChild, wxTreeCtrl* pSceneView ) :
			filename_( filename )
			,aiFlags_( aiflags )
			,pScene_( pToScene )
			,pSceneView_( pSceneView )
			,pAddParent_( pAsChild )
		{

		}

		void		Execute();
		void		Undo();

	private:

		std::string			filename_;
		u32					aiFlags_;
		wxTreeCtrl*			pSceneView_;
		HScene::SceneGraph*	pScene_;
		HScene::Node*		pAddParent_;
	};
}

#endif // ACTIONIMPORTSCENE_H__