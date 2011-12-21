/********************************************************************

	filename: 	MeshListView.h	
	
	Copyright (c) 10:5:2011 James Moran
	
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
#ifndef MESHLISTVIEW_H__
#define MESHLISTVIEW_H__

#include "wx\wx.h"
#include "wx\listctrl.h"
#include "SceneDatabase.h"
#include "IPropertyGridChangeHandler.h"

class wxPropertyGrid;

namespace HScene
{
	class SceneGraph;
}

namespace UI
{
	class MeshListView : public wxListCtrl,
						 public IPropertyGridChangeHandler
	{
	public:
		MeshListView(
			HScene::SceneGraph* pScene,
			wxWindow* parent, 
			wxWindowID id, 
			wxPoint position = wxDefaultPosition, 
			wxSize size = wxDefaultSize
			);
		virtual ~MeshListView();

		void UpdatePropertyGrid( wxListEvent& evt, wxPropertyGrid* pPropertyGrid );
		void UpdateMeshView();

		void OnPropertyGridCellChanging( wxPropertyGridEvent& evt );
		bool OnPropertyGridCellChanged( wxPropertyGridEvent& evt );

	private:

		static const char* MATERIAL_NAME;

		HScene::SceneGraph* pScene_;

	};

}

#endif // MESHLISTVIEW_H__