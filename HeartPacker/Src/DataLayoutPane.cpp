/********************************************************************

	filename: 	DataLayoutPane.cpp	
	
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
#include "PropertiesPane.h"
#include "DataLayoutPane.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DECLARE_WINDOW_PANE_INSTANCE( DataLayoutPane );


BEGIN_EVENT_TABLE( DataLayoutPane, wxTreeCtrl )
    EVT_TREE_ITEM_MENU( DataLayoutPane::GetMenuID(), DataLayoutPane::OnContextMenuRequest )
    EVT_TREE_BEGIN_DRAG( DataLayoutPane::GetMenuID(), DataLayoutPane::OnBeginDragEvent )
    EVT_TREE_END_DRAG( DataLayoutPane::GetMenuID(), DataLayoutPane::OnEndDragEvent )
    EVT_TREE_BEGIN_LABEL_EDIT( DataLayoutPane::GetMenuID(), DataLayoutPane::OnBeginLabelTextEdit )
    EVT_TREE_END_LABEL_EDIT( DataLayoutPane::GetMenuID(), DataLayoutPane::OnEndLableTextEdit )
    EVT_TREE_ITEM_GETTOOLTIP( DataLayoutPane::GetMenuID(), DataLayoutPane::OnToolTipRequest )
    EVT_TREE_DELETE_ITEM( DataLayoutPane::GetMenuID(), DataLayoutPane::OnDeleteTreeItemEvent )
    EVT_TREE_SEL_CHANGED( DataLayoutPane::GetMenuID(), DataLayoutPane::OnItemSelected )
    EVT_MENU( wxID_DELETE, DataLayoutPane::OnContextMenuDelete )
    EVT_MENU( UIID_ADD_FOLDER, DataLayoutPane::OnContextMenuAddFolder )
    EVT_MENU( UIID_BUILD_RESOURCE, DataLayoutPane::OnContextMenuBuild )
    EVT_TREE_KEY_DOWN( DataLayoutPane::GetMenuID(), DataLayoutPane::OnKeyDownEvent )
    EVT_DROP_FILES( DataLayoutPane::OnFileDropEvent )
    //EVT_PG_CHANGING( PropertiesPane::GetMenuID(), DataLayoutPane::OnPropertyGridChangingEvent )
    //EVT_PG_CHANGED( PropertiesPane::GetMenuID(), DataLayoutPane::OnPropertyGridChangedEvent )
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::InitMenuCompents()
{
    RebuildTreeViewFromDatabase();

    //Allow File Drops
    DragAcceptFiles( true );

    itemContextMenu_ = new wxMenu();
    itemContextMenu_->Append( wxID_DELETE, "Delete Resource" );
    itemContextMenu_->Append( UIID_ADD_FOLDER, "Add Folder" );
    itemContextMenu_->Append( UIID_BUILD_RESOURCE, "Build Resource" );

    propertiesPane_->Bind( wxEVT_PG_CHANGED, &DataLayoutPane::OnPropertyGridChangedEvent, this, propertiesPane_->GetMenuID() );
    propertiesPane_->Bind( wxEVT_PG_CHANGING, &DataLayoutPane::OnPropertyGridChangingEvent, this, propertiesPane_->GetMenuID() );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnContextMenuRequest( wxTreeEvent& ev )
{
    selectedItem_ = ev.GetItem();

    if ( selectedItem_.IsOk() )
    {
        DataTreeItem* itemData = static_cast< DataTreeItem* >( GetItemData( selectedItem_ ) );
        // Better solution is to pick a precreated menu based on selection.
        if ( !itemData->GetIsDirectory() )
        {
            itemContextMenu_->Enable( UIID_ADD_FOLDER, false );
            itemContextMenu_->Enable( UIID_BUILD_RESOURCE, true );
        }
        else
        {
            itemContextMenu_->Enable( UIID_ADD_FOLDER, true );
            itemContextMenu_->Enable( UIID_BUILD_RESOURCE, false );
        }

        PopupMenu( itemContextMenu_ );
    }

    selectedItem_ = GetRootItem();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnBeginDragEvent( wxTreeEvent& ev )
{
    DataTreeItem* itemData = static_cast< DataTreeItem* >( GetItemData( ev.GetItem() ) );
    if ( ev.GetItem() == GetRootItem() || !itemData || itemData->GetIsDirectory() )
    {
        ev.Veto();
    }
    else
    {
        selectedItem_ = ev.GetItem();
        ev.Allow();
    }
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnEndDragEvent( wxTreeEvent& ev )
{
    wxTreeItemId item = selectedItem_;
    DataTreeItem* itemData = static_cast< DataTreeItem* >( GetItemData( item ) );
    wxTreeItemId destItem = ev.GetItem();
    DataTreeItem* destItemData = static_cast< DataTreeItem* >( GetItemData( destItem ) );

    if ( !destItem.IsOk() )
    {
        ev.Veto();
        return;
    }

    if ( !destItemData->GetIsDirectory() )
    {
        destItem = GetItemParent( destItem );
    }

    pAssert( item.IsOk() );
    pAssert( mainDatabase_ );

    wxString newPath;
    BuildFullFilePath( destItem, newPath );

    GameData::gdUniqueResourceID resid;
    if ( mainDatabase_->GetResourceIDByCRC( itemData->GetResourceCRC32(), &resid ) != gdERROR_OK )
    {
        wxMessageDialog( this, "Couldn't find item in database" ).ShowModal();
        ev.Veto();
        return;
    }

    pUint32 newCRC;
    if ( mainDatabase_->MoveResource( resid, newPath.c_str(), &newCRC ) != gdERROR_OK )
    {
        wxMessageDialog( this, "Couldn't rename item due to Name CRC Conflict" ).ShowModal();
        ev.Veto();
        return;
    }

    //Set the item data to match the new name
    DataTreeItem* newItemData = new DataTreeItem();
    *newItemData = *itemData;
    newItemData->SetResourceCRC32( newCRC );
    PrependItem( destItem, GetItemText( item ), -1, -1, newItemData );
    
    Delete( item );

    ev.Allow();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnBeginLabelTextEdit( wxTreeEvent& ev )
{
    DataTreeItem* itemData = static_cast< DataTreeItem* >( GetItemData( ev.GetItem() ) );
    if ( ev.GetItem() == GetRootItem() || !itemData || itemData->GetIsDirectory() )
        ev.Veto();
    else
        ev.Allow();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnEndLableTextEdit( wxTreeEvent& ev )
{
    wxTreeItemId item = ev.GetItem();
    DataTreeItem* itemData = static_cast< DataTreeItem* >( GetItemData( item ) );
    pAssert( item.IsOk() );
    pAssert( mainDatabase_ );

    //We control the editting ourselves
    ev.Veto();

    if ( ev.GetLabel().empty() )
        return;

    GameData::gdUniqueResourceID resid;
    if ( mainDatabase_->GetResourceIDByCRC( itemData->GetResourceCRC32(), &resid ) != gdERROR_OK )
    {
        wxMessageDialog( this, "Couldn't find item in database" ).ShowModal();
        return;
    }

    pUint32 newCRC;
    wxString name = ev.GetLabel();
    name.MakeUpper();
    SetItemText( item, name );
    if ( mainDatabase_->RenameResource( resid, name.c_str(), &newCRC ) != gdERROR_OK )
    {
        wxMessageDialog( this, "Couldn't rename item due to Name CRC Conflict" ).ShowModal();
        return;
    }

    //Set the item data to match the new name
    itemData->SetResourceCRC32( newCRC );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnFileDropEvent( wxDropFilesEvent& ev )
{    
    wxWindowDisabler disableAll;

    pUint32 fileCount = ev.GetNumberOfFiles();
    wxString* fileNames = ev.GetFiles();
    int flags;
    wxTreeItemId sels = HitTest( ev.GetPosition(), flags );
    wxTreeItemId parentItem = GetRootItem();

    if ( sels.IsOk() )
    {
        DataTreeItem* data = static_cast< DataTreeItem* >( GetItemData( sels ) );
        if ( data && data->GetIsDirectory() )
        {
            parentItem = sels;
        }
        else if ( sels != parentItem )
        {
            parentItem = GetItemParent( sels );
        }

    }

    for ( pUint32 i = 0; i < fileCount; ++i )
    {
        AddResourceItem( fileNames[i], parentItem );
    }
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnToolTipRequest( wxTreeEvent& ev )
{
    DataTreeItem* itemData = static_cast< DataTreeItem* >( GetItemData( ev.GetItem() ) );
    if ( itemData && !itemData->GetIsDirectory() )
    {
        wxString tt;
        GameData::gdUniqueResourceID resid;
        mainDatabase_->GetResourceIDByCRC( itemData->GetResourceCRC32(), &resid );
        tt.Printf( "Resource %s\nCRC: %u(0x%08X)", 
            resid.GetResourceName(), itemData->GetResourceCRC32(), itemData->GetResourceCRC32() );
        ev.SetToolTip( tt );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnDeleteTreeItemEvent( wxTreeEvent& ev )
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnItemSelected( wxTreeEvent& ev )
{
    //assume we will fail the selection test
    selectedRes_ = NULL;

    DataTreeItem* itemData = static_cast< DataTreeItem* >( GetItemData( ev.GetItem() ) );
    if ( itemData && !itemData->GetIsDirectory() )
    {
        GameData::gdResourceInfo* res;
        if ( mainDatabase_->GetResourceByCRC( itemData->GetResourceCRC32(), &res ) == gdERROR_OK )  
        {
            selectedRes_ = res;
            UpdatePropertiesView( res );
        }
    }
    else
    {
        propertiesPane_->Clear();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnContextMenuDelete( wxCommandEvent& ev )
{
    DataTreeItem* itemData = static_cast< DataTreeItem* >( GetItemData( selectedItem_ ) );
    wxString msgStr;
    msgStr.Printf( "Are you sure you want to delete this %s?", 
        itemData->GetIsDirectory() ? "directory & its resources" : "resource" );
    wxMessageDialog msg( this, msgStr, wxMessageBoxCaptionStr, wxYES_NO|wxCENTER );

    if ( msg.ShowModal() == wxID_NO )
    {
        return;
    }

    //Else remove the resource (& any child resources)
    DeleteResourceFromDatabase( selectedItem_ );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnContextMenuAddFolder( wxCommandEvent& ev )
{
    wxTextEntryDialog textEntry( this, "Enter Folder Name" );
    if ( textEntry.ShowModal() == wxID_OK )
    {
        wxTreeItemId newItemParent = selectedItem_;
        //TODO: more validate name checks
        wxString name = textEntry.GetValue();
        if ( !name.EndsWith( "/" ) )
            name.Append( '/' );
        name.MakeUpper();
        // Check selected item is folder, if not add to the parent of the 
        // selected item
        DataTreeItem* item = static_cast< DataTreeItem* >( GetItemData( selectedItem_ ) );
        pAssert( item );

        if ( !item->GetIsDirectory() )
        {
            newItemParent = GetItemParent( newItemParent );
            pAssert( newItemParent.IsOk() );
        }

        DataTreeItem* newItemData = new DataTreeItem;
        newItemData->SetIsDirectory( true );
        wxTreeItemId newItem = AppendItem( newItemParent, name, -1, -1, newItemData );
        SetItemBold( newItem );

    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnContextMenuBuild( wxCommandEvent& ev )
{
    DataTreeItem* itemData = static_cast< DataTreeItem* >( GetItemData( selectedItem_ ) );

    GameData::gdUniqueResourceID uri;
    gdError er = mainDatabase_->GetResourceIDByCRC( itemData->GetResourceCRC32(), &uri );
    if ( er != gdERROR_OK )
    {
        wxMessageDialog( this, "Failed to find resource in database" ).ShowModal();
        return;
    }

    er = mainDatabase_->BuildSingleResource( uri );

    if ( er != gdERROR_OK )
    {
        wxRichMessageDialog msgDlg(this,"Build Failed.","Failed.",wxOK|wxCENTER);
        wxString msg;
        msg += mainDatabase_->GetErrorMessages();
        msgDlg.ShowDetailedText( msg );
        msgDlg.ShowModal();

    }

    mainDatabase_->ClearBuildMessages();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnKeyDownEvent( wxTreeEvent& evt )
{
    int key = evt.GetKeyCode();

    if ( key == WXK_DELETE || key == WXK_BACK )
    {
        wxMessageDialog msg( this, "Are you sure you want to delete this items?", wxMessageBoxCaptionStr, wxYES_NO|wxCENTER );

        if ( msg.ShowModal() == wxID_NO )
        {
            return;
        }

        wxArrayTreeItemIds sels;
        size_t nsel = GetSelections( sels );
        for ( size_t i = 0; i < nsel; ++i )
        {
            // Its possible a delete action earlier in the 
            // list removed a item, so protect ourselves
            if ( sels[i].IsOk() )
            {
                DeleteResourceFromDatabase( sels[i] );
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnPropertyGridChangingEvent( wxPropertyGridEvent& evt )
{
    //if we don't process call skip and allow others to process
    evt.Skip();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::OnPropertyGridChangedEvent( wxPropertyGridEvent& evt )
{
    if (!selectedRes_)
    {
        evt.Skip();
        return;
    }

    if ( evt.GetPropertyName() == UIPID_RESTYPENAME )
    {
        // Returns the enum value as a string
        //evt.GetPropertyValue().GetString()
        const pChar* name = mainDatabase_->GetResourceTypeNameByIndex( evt.GetPropertyValue().GetInteger() );
        gdError err = selectedRes_->SetResourceType( name, mainDatabase_ );
        if ( err != gdERROR_OK )
            evt.Veto( true );
        else
            UpdatePropertiesView( selectedRes_ );
    }
    else if ( evt.GetPropertyName() == UIPID_RESINPUTFILE )
    {
        selectedRes_->SetInputFilePath( evt.GetPropertyValue().GetString().c_str() );
    }
    else
    {
        //Check if its another of our parameters
        GameData::gdParametersArrayType& params = *selectedRes_->GetResourceParameterArray();
        pUint32 paramsCount = params.size();
        for ( pUint32 i = 0; i < paramsCount; ++i )
        {
            const pChar* paramName = params[i].GetName();
            GameData::gdParameterValue& val = *params[i].GetValue();

            if ( paramName != evt.GetPropertyName() )
                continue;

            if ( val.IsBool() )
            {
                //handle bool params
                val.Set( evt.GetPropertyValue().GetBool() );
            }
            else if ( val.IsInt() )
            {
                //handle number params
                val.Set( evt.GetPropertyValue().GetInteger() );
            }
            else if ( val.IsFloat() )
            {
                //handle float values
                val.Set( (float)evt.GetPropertyValue().GetReal() );
            }
            else if ( val.IsEnum() )
            {
                //handle enum values
                val.SetEnumByValue( evt.GetPropertyValue().GetInteger() );
            }
            else if ( val.IsString() )
            {
                //handle string values
                val.Set( evt.GetPropertyValue().GetString().utf8_str() );
            }
            else if ( val.IsFilepath() )
            {
                val.SetFile( evt.GetPropertyValue().GetString() ); 
            }
        }

        //if we don't process call skip and allow others to process
        evt.Skip();
        return;
    }
    //UpdatePropertiesView()
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::AddResourceItem( const wxString& inputFile, const wxTreeItemId& parentItem )
{
    pAssert( mainDatabase_ );

    wxString resName = inputFile.AfterLast( '/' );
    if ( resName == inputFile )
        resName = inputFile.AfterLast( '\\' );
    wxString resPath;
    BuildFullFilePath( parentItem, resPath );
    
    //TODO: Add resource and path to the gamedata database
    GameData::gdUniqueResourceID resid;
    if ( mainDatabase_->AddResource( inputFile, resPath, resName, &resid ) == gdERROR_OK )
    {
        DataTreeItem* itemData = new DataTreeItem;
        itemData->SetResourceCRC32( resid.GetResourceCRCID() );

        //Add to the tree view
        AppendItem( parentItem, resName, -1, -1, itemData );
    }
    else
    {
        wxString msg;
        msg.Printf( "Couldn't add resource %s because name is already in use", inputFile );
        wxMessageDialog( this, msg ).ShowModal();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::AddResourceItem( const GameData::gdUniqueResourceID& resID )
{
    wxTreeItemId tid = GetTreeItemPathNode( resID.GetResourcePath() );

    DataTreeItem* itemData = new DataTreeItem;
    itemData->SetResourceCRC32( resID.GetResourceCRCID() );

    //Add to the tree view
    AppendItem( tid, resID.GetResourceName(), -1, -1, itemData );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::DeleteResourceFromDatabase( const wxTreeItemId& item )
{
    DataTreeItem* itemData = static_cast< DataTreeItem* >( GetItemData( item ) );
    if ( itemData->GetIsDirectory() )
    {
        wxTreeItemIdValue cookie;
        for ( wxTreeItemId i = GetFirstChild( item, cookie ); i.IsOk(); i = GetFirstChild( item, cookie ) )
        {
            DeleteResourceFromDatabase( i );
        }
    }
    else
    {
        GameData::gdUniqueResourceID uri;
        gdError er = mainDatabase_->GetResourceIDByCRC( itemData->GetResourceCRC32(), &uri );
        if ( er != gdERROR_OK )
        {
            wxMessageDialog( this, "Failed to find resource in database" ).ShowModal();
            return;
        }

        er = mainDatabase_->RemoveResource( uri );

        if ( er != gdERROR_OK )
        {
            wxMessageDialog( this, "Failed to remove resource in database" ).ShowModal();
            return;
        }
    }

    Delete( item );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::BuildFullFilePath( const wxTreeItemId& lastItem, wxString& builtPath )
{
    wxTreeItemId id = lastItem;
    wxTreeItemId root = GetRootItem();

    for ( ; id != GetRootItem() && id.IsOk(); id = GetItemParent( id ) )
    {
        builtPath.Prepend( GetItemText( id ) );
    }

    builtPath.MakeUpper();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

wxTreeItemId DataLayoutPane::GetTreeItemPathNode( const gdChar* folderpath )
{
    wxTreeItemId ret = GetRootItem();

    while ( *folderpath )
    {
        const gdChar* end = strpbrk( folderpath, "\\/" );
        // should never be null because folder paths ALWAYS end in a \ or /
        pAssert( end );
        ++end;
        gdString nodename;
        nodename.assign( folderpath, (size_t)(end-folderpath) );

        bool found = false;
        wxTreeItemIdValue cookie;
        for ( wxTreeItemId i = GetFirstChild( ret, cookie ); i.IsOk(); i = GetNextChild( ret, cookie ) )
        {
            if ( GetItemText( i ) == nodename )
            {
                ret = i;
                found = true;
                break;
            }
        }

        if ( !found )
        {
            DataTreeItem* itemData = new DataTreeItem;
            itemData->SetIsDirectory( true );

            //Add to the tree view
            ret = AppendItem( ret, nodename, -1, -1, itemData );
            SetItemBold( ret );
        }

        folderpath = end;
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::RebuildTreeViewFromDatabase()
{
    DeleteAllItems();    

    DataTreeItem* rootdata = new DataTreeItem;
    rootdata->SetIsDirectory( true );
    AddRoot( GAMEDATA_ROOT_NODE_NAME, -1, -1, rootdata );
    SetItemBold( GetRootItem() );

    if ( !mainDatabase_ )
        return;

    GameData::gdResourceIDArray resids;
    pUint32 size = mainDatabase_->GetResourceIDArray( &resids );

    for ( pUint32 i = 0; i < size; ++i )
    {
        AddResourceItem( resids[i] );
    }

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::UpdatePropertiesView( const GameData::gdResourceInfo* res )
{
    propertiesPane_->Clear();
    propertiesPane_->Append( new wxPropertyCategory( UIPLABEL_RESPARAMS ) );

    wxArrayString resnames;
    wxArrayInt resID;
    GameData::gdStringArray dbResNames;
    const pChar* typeName = res->GetResourceTypeName();
    pUint32 count = mainDatabase_->GetAvailableResoruceTypes( &dbResNames );
    pUint32 resValue = count;
    
    resnames.reserve( count + 1 );
    resID.reserve( count + 1 );
    for ( pUint32 i = 0; i < count; ++i ) 
    {
        if ( typeName && strcmp( typeName, dbResNames[i].c_str() ) == 0 )
        {
            resValue = i;
        }
        resnames.push_back( dbResNames[i] );
        resID.push_back( i );
    }
    if ( resValue == count )
    {
        resnames.push_back( RESOURCE_NONE_TYPE_NAME );
        resID.push_back( count );
    }

    propertiesPane_->Append( 
        new wxEnumProperty( 
            UIPLABEL_RESTYPENAME, 
            UIPID_RESTYPENAME, 
            resnames, 
            resID, 
            resValue ) );

    propertiesPane_->Append(
        new wxFileProperty( 
            UIPLABEL_RESINPUTFILE,
            UIPID_RESINPUTFILE,
            res->GetInputFilePath() ) );

    const GameData::gdParametersArrayType& params = *res->GetResourceParameterArray();
    pUint32 paramsCount = params.size();
    for ( pUint32 i = 0; i < paramsCount; ++i )
    {
        AppendResourceParameterToPropertyGrid( params, i );
    }

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DataLayoutPane::AppendResourceParameterToPropertyGrid( const GameData::gdParametersArrayType& params, pUint32 i )
{
    const pChar* paramName = params[i].GetName();
    const GameData::gdParameterValue& val = *params[i].GetValue();

    if ( val.IsBool() )
    {
        //handle bool params
        propertiesPane_->Append(
            new wxBoolProperty( 
            paramName,
            wxPG_LABEL,
            val.GetAsBool() ) );
        propertiesPane_->SetPropertyAttribute( paramName, wxPG_BOOL_USE_CHECKBOX, true );
    }
    else if ( val.IsInt() )
    {
        //handle number params
        propertiesPane_->Append(
            new wxIntProperty(
            paramName,
            wxPG_LABEL,
            val.GetAsInt() ) );
    }
    else if ( val.IsFloat() )
    {
        //handle float values
        propertiesPane_->Append(
            new wxFloatProperty( 
            paramName,
            wxPG_LABEL,
            val.GetAsFloat() ) );
    }
    else if ( val.IsEnum() )
    {
        //handle enum values
        wxArrayString enames;
        wxArrayInt eval;
        for ( pUint32 i2 = 0; i2 < val.GetEnumValueCount(); ++i2 )
        {
            enames.push_back( val.GetEnumValue(i2).name_ );
            eval.push_back( val.GetEnumValue(i2).value_ );
        }
        propertiesPane_->Append(
            new wxEnumProperty( 
            paramName,
            wxPG_LABEL,
            enames,
            eval,
            val.GetAsEnumValue().value_ ) );
    }
    else if ( val.IsString() )
    {
        //handle string values
        propertiesPane_->Append(
            new wxStringProperty( 
            paramName,
            wxPG_LABEL,
            val.GetAsString() ) );
    }
    else if ( val.IsFilepath() )
    {
        propertiesPane_->Append(
            new wxFileProperty( 
                paramName, 
                wxPG_LABEL, 
                val.GetAsFilepath() ) );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

EVT_DEFINE_EVENT( db_loaded );

void DataLayoutPane::EventRecieve( const Event& evt )
{
    if ( evt == EVT_EVENT( db_loaded ) )
    {
        mainDatabase_ = (GameData::gdGameDatabaseObject*)evt.GetEventData();
        RebuildTreeViewFromDatabase();
    }
}