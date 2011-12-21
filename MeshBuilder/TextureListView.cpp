/********************************************************************

	filename: 	TextureListView.cpp	
	
	Copyright (c) 20:2:2011 James Moran
	
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
#include "TextureListView.h"
#include "SceneDatabase.h"
#include "wx\propgrid\propgrid.h"
#include "wx\propgrid\advprops.h"
#include "SceneGraph.h"

namespace UI
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	const char* TextureListView::FILE_PROP_NAME = "File";
	const char* TextureListView::FORMAT_PROP_NAME = "Format";

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	TextureListView::TextureListView( 
		HScene::SceneGraph* pScene,
		wxWindow* parent, 
		wxWindowID id, 
		wxPoint position /*= wxDefaultPosition*/, 
		wxSize size /*= wxDefaultSize */ ) :
			wxListCtrl( parent, id, position, size, wxLC_LIST | wxLC_SINGLE_SEL, wxDefaultValidator, "TextureListCtrl" )
			,pSceneGraph_( pScene )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	TextureListView::~TextureListView()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void TextureListView::UpdatePropertyGrid( wxListEvent& evt, wxPropertyGrid* pPropertyGrid )
	{
		const wxListItem& item = evt.GetItem();
		hCurrentTexture_ = (u32)item.GetData();

		pPropertyGrid->Clear();
		//TODO: Make these names const, etc...
		//Material Properties
		pPropertyGrid->Append( new wxPropertyCategory( "Texture Properties" ) );

		//
		pPropertyGrid->Append( new wxPropertyCategory( "General" ) );
		pPropertyGrid->Append( new wxFileProperty( FILE_PROP_NAME, wxPG_LABEL, pSceneGraph_->GetTexture( hCurrentTexture_ )->filename_.c_str() ) );
		pPropertyGrid->SetPropertyAttribute( FILE_PROP_NAME, wxPG_FILE_INITIAL_PATH, pSceneGraph_->GetTexture( hCurrentTexture_ )->filename_.c_str() );
		pPropertyGrid->SetPropertyAttribute( FILE_PROP_NAME, wxPG_FILE_WILDCARD, "PNG files (*.png)|*.png" );
		static const wxChar* enumTextureFormat[] = 
		{
			wxT("8 Bit RGBA (32 Bit)"),
			wxT("8 Bit RGB (24 Bit)"),
			wxT("DXT Compressed"),
			NULL
		};
		pPropertyGrid->Append( new wxEnumProperty( FORMAT_PROP_NAME, wxPG_LABEL, enumTextureFormat, NULL, pSceneGraph_->GetTexture( hCurrentTexture_ )->format_ ) );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void TextureListView::UpdateTextureView()
	{
		ClearAll();
		for ( u32 i = 0; i < pSceneGraph_->GetTextureCount(); ++i )
		{
			wxListItem item;
			item.SetData( i );
			item.SetText( pSceneGraph_->GetTexture( i )->filename_.c_str() );
			InsertItem( item );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void TextureListView::OnPropertyGridCellChanging( wxPropertyGridEvent& evt )
	{
		if ( evt.GetPropertyName().Cmp( FILE_PROP_NAME ) == 0 )
		{
			if ( !evt.GetPropertyValue().GetString().EndsWith( ".png" ) && 
				 !evt.GetPropertyValue().GetString().EndsWith( ".PNG" ) )
			{
				evt.Veto();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool TextureListView::OnPropertyGridCellChanged( wxPropertyGridEvent& evt )
	{
		if ( evt.GetPropertyName().Cmp( FILE_PROP_NAME ) == 0 )
		{
			pSceneGraph_->GetTexture( hCurrentTexture_ )->filename_ = evt.GetPropertyValue().GetString().c_str();
		}
		else if ( evt.GetPropertyName().Cmp( FORMAT_PROP_NAME ) == 0 )
		{
			pSceneGraph_->GetTexture( hCurrentTexture_ )->format_ = (Data::TextureFormat)evt.GetPropertyValue().GetInteger();
		}

		return true;
	}

}