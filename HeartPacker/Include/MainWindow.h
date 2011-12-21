/********************************************************************

	filename: 	MainWindow.h	
	
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
#ifndef MAINWINDOW_H__
#define MAINWINDOW_H__

#include "EventDispatcher.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class MainWindow : public wxFrame
{
public:
	MainWindow()
		: wxFrame( NULL, -1, MainWindowTitle )
        , mainResourceDatabase_( NULL )
        , appConfig_( "HeartPacker", "", "heartpacker.config", "heartpacker.config" )
        , auiManager_(NULL)
	{
		InitFrame();
	}
	virtual ~MainWindow()
	{
        delete mainResourceDatabase_;
        mainResourceDatabase_ = NULL;
		auiManager_->UnInit();
        delete auiManager_;
        auiManager_ = NULL;
	}

    GameData::gdGameDatabaseObject* GetGamedatabase() const { return mainResourceDatabase_; }

private:

	DECLARE_EVENT_TABLE();

	static const char* MainWindowTitle;

	void			InitFrame();
    void            FirstTimeInit();
    void            EnableAllPanes( bool val );
    void            LoadLayoutFile( const wxString& path );
    void            LoadDatabaseFromFolderPath( const wxString& folderpath );
    static gdBool   BuildProgressCallback( const GameData::gdUniqueResourceID& res, gdUint progress, void* user );

	// Events
	void			OnShowDataLayout( wxCommandEvent& evt );
    void            OnNewResourceDatabase( wxCommandEvent& evt );
    void            OnSaveResourceDatabase( wxCommandEvent& evt );
    void            OnLoadResourceDatabase( wxCommandEvent& evt );
    void            OnBuildAllData( wxCommandEvent& evt );
    void            OnCleanAllData( wxCommandEvent& evt );
    void            OnSaveWindowLayout( wxCommandEvent& evt );
    void            OnLoadWindowLayout( wxCommandEvent& evt );
    void            OnSetDefaultWindowLayout( wxCommandEvent& evt );
    void            OnSizeEvent( wxSizeEvent& evt );
    void            OnMRUFileEvent( wxCommandEvent& evt );
    void            OnExitEvent( wxCommandEvent& evt );
	void			OnClose( wxCloseEvent& evt );

    EventDispatcher                     eventDispatcher_;
	wxMenuBar*		                    menuBar_;
	wxMenu*			                    windowsMenu_;
    wxMenu*                             fileMenu_;
    wxMenu*                             buildMenu_;
    wxMenu*                             preferencesMenu_;
    wxMenu*                             fileHistoryMenu_;
    wxFileHistory                       fileHistory_;
	wxAuiManager*	                    auiManager_;
    GameData::gdGameDatabaseObject*     mainResourceDatabase_;
    wxFileConfig                        appConfig_;
    wxString                            lastLoadedLayout_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class PackerApp : public wxApp
{
public:
	virtual bool OnInit();

private:

	MainWindow*	mainFrame_;
};

#endif // MAINWINDOW_H__