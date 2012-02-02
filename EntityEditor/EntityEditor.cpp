/********************************************************************

	filename: 	EntityEditor.cpp	
	
	Copyright (c) 28:1:2012 James Moran
	
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
#include "EntityEditor.h"
#include "EntityTree.h"
#include "entitylib\EntityLib.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class EntityEditorApp : public wxApp
{
public:
    virtual bool OnInit();

private:

    MainWindow*	mainFrame_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_APP( EntityEditorApp );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool EntityEditorApp::OnInit()
{
    WCHAR szWindowClass[MAX_PATH];
    GetCurrentDirectory( MAX_PATH, szWindowClass );

    wxInitAllImageHandlers();

    mainFrame_ = new MainWindow();

    mainFrame_->Show();
    SetTopWindow( mainFrame_ );

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( MainWindow, wxFrame )
    EVT_MENU( wxID_NEW,     MainWindow::OnNewEvent )
    EVT_MENU( wxID_OPEN,    MainWindow::OnOpenEvent )
    EVT_MENU( wxID_SAVE,    MainWindow::OnSaveEvent )
    EVT_MENU( wxID_SAVEAS,  MainWindow::OnSaveAsEvent )
    EVT_MENU( uiID_IMPORT,  MainWindow::OnImportEvent )
    EVT_MENU( uiID_EXPORT,  MainWindow::OnExportEvent )
    EVT_MENU( wxID_EXIT,    MainWindow::OnExitEvent )
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::InitWindowWidgets()
{
    fileMenu_ = new wxMenu();
    fileMenu_->Append( wxID_NEW, _T( "&New" ) );
    fileMenu_->Append( wxID_OPEN, _T( "&Open" ) );
    fileMenu_->Append( wxID_SAVE, _T( "&Save" ) );
    fileMenu_->Append( wxID_SAVEAS, _T( "S&ave As..." ) );
    fileMenu_->AppendSeparator();
    fileMenu_->Append( uiID_IMPORT, _T( "&Import" ) );
    fileMenu_->Append( uiID_EXPORT, _T( "&Export" ) );
    fileMenu_->AppendSeparator();
    fileMenu_->Append( wxID_EXIT, _T( "E&xit" ) );

    SetEnableContextMenuBarOptions( false );

    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append( fileMenu_, _T( "&File" ) );

    SetMenuBar( menuBar );

    entityTree_ = new EntityTreeView( this );
    entityProp_ = new wxPropertyGrid( this, uiID_ENTITYPROPVIEW );

    wxFlexGridSizer* sizer = new wxFlexGridSizer( 2, 0, 0 );
    sizer->Add( entityTree_, 0, wxEXPAND );
    sizer->Add( entityProp_, 0, wxEXPAND );
    sizer->AddGrowableCol( 0 );
    sizer->AddGrowableCol( 1 );
    sizer->AddGrowableRow( 0 );

    SetSizer( sizer );
    SetAutoLayout( true );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::SetEnableContextMenuBarOptions( bool enable )
{
    fileMenu_->Enable( wxID_SAVE, enable );
    fileMenu_->Enable( wxID_SAVEAS, enable );
    fileMenu_->Enable( uiID_IMPORT, enable );
    fileMenu_->Enable( uiID_EXPORT, false/*enable*/ );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnNewEvent( wxCommandEvent& /*evt*/ )
{
    library_ = CreateEntityLibraryView();
    SetEnableContextMenuBarOptions( library_ ? true : false );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnOpenEvent( wxCommandEvent& /*evt*/ )
{
    wxFileDialog openFileDialog(this, _("Open COMXML Component file..."), "", "", "COMXML files (*.comxml)|*.comxml", wxFD_OPEN | wxFD_FILE_MUST_EXIST );

    if ( openFileDialog.ShowModal() != wxID_OK )
        return;

    if ( library_ )
    {
        DestroyEntityLibraryView( library_ );
    }

    library_ = CreateEntityLibraryView();

    Entity::IErrorObject er = library_->LoadLibrary( openFileDialog.GetPath().c_str() );
    if ( !er )
    {
        wxMessageDialog(this, er.GetErrorString() ).ShowModal();
        DestroyEntityLibraryView( library_ );
        library_ = NULL;
    }

    SetEnableContextMenuBarOptions( library_ ? true : false );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnSaveEvent( wxCommandEvent& /*evt*/ )
{
    if ( library_ )
    {
        wxFileDialog saveFileDialog(this, _("Save COMXML Component file..."), "", "", "COMXML files (*.comxml)|*.comxml", wxFD_SAVE );

        if ( saveFileDialog.ShowModal() != wxID_OK )
            return;

        Entity::IErrorObject er = library_->SaveLibrary( saveFileDialog.GetPath().c_str() );
        if ( !er )
        {
            wxMessageDialog(this, er.GetErrorString() ).ShowModal();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnSaveAsEvent( wxCommandEvent& /*evt*/ )
{
    if ( library_ )
    {
        wxFileDialog saveFileDialog(this, _("Save COMXML Component file..."), "", "", "COMXML files (*.comxml)|*.comxml", wxFD_SAVE );

        if ( saveFileDialog.ShowModal() != wxID_OK )
            return;

        Entity::IErrorObject er = library_->SaveLibrary( saveFileDialog.GetPath().c_str() );
        if ( !er )
        {
            wxMessageDialog(this, er.GetErrorString() ).ShowModal();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnImportEvent( wxCommandEvent& /*evt*/ )
{
    if ( library_ )
    {
        wxFileDialog openFileDialog(this, _("Import XML Component file..."), "", "", "XML files (*.xml)|*.xml", wxFD_OPEN | wxFD_FILE_MUST_EXIST );

        if ( openFileDialog.ShowModal() != wxID_OK )
            return;


        library_->ImportComponetsFromXML( openFileDialog.GetPath().c_str() );

    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnExportEvent( wxCommandEvent& /*evt*/ )
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnExitEvent( wxCommandEvent& /*evt*/ )
{
    Close();
}



