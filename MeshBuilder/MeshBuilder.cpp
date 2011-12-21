// MeshBuilder.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MeshBuilder.h"
#include "RenderingViewport.h"
#include "assimp.h"
#include "aiScene.h"
#include "aiPostProcess.h"
#include "ImportSceneDialog.h"
#include "SceneGraph.h"
#include "SceneNode.h"
#include "LogWindow.h"
#include "ActionStack.h"
#include "ActionImportScene.h"
#include "wx/wx.h"
#include "wx/treectrl.h"

#define MAX_LOADSTRING 100

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

HWND hWnd;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Main Dialog members - add to object somewhere?
Rendering::Viewport gSceneView;
HScene::SceneGraph* gLoadedScene = NULL;
Action::CommandStack gCommandStack;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BOOL				InitInstance(HINSTANCE, int);
INT_PTR CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	AboutWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ImportSceneWndProc(HWND, UINT, WPARAM, LPARAM);

//
void				UpdateTreeControlView( HWND, HScene::SceneGraph* );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifdef USE_WIN32_API
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MESHBUILDER, szWindowClass, MAX_LOADSTRING);

	InitCommonControls();

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	UI::Log::WriteInfo( "Created Main Window" );

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MESHBUILDER));

	//Create our HScene Viewer
	if ( !gSceneView.Initialise( GetDlgItem( hWnd, IDC_GRAPHICSVIEW ) ) )
	{
		UI::Log::WriteError( "Couldn't Create Main Viewport" );
	}
	else
	{
		UI::Log::WriteInfo( "Created Main Viewport" );
	}

	gLoadedScene = new HScene::SceneGraph();

	UpdateTreeControlView( GetDlgItem( hWnd, IDC_SCENEVIEW ), gLoadedScene );

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		gCommandStack.Tick();

		Rendering::RenderViewport( &gSceneView );

		// Don't eat up all CPU time
		Sleep( 10 );
	}

	delete gLoadedScene;
	gLoadedScene = NULL;

	gSceneView.Finalise();
	
	Direct3D::FinaliseD3D();
	
	return (int) msg.wParam;
}
#endif // USE_WIN32_API


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_MESHBUILDER), NULL, &WndProc);	

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   UI::Log::Initilaise( hInst );

   return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
// 	int wmId, wmEvent;
// 	PAINTSTRUCT ps;
// 	HDC hdc;
// 
// 	switch (message)
// 	{
// 	case WM_COMMAND:
// 		wmId    = LOWORD(wParam);
// 		wmEvent = HIWORD(wParam);
// 		// Parse the menu selections:
// 		switch (wmId)
// 		{
// 		case IDM_ABOUT:
// 			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutWndProc);
// 			break;
// 		case ID_FILE_IMPORTSCENE:
// 			{
// 				gCommandStack.PushCommand( new Action::ImportScene( hInst, hWnd, gLoadedScene ) );
// 			}
// 			break;
// 		case ID_VIEW_SHOWLOG:
// 			{
// 				HMENU hMenu = GetMenu( hWnd );
// 				if ( !UI::Log::IsLogVisiable() )
// 				{
// 					UI::Log::WriteLog( "Showing Log View" );
// 					CheckMenuItem( hMenu, ID_VIEW_SHOWLOG, MF_CHECKED );
// 					UI::Log::ShowLogWindow();
// 				}
// 				else
// 				{
// 					UI::Log::WriteLog( "Hiding Log View" );
// 					CheckMenuItem( hMenu, ID_VIEW_SHOWLOG, MF_UNCHECKED );
// 					UI::Log::HideLogWindow();
// 				}
// 			}
// 			break;
// 		case ID_FILE_EXIT:
// 			PostQuitMessage(0);
// 			DestroyWindow(hWnd);
// 			break;
// 		default:
// 			return FALSE;
// 		}
// 		break;
// 	case WM_NOTIFY:
// 		{
// 			LPNMHDR pNmdr = (LPNMHDR)lParam;
// 
// 			switch ( pNmdr->idFrom )
// 			{
// 			case IDC_SCENEVIEW:
// 				{
// 					switch( pNmdr->code )
// 					{
// 					case NM_RCLICK:
// 						UI::Log::WriteInfo( "NM_RCLICK message got" );
// 						return TRUE;
// 					}
// 				}
// 				break;
// 			}
// 		}
// 		break;
// 	case WM_SIZE:
// 		RECT rcClient;
// 		GetClientRect(hWnd, &rcClient);
// 		//TODO: Resize components
// 		break;
// 	case WM_PAINT:
// 		hdc = BeginPaint(hWnd, &ps);
// 		// TODO: Add any drawing code here...
// 		EndPaint(hWnd, &ps);
// 		break;
// 	case WM_CLOSE:
// 		PostQuitMessage(0);
// 		DestroyWindow(hWnd);
// 		break;
// 	default:
// 		return FALSE;
// 	}
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK AboutWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void RecursiveUpdateTree( wxTreeCtrl* ptree, HScene::Node* pnode, wxTreeItemId* itemparent )
{
	wxTreeItemId* newitem = NULL;
	if ( itemparent == NULL && pnode->TreeItemID() == NULL )
	{		
		newitem = new wxTreeItemId;
		HScene::NodeTreeHook* pitemhook = new HScene::NodeTreeHook;
		pitemhook->pHook_ = pnode;
		*newitem = ptree->AddRoot( pnode->pName(), -1, -1, pitemhook );
		pnode->TreeItemID( newitem );
	}
	else if ( pnode->TreeItemID() == NULL )
	{
		newitem = new wxTreeItemId;
		HScene::NodeTreeHook* pitemhook = new HScene::NodeTreeHook;
		pitemhook->pHook_ = pnode;
		*newitem = ptree->AppendItem( *itemparent, pnode->pName(), -1, -1, pitemhook );
		pnode->TreeItemID( newitem );
	}
	else
	{
		ptree->SetItemText( *pnode->TreeItemID(), pnode->pName() );
	}

	for ( HScene::Node::ChildListType::const_iterator 
		  i = pnode->pChildren()->begin(), iend = pnode->pChildren()->end(); i != iend; ++i )
	{
		RecursiveUpdateTree( ptree, *i, pnode->TreeItemID() );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void UpdateTreeControlView( wxTreeCtrl* ptree, HScene::SceneGraph* pScene )
{
	if ( pScene->pRoot() )
	{
		RecursiveUpdateTree( ptree, pScene->pRoot(), pScene->pRoot()->TreeItemID() );
	}
}
