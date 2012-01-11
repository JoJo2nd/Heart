/********************************************************************

	filename: 	MainWindow.cpp	
	
	Copyright (c) 18:8:2011 James Moran
	
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

#include "PreCompiled.h"
#include "MainWindow.h"
#include "DataLayoutPane.h"
#include "PropertiesPane.h"

IMPLEMENT_APP( PackerApp );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool PackerApp::OnInit()
{
	//TCHAR szTitle[MAX_PATH];					// The title bar text
	WCHAR szWindowClass[MAX_PATH];			// the main window class name
	//LoadString(NULL, IDS_APP_TITLE, szTitle, MAX_PATH);
	//LoadString(NULL, IDC_MESHBUILDER, szWindowClass, MAX_PATH);
	GetCurrentDirectory( MAX_PATH, szWindowClass );

	wxInitAllImageHandlers();

	mainFrame_ = new MainWindow();

	mainFrame_->Show();
	SetTopWindow( mainFrame_ );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Our Custom Events that we send ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
EVT_DEFINE_EVENT( db_loaded );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( MainWindow, wxFrame )
	EVT_MENU( UIID_SHOWDATALAYOUT, MainWindow::OnShowDataLayout )
    EVT_MENU( UIID_NEWGAMEDATADATABASE, MainWindow::OnNewResourceDatabase )
    EVT_MENU( UIID_SAVEGAMEDATADATABASE, MainWindow::OnSaveResourceDatabase )
    EVT_MENU( UIID_OPENGAMEDATADATABASE, MainWindow::OnLoadResourceDatabase )
    EVT_MENU( UIID_BUILDALL, MainWindow::OnBuildAllData )
    EVT_MENU( UIID_CLEAN, MainWindow::OnCleanAllData )
    EVT_MENU( UIID_SAVEWINDOWLAYOUT, MainWindow::OnSaveWindowLayout )
    EVT_MENU( UIID_LOADWINDOWLAYOUT, MainWindow::OnLoadWindowLayout )
    EVT_MENU( UIID_SETLAYOUTASDEFAULT, MainWindow::OnSetDefaultWindowLayout )
    EVT_MENU_RANGE( wxID_FILE1, wxID_FILE9, MainWindow::OnMRUFileEvent )
    EVT_MENU( wxID_EXIT, MainWindow::OnExitEvent )
    EVT_SIZE( MainWindow::OnSizeEvent )
	EVT_CLOSE( MainWindow::OnClose )
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const char* MainWindow::MainWindowTitle = "Heart Editor";

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::InitFrame()
{
    auiManager_ = new wxAuiManager();
	auiManager_->SetManagedWindow( this );

    PropertiesPane* properties = new PropertiesPane( this );
	DataLayoutPane* datalayout = new DataLayoutPane( this, &eventDispatcher_, properties );

	auiManager_->AddPane( datalayout, datalayout->GetDefaultPaneInfo() );
	auiManager_->AddPane( properties, properties->GetDefaultPaneInfo() );

	menuBar_ = new wxMenuBar();

    fileHistoryMenu_ = new wxMenu();

	//File Menu
    fileMenu_ = new wxMenu();
    fileMenu_->Append( UIID_NEWGAMEDATADATABASE, _T( "&New" ) );
    fileMenu_->Append( UIID_OPENGAMEDATADATABASE, _T( "&Open" ) );
    fileMenu_->Append( UIID_SAVEGAMEDATADATABASE, _T( "&Save" ) );
    fileMenu_->AppendSeparator();
    fileMenu_->Append( wxID_ANY, _T( "Recent Databases" ), fileHistoryMenu_ );
    fileMenu_->AppendSeparator();
    fileMenu_->Append( wxID_EXIT, _T( "E&xit" ) );
    menuBar_->Append( fileMenu_, "&File" );

    //Build Menu
    buildMenu_ = new wxMenu();
    buildMenu_->Append( UIID_BUILDALL, _T( "Build All" ) );
    buildMenu_->Append( UIID_CLEAN, _T( "Clean Data" ) );
    menuBar_->Append( buildMenu_, "&Build" );

    //Windows Menu
	windowsMenu_ = new wxMenu();
	windowsMenu_->Append( UIID_SAVEWINDOWLAYOUT, _T( "Save Window Layout" ) );
    windowsMenu_->Append( UIID_LOADWINDOWLAYOUT, _T( "Load Window Layout" ) );
    windowsMenu_->Append( UIID_SETLAYOUTASDEFAULT, _T( "Set Currnet Window Layout As Default" ) );
	windowsMenu_->AppendSeparator();
	windowsMenu_->Append( UIID_SHOWDATALAYOUT, _T( "&Properties" ) );

    windowsMenu_->Enable( UIID_SETLAYOUTASDEFAULT, false );

	menuBar_->Append( windowsMenu_, _T( "&Windows" ) );

	SetMenuBar( menuBar_ );

	Maximize();

	auiManager_->Update();

    EnableAllPanes( false );

    FirstTimeInit();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnShowDataLayout( wxCommandEvent& evt )
{
	wxAuiPaneInfo& i = auiManager_->GetPane( PropertiesPane::GetWindowPaneName() );
	i.Show();
	auiManager_->Update();
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void MainWindow::OnNewResourceDatabase( wxCommandEvent& evt )
{
    wxDirDialog dirSelect( 
        this, 
        "Choose database directory", 
        "",
        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if ( dirSelect.ShowModal() == wxID_CANCEL )
        return;

    mainResourceDatabase_ = new GameData::gdGameDatabaseObject();

    if ( mainResourceDatabase_->Create( dirSelect.GetPath().wc_str() ) != gdERROR_OK )
    {
        wxMessageDialog( this, "Could not create database at that directory" ).ShowModal();
        delete mainResourceDatabase_;
        mainResourceDatabase_ = NULL;
    }
    else
    {
        EnableAllPanes( true );
    }

    //Notify all of the new database (null pointer or not)
    eventDispatcher_.DispatchEvent( EVT_EVENT( db_loaded ), mainResourceDatabase_, 0 );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnSaveResourceDatabase( wxCommandEvent& evt )
{
    if ( !mainResourceDatabase_ )
    {
        wxMessageDialog( this, "No Database to save" ).ShowModal();
    }
    else
    {
        mainResourceDatabase_->Save();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnLoadResourceDatabase( wxCommandEvent& evt )
{
    EnableAllPanes( false );
    delete mainResourceDatabase_;
    mainResourceDatabase_ = NULL;
    eventDispatcher_.DispatchEvent( EVT_EVENT( db_loaded ), mainResourceDatabase_, 0 );

    wxDirDialog dirSelect( 
        this, 
        "Choose database directory", 
        "",
        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if ( dirSelect.ShowModal() == wxID_CANCEL )
        return;
    
    LoadDatabaseFromFolderPath( dirSelect.GetPath() );

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::LoadDatabaseFromFolderPath( const wxString& folderpath )
{
    mainResourceDatabase_ = new GameData::gdGameDatabaseObject();
    if ( mainResourceDatabase_->Load( folderpath.wc_str() ) == gdERROR_OK )
    {
        eventDispatcher_.DispatchEvent( EVT_EVENT( db_loaded ), mainResourceDatabase_, 0 );
        EnableAllPanes( true );

        fileHistory_.AddFileToHistory( folderpath );
        fileHistory_.Save( appConfig_ );
    }
    else
    {
        wxMessageDialog( this, "Failed To load database").ShowModal();
        delete mainResourceDatabase_;
        mainResourceDatabase_ = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnMRUFileEvent( wxCommandEvent& evt )
{
    wxString f( fileHistory_.GetHistoryFile( evt.GetId() - wxID_FILE1 ) );
    if ( !f.empty() )
    {
        LoadDatabaseFromFolderPath( f );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnExitEvent( wxCommandEvent& evt )
{
    Close();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnSizeEvent( wxSizeEvent& evt )
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnClose( wxCloseEvent& evt )
{
    appConfig_.SetPath( "UserPref" );

    appConfig_.Write( "width", GetSize().GetWidth() );
    appConfig_.Write( "height", GetSize().GetHeight() );
    appConfig_.Write( "maximize", IsMaximized() );

    appConfig_.SetPath( ".." );

	evt.Skip();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::EnableAllPanes( bool val )
{
    wxAuiPaneInfoArray& panes = auiManager_->GetAllPanes();
    pUint32 s = panes.size();

    for ( pUint32 i = 0; i < s; ++i )
    {
        panes[i].window->Enable( val );
    }
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::FirstTimeInit()
{
    appConfig_.SetPath( "UserPref" );

    if ( appConfig_.Read( "DefaultLayout", &lastLoadedLayout_ ) )
    {
        LoadLayoutFile( lastLoadedLayout_ );
    }
    
    int w,h;
    bool maximize;
    if ( appConfig_.Read( "width", &w ) &&
         appConfig_.Read( "height", &h ) )
    {
        SetClientSize( wxSize(w,h) );
    }

    if ( appConfig_.Read( "maximize", &maximize ) )
    {
        Maximize( maximize );
    }

    appConfig_.SetPath( ".." );

    fileHistory_.UseMenu( fileHistoryMenu_ );
    fileHistory_.Load( appConfig_ );
    fileMenu_->UpdateUI();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnSaveWindowLayout( wxCommandEvent& evt )
{
    wxFileDialog saveDialog( 
        this, 
        "Save Layout file", "", "",
        "LAYOUT files (*.layout)|*.layout", 
        wxFD_SAVE|wxFD_OVERWRITE_PROMPT );

    if ( saveDialog.ShowModal() == wxID_CANCEL )
        return;

    wxFileOutputStream file( saveDialog.GetPath() );
    if ( !file.IsOk() )
    {
        wxMessageDialog( this, "Error Saving Layout File" ).ShowModal();
        return ;
    }

    wxString layout = auiManager_->SavePerspective();
    file.Write( layout.c_str(), layout.size() );

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnLoadWindowLayout( wxCommandEvent& evt )
{
    wxFileDialog openDialog(
        this, 
        "Open Layout File",
        "",
        "",
        "LAYOUT files (*.layout)|*.layout",
        wxFD_OPEN|wxFD_FILE_MUST_EXIST );

    if ( openDialog.ShowModal() == wxID_CANCEL )
        return;

    LoadLayoutFile( openDialog.GetPath() );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnSetDefaultWindowLayout( wxCommandEvent& evt )
{
    if ( !lastLoadedLayout_.IsEmpty() )
    {
        appConfig_.SetPath( "UserPref" );

        appConfig_.Write( "DefaultLayout", lastLoadedLayout_ );

        appConfig_.SetPath( ".." );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::LoadLayoutFile( const wxString& path )
{
    wxFileInputStream file( path );
    if ( !file.IsOk() )
    {
        wxMessageDialog( this, "Error Opening Layout File" ).ShowModal();
        return ;
    }

    pChar* buffer = new pChar[file.GetLength()+1];

    file.Read( buffer, file.GetLength() );
    buffer[file.GetLength()] = 0;

    if ( !auiManager_->LoadPerspective( wxString( buffer ) ) )
        wxMessageDialog( this, "Error Parsing Layout File" ).ShowModal();
    else
    {
        windowsMenu_->Enable( UIID_SETLAYOUTASDEFAULT, true );
        lastLoadedLayout_ = path;
    }

    delete[] buffer;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnBuildAllData( wxCommandEvent& evt )
{
    if ( mainResourceDatabase_ )
    {
        wxProgressDialog progress/* = new wxProgressDialog*/( _T("Building..."), _T("Building Data..."), 102, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT );
        
        gdError res = mainResourceDatabase_->BuildAllResources( BuildProgressCallback, &progress );
        progress.Update( 102 );

        if ( res )
        {
            //Build Failed! Report
            wxRichMessageDialog msgDlg(this,"Build Failed.","Failed.",wxOK|wxCENTER);
            wxString msg;
            msg += mainResourceDatabase_->GetErrorMessages();
            msg += mainResourceDatabase_->GetWarningMessages();
            msg += mainResourceDatabase_->GetBuiltResourcesMessages();
            msgDlg.ShowDetailedText( msg );
            msgDlg.ShowModal();
        }
        else 
        {
            wxRichMessageDialog msgDlg(this,"Build Success.","Complete.",wxOK|wxCENTER);
            wxString msg;
            msg += mainResourceDatabase_->GetWarningMessages();
            msg += mainResourceDatabase_->GetBuiltResourcesMessages();
            msgDlg.ShowDetailedText( msg );
            msgDlg.ShowModal();
        }

        mainResourceDatabase_->ClearBuildMessages();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

gdBool MainWindow::BuildProgressCallback( const GameData::gdUniqueResourceID& res, gdUint progress, void* user )
{
    wxProgressDialog* dialog = (wxProgressDialog*)user;
    wxString msg;
    msg.Printf( "Building...\n\t%s%s\n\tCRC32::0x%08X", res.GetResourcePath(), res.GetResourceName(), res.GetResourceCRCID() );
    return dialog->Update( progress, msg );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnCleanAllData( wxCommandEvent& evt )
{
    if ( mainResourceDatabase_ )
    {
        wxWindowDisabler disableAll;
        wxBusyInfo wait("Please wait, working...");

        mainResourceDatabase_->CleanData();
    }
}
