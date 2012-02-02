/********************************************************************

	filename: 	EntityTree.cpp	
	
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

#include "stdafx.h"
#include "EntityTree.h"
#include "entitylib\EntityLib.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( EntityTreeView, wxTreeCtrl )
    EVT_RIGHT_DOWN( EntityTreeView::OnEntityContextMenu )
    EVT_MENU( uiID_NEWENTITY,        EntityTreeView::OnNewEntity )
    EVT_MENU( uiID_DELETEENTITY,     EntityTreeView::OnDeleteEntity )
    EVT_MENU( uiID_ADDCOMPONENTS,    EntityTreeView::OnAddComponents )
    EVT_MENU( uiID_REMOVECOMPONENTS, EntityTreeView::OnRemoveComponents )
END_EVENT_TABLE()


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::InitWindowWidgets()
{
    contextMenu_ = new wxMenu();
    contextMenu_->Append( uiID_NEWENTITY, _T( "New Entity" ) );
    contextMenu_->Append( uiID_ADDCOMPONENTS, _T( "Add Components..." ) );
    contextMenu_->Append( uiID_REMOVECOMPONENTS, _T( "Remove Components..." ) );
    contextMenu_->Append( uiID_DELETEENTITY, _T( "Delete Entity" ) );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::OnNewEntity( wxCommandEvent& evt )
{
    Entity::IErrorObject er = parent_->GetLibraryView()->AddEntity( "test" );

    if ( !er )
    {
        wxMessageDialog(this,er.GetErrorString()).ShowModal();
        return;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::OnDeleteEntity( wxCommandEvent& evt )
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::OnAddComponents( wxCommandEvent& evt )
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::OnRemoveComponents( wxCommandEvent& evt )
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::OnEntityContextMenu( wxMouseEvent& evt )
{
    if ( !parent_->GetLibraryView() )
        return;

    wxTreeItemId item = HitTest( evt.GetPosition() );

    if ( !item.IsOk() )
    {
        contextMenu_->Enable( uiID_ADDCOMPONENTS, false );
        contextMenu_->Enable( uiID_REMOVECOMPONENTS, false );
        contextMenu_->Enable( uiID_DELETEENTITY, false );
    }
    else
    {
        contextMenu_->Enable( uiID_ADDCOMPONENTS, true );
        contextMenu_->Enable( uiID_REMOVECOMPONENTS, true );
        contextMenu_->Enable( uiID_DELETEENTITY, true );
    }

    PopupMenu( contextMenu_ );
}
