/********************************************************************

	filename: 	MeshBuilderApp.h	
	
	Copyright (c) 18:1:2011 James Moran
	
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

#ifndef MESHBUILDERAPP_H__
#define MESHBUILDERAPP_H__

#include "wx/wx.h"
#include "wx/treectrl.h"
#include "wx/treebase.h"
#include "wx/listctrl.h"
#include "RenderingViewport.h"

class wxTreeCtrl;
class wxPropertyGrid;
class wxPropertyGridEvent;
class wxNotebook;

namespace UI
{

	class IPropertyGridChangeHandler;
	class MaterialListView;
	class TextureListView;
	class MeshListView;
	class SceneGraphTreeViewCtrl;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	class RenderTimer : public wxTimer
	{
	public:
		RenderTimer( Rendering::Viewport* pVeiwport ) :
			pViewport_( pVeiwport )
		{

		}
		void Notify();
		void BeginRenderingTimer()
		{
			wxTimer::Start( 16 );
		}
	private:

		Rendering::Viewport* pViewport_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class MainFrame : public wxFrame
	{
		DECLARE_EVENT_TABLE();
	public:
		MainFrame( const wxChar* title, int xpos, int ypos, int width, int height );
		virtual ~MainFrame();

	protected:

		void OnNew( wxCommandEvent& evt );
		void OnOpen( wxCommandEvent& evt );
		void OnSave( wxCommandEvent& evt );
		void OnImportScene( wxCommandEvent& evt );
		void OnExportScene( wxCommandEvent& evt );
		void OnShowLog( wxCommandEvent& evt );
		void OnAbout( wxCommandEvent& evt );
		void OnExit( wxCommandEvent& evt );
		void ExpandAll( wxCommandEvent& evt );
		void HideAll( wxCommandEvent& evt );
		void DeleteSceneNode( wxCommandEvent& evt );
		void AddTexture( wxCommandEvent& evt );

		//Tree View Events
		void TreeViewBeginDrag( wxTreeEvent& evt );
		void TreeViewEndDrag( wxTreeEvent& evt );
		void TreeViewDelete( wxTreeEvent& evt );
		void TreeViewContextMenu( wxTreeEvent& evt );
		void TreeViewNameEdit( wxTreeEvent& evt );
		void TreeViewSelChanged( wxTreeEvent& evt );
		void TreeViewActivate( wxTreeEvent& evt );

		//Material List Events
		void OnMaterialListItemFocused( wxListEvent& evt );
		void OnTextureListItemFocused( wxListEvent& evt );
		void OnMeshListItemFocused( wxListEvent& evt );

		//PropertyGrid
		void OnPropertyChanging( wxPropertyGridEvent& evt );
		void OnPropertyChanged( wxPropertyGridEvent& evt );

		//Drag & Drop
		void OnFileDrop( wxDropFilesEvent& evt );

		void OnClose( wxCloseEvent& evt );

		void OnTest( wxCommandEvent& evt );

		void UpdateUIViews();

	private:

		HScene::SceneGraph*				pMainScene_;
		wxBoxSizer*						pTopSizer_;
		UI::SceneGraphTreeViewCtrl*		pSceneTreeView_;
		wxMenuBar*						pMenuBar_;
		wxMenu*							pFileMenu_;
		wxMenu*							pViewMenu_;
		wxMenu*							pHelpMen_;
		wxMenu*							pSTVContextMenu_;
		wxMenu*							pSTVNonContextMenu_;
		MaterialListView*				pMaterialList_;
		TextureListView*				pTextureList_;
		MeshListView*					pMeshList_;
		wxPropertyGrid*					pPropertyGrid_;
		RenderTimer*					pRenderTimer_;
		wxNotebook*						pLeftTabPane_;
		Rendering::Viewport				renderingViewport_;
		wxTreeItemId					dragBeginItem_;
		IPropertyGridChangeHandler*		pCurrentPropHandler_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class MainApp : public wxApp
	{
	public:
		virtual bool OnInit();
	
	private:
		
		MainFrame*	pFrame_;
	};
}

#endif // MESHBUILDERAPP_H__