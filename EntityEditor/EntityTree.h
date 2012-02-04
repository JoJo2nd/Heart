/********************************************************************

	filename: 	EntityTree.h	
	
	Copyright (c) 2:2:2012 James Moran
	
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
#ifndef ENTITYTREE_H__
#define ENTITYTREE_H__

#include "EntityEditorConfig.h"
#include "EntityEditor.h"
#include "entitylib\EntityLib.h"

class MainWindow;

class EntityTreeView : public wxTreeCtrl
{
public:
    EntityTreeView( MainWindow* parent, wxPropertyGrid* propertyGrid )
        : wxTreeCtrl( parent, uiID_ENTITYTREEVIEW, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_HIDE_ROOT )
        , parent_(parent)
        , contextMenu_(NULL)
        , propertyGrid_(propertyGrid)
    {
        InitWindowWidgets();
    }

    void RefeshView();

private:

    DECLARE_EVENT_TABLE();

    struct ItemData : public wxTreeItemData
    {
        Entity::IEntityDefinitionView* entity_;
    };

    void InitWindowWidgets();
    void AddEntityToTree( Entity::IEntityDefinitionView* entity );
    void UpdatePropertyView( Entity::IEntityDefinitionView* entity );
    //Tree View
    void OnNewEntity( wxCommandEvent& evt );
    void OnDeleteEntity( wxCommandEvent& evt );
    void OnAddComponents( wxCommandEvent& evt );
    void OnRemoveComponents( wxCommandEvent& evt );
    void OnEntityContextMenu( wxMouseEvent& evt );
    void OnSelectionChange( wxTreeEvent& evt );
    void OnPropertyGridChangedEvent( wxPropertyGridEvent& evt );

    MainWindow*     parent_;
    wxMenu*         contextMenu_;
    wxTreeItemId    contextItem_;
    wxPropertyGrid* propertyGrid_;
};

#endif // ENTITYTREE_H__