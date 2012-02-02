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

class MainWindow;

class EntityTreeView : public wxTreeCtrl
{
public:
    EntityTreeView( MainWindow* parent )
        : wxTreeCtrl( parent, uiID_ENTITYTREEVIEW )
        , parent_(parent)
        , contextMenu_(NULL)
    {
        InitWindowWidgets();
    }

private:

    DECLARE_EVENT_TABLE();

    void InitWindowWidgets();
    //Tree View
    void OnNewEntity( wxCommandEvent& evt );
    void OnDeleteEntity( wxCommandEvent& evt );
    void OnAddComponents( wxCommandEvent& evt );
    void OnRemoveComponents( wxCommandEvent& evt );
    void OnEntityContextMenu( wxMouseEvent& evt );

    MainWindow*     parent_;
    wxMenu*         contextMenu_;
};

#endif // ENTITYTREE_H__