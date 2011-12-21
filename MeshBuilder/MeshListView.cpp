/********************************************************************

	filename: 	MeshListView.cpp	
	
	Copyright (c) 10:5:2011 James Moran
	
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
#include "MeshListView.h"
#include "SceneDatabase.h"
#include "wx\propgrid\propgrid.h"
#include "wx\propgrid\advprops.h"
#include "SceneGraph.h"

namespace UI
{
namespace 
{
	struct MaterialInfo
	{
		wxArrayString	name_;
		wxArrayInt		idx_;
	};
}
	const char* MeshListView::MATERIAL_NAME			= "Material";

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	MeshListView::MeshListView( 
		HScene::SceneGraph* pScene,
		wxWindow* parent, 
		wxWindowID id, 
		wxPoint position /*= wxDefaultPosition*/, 
		wxSize size /*= wxDefaultSize */ ) :
		wxListCtrl( parent, id, position, size, wxLC_LIST | wxLC_SINGLE_SEL, wxDefaultValidator, "TextureListCtrl" )
		,pScene_( pScene )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	MeshListView::~MeshListView()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MeshListView::UpdatePropertyGrid( wxListEvent& evt, wxPropertyGrid* pPropertyGrid )
	{
		const wxListItem& item = evt.GetItem();
		Data::Mesh* mesh = pScene_->GetMesh( item.GetData() );

		pPropertyGrid->Clear();
 		//TODO: Make these names const, etc...
 		//Material Properties
		pPropertyGrid->Append( new wxPropertyCategory( "Mesh Properties" ) );

		pPropertyGrid->Append( new wxPropertyCategory( "General" ) );

		MaterialInfo materials;
		for ( u32 i = 0; i < pScene_->GetMaterialCount(); ++i )
		{
			Data::Material* mat = pScene_->GetMaterial(i);
			materials.name_.Add( mat->GetName() );
			materials.idx_.Add( i );
		}
		pPropertyGrid->Append( new wxEnumProperty( MATERIAL_NAME, wxPG_LABEL, materials.name_, materials.idx_, mesh->GetMeshMaterialIndex() ) );

		pPropertyGrid->Append( new wxBoolProperty( "Has Normals", wxPG_LABEL, mesh->GetnNormals() > 0 ) );
		pPropertyGrid->SetPropertyAttribute( "Has Normals", wxPG_BOOL_USE_CHECKBOX, true );
		pPropertyGrid->SetPropertyBackgroundColour( "Has Normals", *wxLIGHT_GREY );
		pPropertyGrid->SetPropertyReadOnly( "Has Normals", true );

		pPropertyGrid->Append( new wxBoolProperty( "Has Tangents", wxPG_LABEL, mesh->GetnTangents() > 0 ) );
		pPropertyGrid->SetPropertyAttribute( "Has Tangents", wxPG_BOOL_USE_CHECKBOX, true );
		pPropertyGrid->SetPropertyBackgroundColour( "Has Tangents", *wxLIGHT_GREY );
		pPropertyGrid->SetPropertyReadOnly( "Has Tangents", true );

		pPropertyGrid->Append( new wxBoolProperty( "Has Binormals", wxPG_LABEL, mesh->GetnBinormals() > 0 ) );
		pPropertyGrid->SetPropertyAttribute( "Has Binormals", wxPG_BOOL_USE_CHECKBOX, true );
		pPropertyGrid->SetPropertyBackgroundColour( "Has Binormals", *wxLIGHT_GREY );
		pPropertyGrid->SetPropertyReadOnly( "Has Binormals", true );
		
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MeshListView::UpdateMeshView()
	{
		ClearAll();
		for ( u32 i = 0; i < pScene_->GetMeshCount(); ++i )
		{
			wxListItem item;
			item.SetData( i );
			item.SetText( pScene_->GetMesh( i )->GetName() );
			InsertItem( item );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MeshListView::OnPropertyGridCellChanging( wxPropertyGridEvent& evt )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool MeshListView::OnPropertyGridCellChanged( wxPropertyGridEvent& evt )
	{

		return false;
	}
}