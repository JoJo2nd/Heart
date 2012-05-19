/********************************************************************

	filename: 	DataLayoutPane.h	
	
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

#ifndef DATALAYOUTPANE_H__
#define DATALAYOUTPANE_H__

#include "EventDispatcher.h"

class PropertiesPane;

#define GAMEDATA_ROOT_NODE_NAME     "GAMEDATA:/"

class DataLayoutPane : public wxTreeCtrl
                     , public EventListener
{
public: 
	DataLayoutPane( 
        wxWindow* parent, 
        EventDispatcher* evtDispatcher, 
        PropertiesPane* propertiesPane )
		    : wxTreeCtrl( 
                parent, 
                GetMenuID(),
                wxDefaultPosition, 
                wxDefaultSize,
                DEFAULT_STYLE )
            , itemContextMenu_(NULL)
            , mainDatabase_(NULL)
            , propertiesPane_(propertiesPane)
            , selectedRes_(NULL)
	{
		instance_ = this;

		defaultPaneInfo_.Dock();
		defaultPaneInfo_.Floatable( false );
		defaultPaneInfo_.Caption( GetWindowPaneName() );
		defaultPaneInfo_.Name( GetWindowPaneName() );
		defaultPaneInfo_.Show();
		defaultPaneInfo_.MinSize( 100, 300 );
		defaultPaneInfo_.MaximizeButton( true );
		defaultPaneInfo_.PinButton( false );
		defaultPaneInfo_.CloseButton( false );
		defaultPaneInfo_.CentrePane();

        evtDispatcher->RegisterListener( this );

        InitMenuCompents();
	}

    void InitMenuCompents();

    ~DataLayoutPane() 
	{
        //UnregisterListener();
        delete itemContextMenu_;
        itemContextMenu_ = NULL;
		instance_ = NULL;
	}

	DEFINE_WINDOW_PANE_INSTANCE( DataLayoutPane );

private:
    
    DECLARE_EVENT_TABLE();

    static const unsigned long DEFAULT_STYLE = 
        wxTR_HAS_BUTTONS | 
        wxTR_TWIST_BUTTONS |
        wxTR_LINES_AT_ROOT | 
        wxTR_EDIT_LABELS |
        wxTR_MULTIPLE;

    class DataTreeItem : public wxTreeItemData
    {
    public:
        DataTreeItem()
            : resourcesCRC32_(0)
            , isDirectory_(false)
        {

        }
        ~DataTreeItem()
        {

        }

        bool                GetIsDirectory() const { return isDirectory_; }
        void                SetIsDirectory( bool v ) { isDirectory_ = v; }
        pUint32             GetResourceCRC32() const { return resourcesCRC32_; }
        void                SetResourceCRC32( pUint32 v ) { resourcesCRC32_ = v; }

    private:
        
        pUint32             resourcesCRC32_;      
        bool                isDirectory_;
    };

    enum ContextMenuIDs
    {
        UIID_ADD_FOLDER,
        UIID_BUILD_RESOURCE,
        UIID_CLEAN_RESOURCE,
    };

    //
    void AddResourceItem( const wxString& inputFile, const wxTreeItemId& parentItem );
    void AddResourceItem( const GameData::gdUniqueResourceID& resID );
    void DeleteResourceFromDatabase( const wxTreeItemId& item );
    void RenameResourceInDatabase( const wxTreeItemId& item, const wxString& newName );
    void BuildFullFilePath( const wxTreeItemId& lastItem, wxString& builtPath );
    void RebuildTreeViewFromDatabase();
    wxTreeItemId GetTreeItemPathNode( const gdChar* param1 );
    void UpdatePropertiesView( const GameData::gdResourceInfo* res );
    void AppendResourceParameterToPropertyGrid( const GameData::gdParametersArrayType &params, pUint32 i );

    //Events
    void EventRecieve( const Event& evt );
    void OnContextMenuRequest( wxTreeEvent& ev );
    void OnBeginDragEvent( wxTreeEvent& ev );
    void OnEndDragEvent( wxTreeEvent& ev );
    void OnBeginLabelTextEdit( wxTreeEvent& ev );
    void OnEndLableTextEdit( wxTreeEvent& ev );
    void OnToolTipRequest( wxTreeEvent& ev );
    void OnDeleteTreeItemEvent( wxTreeEvent& ev );
    void OnItemSelected( wxTreeEvent& ev );
    void OnFileDropEvent( wxDropFilesEvent& ev ); 
    void OnKeyDownEvent( wxTreeEvent& ev );
    void OnContextMenuDelete( wxCommandEvent& ev );
    void OnContextMenuAddFolder( wxCommandEvent& ev );
    void OnContextMenuBuild( wxCommandEvent& ev );
    void OnContextMenuClean( wxCommandEvent& ev );
    void OnPropertyGridChangingEvent( wxPropertyGridEvent& evt );
    void OnPropertyGridChangedEvent( wxPropertyGridEvent& evt );



    GameData::gdGameDatabaseObject*     mainDatabase_;
    PropertiesPane*                     propertiesPane_;
    wxMenu*                             itemContextMenu_;
    wxTreeItemId                        selectedItem_;
    GameData::gdResourceInfo*           selectedRes_;
};

#endif // DATALAYOUTPANE_H__