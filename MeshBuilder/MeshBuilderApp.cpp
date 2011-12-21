 /********************************************************************
 
 	filename: 	MeshBuilderApp.cpp	
 	
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

#include "stdafx.h"
#include "MeshBuilderApp.h"
#include "UIDefs.h"
#include "wx/treebase.h"
#include "wx/treectrl.h"
#include "wx/propgrid/propgrid.h"
#include "wx/notebook.h"
#include "wx/listctrl.h"
#include "ImportSceneDialog.h"
#include "SceneGraph.h"
#include "ActionStack.h"
#include "aiPostProcess.h"
#include "ActionImportScene.h"
#include "Resource.h"
#include "MaterialListView.h"
#include "TextureListView.h"
#include "SceneGraphTreeViewCtrl.h"
#include "Panel3DMovement.h"
#include "ActionExportScene.h"
#include "MeshListView.h"

extern Action::CommandStack gCommandStack;

extern void UpdateTreeControlView( wxTreeCtrl* , HScene::SceneGraph* );

namespace UI
{

	IMPLEMENT_APP( MainApp );

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool MainApp::OnInit()
	{
		TCHAR szTitle[MAX_PATH];					// The title bar text
		TCHAR szWindowClass[MAX_PATH];			// the main window class name
		LoadString(NULL, IDS_APP_TITLE, szTitle, MAX_PATH);
		LoadString(NULL, IDC_MESHBUILDER, szWindowClass, MAX_PATH);
		GetCurrentDirectory( MAX_PATH, szWindowClass );

		wxInitAllImageHandlers();

		pFrame_ = new MainFrame( _T("Heart Engine Scene Builder"), -1, -1, -1, -1 );

		pFrame_->Show();
		SetTopWindow( pFrame_ );
		
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	BEGIN_EVENT_TABLE(MainFrame,wxFrame)
		EVT_MENU( wxID_NEW, MainFrame::OnNew )
		EVT_MENU( wxID_OPEN, MainFrame::OnOpen )
		EVT_MENU( wxID_SAVE, MainFrame::OnSave )
		EVT_MENU( ID_IMPORTSCENE, MainFrame::OnImportScene )
		EVT_MENU( ID_EXPORTSCENE, MainFrame::OnExportScene )
		EVT_MENU( ID_SHOWLOG, MainFrame::OnShowLog )
		EVT_MENU( wxID_ABOUT, MainFrame::OnAbout )
		EVT_MENU( wxID_EXIT, MainFrame::OnExit )
		EVT_MENU( ID_TEST, MainFrame::OnTest )
		EVT_MENU( ID_EXPANDSCENE, MainFrame::ExpandAll )
		EVT_MENU( ID_HIDESCENE, MainFrame::HideAll )
		EVT_MENU( ID_DELETESCENENODE, MainFrame::DeleteSceneNode )
		EVT_MENU( ID_ADDTEXTURE, MainFrame::AddTexture )
		EVT_DROP_FILES( MainFrame::OnFileDrop )
		EVT_TREE_ITEM_ACTIVATED( ID_SCENEVIEWTREE, MainFrame::TreeViewActivate )
		EVT_TREE_BEGIN_DRAG( ID_SCENEVIEWTREE, MainFrame::TreeViewBeginDrag )
		EVT_TREE_END_DRAG( ID_SCENEVIEWTREE, MainFrame::TreeViewEndDrag )
		EVT_TREE_DELETE_ITEM( ID_SCENEVIEWTREE, MainFrame::TreeViewDelete )
		EVT_TREE_ITEM_MENU( ID_SCENEVIEWTREE, MainFrame::TreeViewContextMenu )
		EVT_TREE_SEL_CHANGED( ID_SCENEVIEWTREE, MainFrame::TreeViewSelChanged )
		EVT_TREE_END_LABEL_EDIT( ID_SCENEVIEWTREE, MainFrame::TreeViewNameEdit )
		EVT_LIST_ITEM_SELECTED( ID_MATERIALLIST, MainFrame::OnMaterialListItemFocused )
		EVT_LIST_ITEM_SELECTED( ID_TEXTURELIST, MainFrame::OnTextureListItemFocused )
		EVT_LIST_ITEM_SELECTED( ID_MESHLIST, MainFrame::OnMeshListItemFocused )
		EVT_PG_CHANGED( ID_PROPS, MainFrame::OnPropertyChanged )
		EVT_PG_CHANGING( ID_PROPS, MainFrame::OnPropertyChanging )
		EVT_CLOSE( MainFrame::OnClose )
	END_EVENT_TABLE()

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	MainFrame::MainFrame( const wxChar* title, int xpos, int ypos, int width, int height ) :
		wxFrame( NULL, -1, title, wxPoint(xpos, ypos), wxSize(width, height) ),
		pCurrentPropHandler_( NULL )
	{
		pMainScene_ = new HScene::SceneGraph();

		pTopSizer_ = new wxBoxSizer( wxHORIZONTAL );
		//TODO: These need to become members
		Panel3DMovement* panel = new Panel3DMovement( this );

		//Allow Drag & Drop
		DragAcceptFiles( true );

		//TODO: Set up the properties grid under the rendering view

		//Property Grid
		pPropertyGrid_ = new wxPropertyGrid( this, ID_PROPS );

		//Create the left pane view
		pLeftTabPane_ = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxSize( 300, 480 ) );

		//TODO: These needs to become a custom class
		pSceneTreeView_ = new UI::SceneGraphTreeViewCtrl( pMainScene_, pLeftTabPane_, ID_SCENEVIEWTREE );

		//TODO: custom classes for these
		pMaterialList_ = new MaterialListView( pMainScene_, pLeftTabPane_, ID_MATERIALLIST, pPropertyGrid_ );
		pTextureList_ = new TextureListView( pMainScene_, pLeftTabPane_, ID_TEXTURELIST );
		pMeshList_ = new MeshListView( pMainScene_, pLeftTabPane_, ID_MESHLIST );

		//Add the Elements to the left tab pane (e.g. scene view, material list, texture list, etc)
		pLeftTabPane_->AddPage( pSceneTreeView_, "Scene Layout" );
		pLeftTabPane_->AddPage( pMaterialList_, "Materials" );
		pLeftTabPane_->AddPage( pTextureList_, "Textures" );
		pLeftTabPane_->AddPage( pMeshList_, "Meshes" );

		pMenuBar_ = new wxMenuBar();
		//File Menu
		pFileMenu_ = new wxMenu();
		pFileMenu_->Append( wxID_NEW, _T( "&New" ) );
		pFileMenu_->Append( wxID_OPEN, _T("&Open" ) );
		pFileMenu_->Append( wxID_SAVE, _T("&Save" ) );
		pFileMenu_->AppendSeparator();
		pFileMenu_->Append( ID_IMPORTSCENE, _T( "&Import" ) );
		pFileMenu_->Append( ID_EXPORTSCENE, _T( "&Export" ) );
		pFileMenu_->AppendSeparator();
		pFileMenu_->Append( wxID_EXIT, _T( "E&xit") );

		pMenuBar_->Append( pFileMenu_, _T( "&File" ) );

		//Edit
		wxMenu* editMenu = new wxMenu();
		editMenu->Append( ID_ADDTEXTURE, _T( "Add &Texture" ) );

		pMenuBar_->Append( editMenu, _T( "&Edit" ) );

		//View Menu
		pViewMenu_ = new wxMenu();
		pViewMenu_->Append( ID_SHOWLOG, _T( "Show &Log" ) );

		pMenuBar_->Append( pViewMenu_, _T( "&View" ) );

		//Help Menu
		pHelpMen_ = new wxMenu();
		pHelpMen_->Append( wxID_ABOUT, _T( "A&bout" ) );

		pMenuBar_->Append( pHelpMen_, _T( "?" ) );

		SetMenuBar( pMenuBar_ );

		pTopSizer_->Add( pLeftTabPane_, 15, wxEXPAND | wxALL, 0 );
		pTopSizer_->Add( panel, 50, wxEXPAND | wxALL, 0 );
		pTopSizer_->Add( pPropertyGrid_, 15, wxEXPAND | wxALL, 0 );

		SetAutoLayout( true );
		SetSizer( pTopSizer_ );
		pTopSizer_->Fit( this );
		pTopSizer_->SetSizeHints( this );

		pRenderTimer_ = new RenderTimer( &renderingViewport_ );
		pRenderTimer_->BeginRenderingTimer();

		renderingViewport_.Initialise( (HWND)panel->GetHandle() );
		renderingViewport_.pScene( pMainScene_ );

		panel->SetDevice( renderingViewport_.pDevice(), &renderingViewport_ );

		pSTVNonContextMenu_ = new wxMenu();
		pSTVNonContextMenu_->Append( ID_EXPANDSCENE, "Expand All" );
		pSTVNonContextMenu_->Append( ID_HIDESCENE, "Hide All" );

		pSTVContextMenu_ = new wxMenu();
		pSTVContextMenu_->Append( ID_DELETESCENENODE, "Delete" );

		// 
		UpdateTreeControlView( pSceneTreeView_, pMainScene_ );
		
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	MainFrame::~MainFrame()
	{
		delete pSTVContextMenu_;
		delete pSTVNonContextMenu_;
	}

	//////////////////////////////////////////////////////////////////////////
	// 14:59:23 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnNew( wxCommandEvent& evt )
	{
		renderingViewport_.Enabled( false );

		pSceneTreeView_->Delete( pSceneTreeView_->GetRootItem() );
		pMainScene_->NewScene();
		UpdateUIViews();

		renderingViewport_.Enabled( true );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnOpen( wxCommandEvent& evt )
	{
		renderingViewport_.Enabled( false );

		std::string openfile;
		wxString filetypes = "Scene files (*.xml)|*.xml";//"Scene|*.xml";
		//filetypes += sz.data;

		wxFileDialog openFileDialog = new wxFileDialog( this );

		openFileDialog.SetMessage( "Open Scene File" );
		openFileDialog.SetWildcard( filetypes );

		if ( openFileDialog.ShowModal() != wxID_OK )
		{
			return;
		}

		openfile += openFileDialog.GetDirectory();
		openfile += "\\";
		openfile += openFileDialog.GetFilename();

		TiXmlDocument doc; 
		doc.SetCondenseWhiteSpace( false );
		if ( !doc.LoadFile( openfile.c_str() ) )
		{
			wxMessageDialog msg( this, _T( "Couldn't open or parse file" ) );
			msg.ShowModal();
			return;
		}

		TiXmlElement* scene = doc.FirstChildElement( "scene" );
		if ( !scene )
		{
			wxMessageDialog msg( this, _T( "Couldn't open or parse file" ) );
			msg.ShowModal();
			return;
		}

		pMainScene_->ClearScene();

		if ( !pMainScene_->Deserialise( scene ) )
		{
			wxMessageDialog msg( this, _T( "Couldn't open or parse file" ) );
			msg.ShowModal();
			return;
		}

		pSceneTreeView_->Delete( pSceneTreeView_->GetRootItem() );

		UpdateUIViews();

		renderingViewport_.Enabled( true );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnSave( wxCommandEvent& evt )
	{
		renderingViewport_.Enabled( false );

		std::string savefile;
		std::string filetypes = "Scene files (*.xml)|*.xml";//"Scene|*.xml";

		savefile = wxFileSelector( "Save Scene File", wxEmptyString, wxEmptyString, ".xml", filetypes, wxFD_SAVE, this );
		if ( savefile.empty() )
		{
			return;
		}
		
		TiXmlDocument doc; 
		doc.SetCondenseWhiteSpace( false );
		TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "yes" ); 
		TiXmlElement* pRootEle = new TiXmlElement( "scene" );
		doc.LinkEndChild( decl );
		doc.LinkEndChild( pRootEle );

		pMainScene_->Serialise( pRootEle );

		doc.SaveFile( savefile.c_str() );

		renderingViewport_.Enabled( true );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnImportScene( wxCommandEvent& evt )
	{
		UI::ImportSceneDialog importDialog( pMainScene_, this, wxID_ANY );

		if ( importDialog.ShowModal() == wxID_OK )
		{
			u32 flags = 0;
			flags |= importDialog.FindInstances() ? aiProcess_FindInstances : 0;
			flags |= importDialog.JoinIdentical() ? aiProcess_JoinIdenticalVertices : 0;
			flags |= importDialog.OptimiseMesh() ? aiProcess_OptimizeMeshes : 0;
			flags |= importDialog.OptimiseGraph() ? aiProcess_OptimizeGraph : 0;
			flags |= importDialog.CalcTangentsBinormals() ? aiProcess_CalcTangentSpace : 0;
			gCommandStack.PushCommand( new Action::ImportScene( importDialog.GetFileName(), flags, pMainScene_, importDialog.pParentNode(), pSceneTreeView_ ) );

			UpdateUIViews();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnExportScene( wxCommandEvent& evt )
	{
		wxTextEntryDialog nameEntry( this, "Enter Scene Export Name" );
		wxDirDialog dirDialog( this, "Choose an export directory", pMainScene_->GetLastExportPath(), wxDD_DEFAULT_STYLE );

		if ( nameEntry.ShowModal() == wxID_OK )
		{
			wxString name = nameEntry.GetValue();

			if ( name.IsEmpty() )
			{
				wxMessageDialog msg( this, "A valid name is needed." );
				msg.ShowModal();
				return;
			}

			if ( dirDialog.ShowModal() == wxID_OK )
			{
				gCommandStack.PushCommand( new Action::ExportScene( pMainScene_, &dirDialog.GetPath(), &name ) );
				pMainScene_->SetLastExportPath( dirDialog.GetPath().c_str() );
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnShowLog( wxCommandEvent& evt )
	{
		wxMessageDialog msg( this, _T( "TODO" ) );
		msg.ShowModal();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnAbout( wxCommandEvent& evt )
	{
		wxMessageDialog msg( this, _T( "TODO" ) );
		msg.ShowModal();
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnExit( wxCommandEvent& evt )
	{
		Close( false );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnClose( wxCloseEvent& evt )
	{
		pRenderTimer_->Stop();
		delete pRenderTimer_;
		pRenderTimer_ = NULL;
		evt.Skip();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::AddTexture( wxCommandEvent& evt )
	{
		std::string texfile;
		std::string filetypes = "PNG files (*.PNG)|*.PNG";//"Scene|*.xml";

		texfile = wxFileSelector( "Open Texture File", wxEmptyString, wxEmptyString, ".png", filetypes, wxFD_OPEN, this );
		if ( texfile.empty() )
		{
			return;
		}
		pMainScene_->ImportTextureToScene( texfile.c_str() );
		UpdateUIViews();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::TreeViewBeginDrag( wxTreeEvent& evt )
	{
		wxTreeItemId id = evt.GetItem();
		if ( id == pSceneTreeView_->GetRootItem() || !id.IsOk() )
		{
			// Don't allow the root to be moved
			evt.Veto();
		}

		dragBeginItem_ = id;
		evt.Allow();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::TreeViewEndDrag( wxTreeEvent& evt )
	{
		wxTreeItemId id = evt.GetItem();
		if ( id.IsOk() )
		{
			wxString beginname = pSceneTreeView_->GetItemText( dragBeginItem_ );
			wxString endname = pSceneTreeView_->GetItemText( id );
			HScene::Node* pbegin = HScene::FindNode( pMainScene_->pRoot(), (const char*)beginname.c_str() );
			HScene::Node* pend = HScene::FindNode( pMainScene_->pRoot(), (const char*)endname.c_str() );

			if ( pbegin && pend && id != dragBeginItem_ )
			{
				//delete the items from the tree view
				pSceneTreeView_->DeleteChildren( dragBeginItem_ );
				pSceneTreeView_->Delete( dragBeginItem_ );

				//Fix the real data
				if ( pbegin->pParent() )
				{
					pbegin->pParent()->RemoveChild( pbegin );
				}
				pend->AttachChild( pbegin );

				//Update the view to show the new layout
				UpdateTreeControlView( pSceneTreeView_, pMainScene_ );
			}
		}
		else
		{
			evt.Veto();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::TreeViewDelete( wxTreeEvent& evt )
	{
		wxString name = pSceneTreeView_->GetItemText( evt.GetItem() );
		
		HScene::Node* pnode = HScene::FindNode( pMainScene_->pRoot(), (const char*)name.c_str() );
		if ( pnode )
		{
			pnode->TreeItemID( NULL );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::TreeViewContextMenu( wxTreeEvent& evt )
	{
		int hitflags;
		wxTreeItemId id = pSceneTreeView_->HitTest( evt.GetPoint(), hitflags );
		if ( hitflags & wxTREE_HITTEST_ONITEM )
		{
			PopupMenu( pSTVContextMenu_ );
		}
		else
		{
			PopupMenu( pSTVNonContextMenu_ );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::TreeViewNameEdit( wxTreeEvent& evt )
	{
		wxMessageDialog msg( this, _T( "TODO" ) );
		msg.ShowModal();
		evt.Veto();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void MainFrame::TreeViewActivate( wxTreeEvent& evt )
	{
		wxString name = pSceneTreeView_->GetItemText( evt.GetItem() );

		HScene::Node* pnode = HScene::FindNode( pMainScene_->pRoot(), (const char*)name.c_str() );
		if ( pnode )
		{
			renderingViewport_.pDevice()->SetViewTranslation( D3DXVECTOR3( pnode->globalMatrix_._41, pnode->globalMatrix_._42, pnode->globalMatrix_._43 ) );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnTest( wxCommandEvent& evt )
	{
		wxMessageDialog msg( this, _T( "TODO: Context Click" ) );
		msg.ShowModal();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::ExpandAll( wxCommandEvent& evt )
	{
		pSceneTreeView_->ExpandAll();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::HideAll( wxCommandEvent& evt )
	{
		pSceneTreeView_->CollapseAll();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::DeleteSceneNode( wxCommandEvent& evt )
	{
		wxMessageDialog msg( this, _T( "TODO: Delete Node" ) );
		msg.ShowModal();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnMaterialListItemFocused( wxListEvent& evt )
	{
		pMaterialList_->UpdatePropertyGrid( evt, pPropertyGrid_ );
		pCurrentPropHandler_ = pMaterialList_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnTextureListItemFocused( wxListEvent& evt )
	{
		pTextureList_->UpdatePropertyGrid( evt, pPropertyGrid_ );
		pCurrentPropHandler_ = pTextureList_;
	}

	//////////////////////////////////////////////////////////////////////////
	// 19:23:26 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnMeshListItemFocused( wxListEvent& evt )
	{
		pMeshList_->UpdatePropertyGrid( evt, pPropertyGrid_ );
		pCurrentPropHandler_ = pMeshList_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::TreeViewSelChanged( wxTreeEvent& evt )
	{
		pSceneTreeView_->UpdatePropertyGrid( evt, pPropertyGrid_ );
		pCurrentPropHandler_ = pSceneTreeView_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnPropertyChanging( wxPropertyGridEvent& evt )
	{
		if ( pCurrentPropHandler_ )
		{
			pCurrentPropHandler_->OnPropertyGridCellChanging( evt );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnPropertyChanged( wxPropertyGridEvent& evt )
	{
		if ( pCurrentPropHandler_ )
		{
			if ( pCurrentPropHandler_->OnPropertyGridCellChanged( evt ) )
			{
				UpdateUIViews();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::OnFileDrop( wxDropFilesEvent& evt )
	{
		wxString* files = evt.GetFiles();
		for ( s32 i = 0; i < evt.GetNumberOfFiles(); ++i )
		{
			if ( files[i].EndsWith( ".dae" ) || files[i].EndsWith( ".DAE" ) )
			{
				UI::ImportSceneDialog importDialog( pMainScene_, this, wxID_ANY, &files[i] );

				if ( importDialog.ShowModal() == wxID_OK )
				{
					u32 flags = 0;
					flags |= importDialog.FindInstances() ? aiProcess_FindInstances : 0;
					flags |= importDialog.JoinIdentical() ? aiProcess_JoinIdenticalVertices : 0;
					flags |= importDialog.OptimiseMesh() ? aiProcess_OptimizeMeshes : 0;
					flags |= importDialog.OptimiseGraph() ? aiProcess_OptimizeGraph : 0;
					flags |= importDialog.CalcTangentsBinormals() ? aiProcess_CalcTangentSpace : 0;
					gCommandStack.PushCommand( new Action::ImportScene( importDialog.GetFileName(), flags, pMainScene_, importDialog.pParentNode(), pSceneTreeView_ ) );

					pMaterialList_->UpdateMaterialView();
					pTextureList_->UpdateTextureView();
					pMeshList_->UpdateMeshView();
				}
			}
			if ( files[i].EndsWith( ".png" ) || files[i].EndsWith( ".PNG" ) )
			{
				pMainScene_->ImportTextureToScene( files[i] );
				UpdateUIViews();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 15:01:24 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MainFrame::UpdateUIViews()
	{
		pPropertyGrid_->Clear();
		UpdateTreeControlView( pSceneTreeView_, pMainScene_ );
		pMaterialList_->UpdateMaterialView();
		pTextureList_->UpdateTextureView();
		pMeshList_->UpdateMeshView();
		pSceneTreeView_->ExpandAll();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RenderTimer::Notify()
	{
		Rendering::RenderViewport( pViewport_ );
	}

}