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
    EVT_TREE_SEL_CHANGED( uiID_ENTITYTREEVIEW, EntityTreeView::OnSelectionChange )
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

    AddRoot( "root" );

    propertyGrid_->Bind( wxEVT_PG_CHANGED, &EntityTreeView::OnPropertyGridChangedEvent, this, propertyGrid_->GetId() );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::OnNewEntity( wxCommandEvent& evt )
{
    wxTextEntryDialog nameEntry( this, "", "Enter Entity Name...", "entity" );

    if ( nameEntry.ShowModal() == wxID_OK )
    {
        Entity::IEntityDefinitionView* result;
        Entity::IErrorObject er = parent_->GetLibraryView()->AddEntity( nameEntry.GetValue(), &result );

        if ( !er )
        {
            wxMessageDialog(this,er.GetErrorString()).ShowModal();
            return;
        }

        AddEntityToTree( result );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::OnDeleteEntity( wxCommandEvent& evt )
{
    Entity::ILibraryView* library = parent_->GetLibraryView();
    wxArrayString choices;

    if ( !contextItem_.IsOk() || !library ) 
        return;

    Entity::IEntityDefinitionView* entity = static_cast< ItemData* >( GetItemData( contextItem_ ) )->entity_;
    library->RemoveEntity( entity );
    Delete( contextItem_ );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::OnAddComponents( wxCommandEvent& evt )
{
    Entity::ILibraryView* library = parent_->GetLibraryView();
    wxArrayString choices;

    if ( !contextItem_.IsOk() || !library ) 
        return;

    Entity::IEntityDefinitionView* entity = static_cast< ItemData* >( GetItemData( contextItem_ ) )->entity_;
    uint count = library->GetComponentCount();
    for ( uint i = 0; i < count; ++i )
    {
        if ( !entity->HasComponent( library->GetComponentView(i) ) )
        {
            choices.push_back( library->GetComponentView(i)->GetName() );
        }
    }

    if ( choices.size() == 0 )
    {
        wxMessageDialog( this, "Entity Contains all possible components." ).ShowModal();
        return;
    }

    wxMultiChoiceDialog dlg( this, "Select Components...", "Select Components...", choices );
    
    if ( dlg.ShowModal() == wxID_OK )
    {
        //Get choices...
        wxArrayInt sel = dlg.GetSelections();
        for ( uint i = 0; i < sel.size(); ++i )
        {
            Entity::IErrorObject er = entity->AppendComponent( library->GetComponentViewByName( choices[sel[i]].c_str() ) );
            if ( !er )
            {
                wxMessageDialog(this,er.GetErrorString()).ShowModal();
                return;
            }
        }

        UpdatePropertyView( entity );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::OnRemoveComponents( wxCommandEvent& evt )
{
    Entity::ILibraryView* library = parent_->GetLibraryView();
    wxArrayString choices;

    if ( !contextItem_.IsOk() || !library ) 
        return;

    Entity::IEntityDefinitionView* entity = static_cast< ItemData* >( GetItemData( contextItem_ ) )->entity_;
    uint count = entity->GetComponentCount();
    for ( uint i = 0; i < count; ++i )
    {
        choices.push_back( entity->GetComponent(i)->GetName() );
    }

    if ( choices.size() == 0 )
    {
        wxMessageDialog( this, "Entity does not contain any components." ).ShowModal();
        return;
    }

    wxMultiChoiceDialog dlg( this, "Select Components...", "Select Components...", choices );

    if ( dlg.ShowModal() == wxID_OK )
    {
        //Get choices...
        wxArrayInt sel = dlg.GetSelections();
        for ( uint i = 0; i < sel.size(); ++i )
        {
            Entity::IErrorObject er = entity->RemoveComponent( choices[sel[i]].c_str() );
            if ( !er )
            {
                wxMessageDialog(this,er.GetErrorString()).ShowModal();
                return;
            }
        }

        UpdatePropertyView( entity );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::OnEntityContextMenu( wxMouseEvent& evt )
{
    if ( !parent_->GetLibraryView() )
        return;

    contextItem_ = HitTest( evt.GetPosition() );

    if ( !contextItem_.IsOk() )
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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::RefeshView()
{
    DeleteChildren( GetRootItem() );

    if ( !parent_->GetLibraryView() )
        return;

    uint count = parent_->GetLibraryView()->GetEntityDefinitionCount();
    for ( uint i = 0; i < count; ++i )
    {
        AddEntityToTree( parent_->GetLibraryView()->GetEntityDefinition( i ) );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::AddEntityToTree( Entity::IEntityDefinitionView* entity )
{
    ItemData* d = new ItemData();
    d->entity_ = entity;
    AppendItem( GetRootItem(), entity->GetName(), -1, -1, d );
    // GetRootItem(),
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::UpdatePropertyView( Entity::IEntityDefinitionView* entity )
{
    propertyGrid_->Clear();

    if ( !entity )
        return;

    uint comps = entity->GetComponentCount();
    for ( uint i = 0; i < entity->GetComponentCount(); ++i )
    {
        propertyGrid_->Append( new wxPropertyCategory( entity->GetComponent(i)->GetName() ) );
    }
    
    uint props = entity->GetPropertyCount();
    for ( uint i = 0; i < props; ++i )
    {
        Entity::IComponentProperty* prop = entity->GetProperty( i );
        switch ( prop->GetValueType() )
        {
        case Entity::PropertyValueType_Bool:
            {
                propertyGrid_->AppendIn(
                        prop->GetOwnerComponent()->GetName(),
                        new wxBoolProperty( 
                            prop->GetName(),
                            wxPG_LABEL,
                            prop->GetValueBool() ) );
                propertyGrid_->SetPropertyAttribute( prop->GetName(), wxPG_BOOL_USE_CHECKBOX, true );
                propertyGrid_->SetPropertyClientData( prop->GetName(), prop );
                break;
            }
        case Entity::PropertyValueType_Int:
            {
                propertyGrid_->AppendIn(
                    prop->GetOwnerComponent()->GetName(),
                    new wxIntProperty( 
                        prop->GetName(),
                        wxPG_LABEL,
                        prop->GetValueInt() ) );
                propertyGrid_->SetPropertyClientData( prop->GetName(), prop );
                break;
            }
        case Entity::PropertyValueType_Uint:
            {
                propertyGrid_->AppendIn(
                    prop->GetOwnerComponent()->GetName(),
                    new wxUIntProperty( 
                        prop->GetName(),
                        wxPG_LABEL,
                        prop->GetValueUint() ) );
                propertyGrid_->SetPropertyClientData( prop->GetName(), prop );
            }
        case Entity::PropertyValueType_Float:
            {
                propertyGrid_->AppendIn(
                    prop->GetOwnerComponent()->GetName(),
                    new wxFloatProperty( 
                        prop->GetName(),
                        wxPG_LABEL,
                        prop->GetValueFloat() ) );
                propertyGrid_->SetPropertyClientData( prop->GetName(), prop );
                break;
            }
        case Entity::PropertyValueType_String:
            {
                propertyGrid_->AppendIn(
                    prop->GetOwnerComponent()->GetName(),
                    new wxStringProperty( 
                        prop->GetName(),
                        wxPG_LABEL,
                        prop->GetValueString() ) );
                propertyGrid_->SetPropertyClientData( prop->GetName(), prop );
            }
            break;
        case Entity::PropertyValueType_ResourceAssetID:
            {
                propertyGrid_->AppendIn(
                    prop->GetOwnerComponent()->GetName(),
                    new wxUIntProperty( 
                        prop->GetName(),
                        wxPG_LABEL,
                        prop->GetValueUint() ) );
                propertyGrid_->SetPropertyClientData( prop->GetName(), prop );
            }
            break;
        default:
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::OnSelectionChange( wxTreeEvent& evt )
{
    wxTreeItemId itemid = evt.GetItem();
    if ( !itemid.IsOk() )
    {
        UpdatePropertyView( NULL );
    }
    else
    {
        Entity::IEntityDefinitionView* entity = static_cast< ItemData* >( GetItemData( itemid ) )->entity_;
        UpdatePropertyView( entity );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EntityTreeView::OnPropertyGridChangedEvent( wxPropertyGridEvent& evt )
{
    Entity::IComponentProperty* prop = static_cast< Entity::IComponentProperty* >( propertyGrid_->GetPropertyClientData( evt.GetPropertyName() ) );
    switch ( prop->GetValueType() )
    {
    case Entity::PropertyValueType_Bool:
        {
            prop->SetValueBool( evt.GetPropertyValue().GetBool() );
            break;
        }
    case Entity::PropertyValueType_Int:
        {
            prop->SetValueInt( evt.GetPropertyValue().GetLong() );
            break;
        }
    case Entity::PropertyValueType_Uint:
        {
            prop->SetValueUint( evt.GetPropertyValue().GetULongLong().GetLo() );
            break;
        }
    case Entity::PropertyValueType_Float:
        {
            prop->SetValueFloat( (float)evt.GetPropertyValue().GetDouble() );
            break;
        }
    case Entity::PropertyValueType_String:
        {
            prop->SetValueString( evt.GetPropertyValue().GetString() );
            break;
        }
        break;
    case Entity::PropertyValueType_ResourceAssetID:
        {
            prop->SetValueUint( evt.GetPropertyValue().GetULongLong().GetLo() );
            break;
        }
    default:
        break;
    }

}
