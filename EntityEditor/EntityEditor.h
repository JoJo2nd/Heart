/********************************************************************

	filename: 	EntityEditor.h	
	
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

#ifndef ENTITYEDITOR_H__
#define ENTITYEDITOR_H__

namespace Entity
{
    class ILibraryView;
}

class MainWindow : public wxFrame
{
public:
    MainWindow()
        : wxFrame(NULL, -1, "Heart Entity Editor")
        , appConfig_("HeartEntityEditor", "", "heartentityeditor.config", "heartentityeditor.config")
        , entityTree_(NULL)
        , entityProp_(NULL)
        , library_(NULL)
    {
        InitWindowWidgets();
    }
    virtual ~MainWindow()
    {
    }

private:

    DECLARE_EVENT_TABLE();

    enum 
    {
        uiID_IMPORT,
        uiID_EXPORT,
    };

    void InitWindowWidgets();
    void OnNewEvent( wxCommandEvent& evt );
    void OnOpenEvent( wxCommandEvent& evt );
    void OnSaveEvent( wxCommandEvent& evt );
    void OnSaveAsEvent( wxCommandEvent& evt );
    void OnImportEvent( wxCommandEvent& evt );
    void OnExportEvent( wxCommandEvent& evt );
    void OnExitEvent( wxCommandEvent& evt );
    void SetEnableContextMenuOptions( bool enable );

    wxMenu*                 fileMenu_;
    wxFileConfig            appConfig_;
    wxTreeCtrl*             entityTree_;
    wxPropertyGrid*         entityProp_;
    Entity::ILibraryView*   library_;
};

#endif // ENTITYEDITOR_H__

