/********************************************************************

	filename: 	MaterialListView.cpp	
	
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
#include "MaterialListView.h"
#include "DataMaterial.h"
#include "wx\propgrid\propgrid.h"
#include "wx\propgrid\advprops.h"
#include "SceneGraph.h"



namespace UI
{
namespace 
{
	struct TextureInfo
	{
		wxArrayString			filenames_;
		wxArrayInt 				hTex_;
	};
}

	const char* MaterialListView::NAME_PROPNAME			= "Name";
	const char* MaterialListView::TWOSIDE_PROPNAME		= "Two Sided";
	const char* MaterialListView::WIREFRAME_PROPNAME	= "Wireframe";
	const char* MaterialListView::DEPTHTEST_PROPNAME	= "Depth Test";
	const char* MaterialListView::DEPTHWRITE_PROPNAME	= "Depth Write";
	const char* MaterialListView::ZTESTOP_PROPNAME		= "Z Test Op";
	const char* MaterialListView::BLENDFUNC_PROPNAME	= "Blend Function";
	const char* MaterialListView::SRCBLENDOP_PROPNAME	= "Source Blend Op";
	const char* MaterialListView::DSTBLENDOP_PROPNAME	= "Dest Blend Op";
	const char* MaterialListView::DIFFUSE_PROPNAME		= "Diffuse";
	const char* MaterialListView::AMBIENT_PROPNAME		= "Ambient";
	const char* MaterialListView::SPECULAR_PROPNAME		= "Specular";
	const char* MaterialListView::EMISSIVE_PROPNAME		= "Emissive";
	const char* MaterialListView::HLSLCODE_PROPNAME		= "HLSL Code";
	const char* MaterialListView::DIFFUSETEX_PROPNAME	= "Diffuse Texture";
	const char* MaterialListView::NORMALTEX_PROPNAME	= "Normal Map";
	const char* MaterialListView::SPECULARMAP_PROPNAME	= "Specular Map";
	const char* MaterialListView::LIGHTMAP_PROPNAME		= "Light Map";

	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	MaterialListView::MaterialListView( 
		HScene::SceneGraph* pScene,
		wxWindow* parent, 
		wxWindowID id, 
		wxPropertyGrid* pPropertyGrid,
		wxPoint position /*= wxDefaultPosition*/, 
		wxSize size /*= wxDefaultSize*/ ) :
			wxListCtrl( parent, id, position, size, wxLC_LIST | wxLC_SINGLE_SEL, wxDefaultValidator, "MaterialListCtrl" )
			,pPropertyGrid_( pPropertyGrid )
			,pCurrentMat_( NULL )
			,pScene_( pScene )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	MaterialListView::~MaterialListView()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MaterialListView::UpdatePropertyGrid( wxListEvent& evt, wxPropertyGrid* pPropertyGrid )
	{
		const wxListItem& item = evt.GetItem();
		Data::Material* pMat = (Data::Material*)item.GetData();
		pCurrentMat_ = pMat;

		pPropertyGrid->Clear();
		//TODO: Make these names const, etc...
		//Material Properties
		pPropertyGrid->Append( new wxPropertyCategory( "Material Properties" ) );
		
		//
		pPropertyGrid->Append( new wxPropertyCategory( "Parameters" ) );
		pPropertyGrid->Append( new wxStringProperty( NAME_PROPNAME, wxPG_LABEL, pMat->GetName() ) );
		pPropertyGrid->Append( new wxBoolProperty( TWOSIDE_PROPNAME, wxPG_LABEL, pMat->GetTwoSided() ) );
		pPropertyGrid->Append( new wxBoolProperty( WIREFRAME_PROPNAME, wxPG_LABEL, pMat->GetWireframe() ) );
		pPropertyGrid->Append( new wxBoolProperty( DEPTHTEST_PROPNAME, wxPG_LABEL, pMat->GetZDepthTest() ) );
		pPropertyGrid->Append( new wxBoolProperty( DEPTHWRITE_PROPNAME, wxPG_LABEL, pMat->GetZWrite() ) );
		static const wxChar* enumZTestState[] = 
		{
			wxT("Never"),						//MAT_Z_CMP_NEVER,
			wxT("Less Than"),					//MAT_Z_CMP_LESS,
			wxT("Equal To"),					//MAT_Z_CMP_EQUAL,
			wxT("Less Than or Equal To"),		//MAT_Z_CMP_LESSEQUAL,
			wxT("Greater Than"),				//MAT_Z_CMP_GREATER,
			wxT("Not Equal To"),				//MAT_Z_CMP_NOT_EQUAL,
			wxT("Greater Than or Equal To"),	//MAT_Z_CMP_GREATER_EQUAL,
			wxT("Always"),						//MAT_Z_CMP_ALWAYS,
			NULL
		};
		pPropertyGrid->Append( new wxEnumProperty( ZTESTOP_PROPNAME, wxPG_LABEL, enumZTestState, NULL, pMat->GetZTestState() ) );
		static const wxChar* enumBlendFuntion[] = 
		{
			wxT("None"),			//MAT_BLEND_FUNC_NONE,
			wxT("Blend ADD"),		//MAT_BLEND_FUNC_ADD,
			wxT("Blend SUB"),		//MAT_BLEND_FUNC_SUB,
			wxT("Blend MIN"),		//MAT_BLEND_FUNC_MIN,
			wxT("Blend MAX"),		//MAT_BLEND_FUNC_MAX,
			NULL
		};
		pPropertyGrid->Append( new wxEnumProperty( BLENDFUNC_PROPNAME, wxPG_LABEL, enumBlendFuntion, NULL, pMat->GetBlendFunction() ) );
		static const wxChar* enumBlendOP[] = 
		{
			wxT("Zero"),					//MAT_BLEND_OP_ZERO,
			wxT("One"),						//MAT_BLEND_OP_ONE,
			wxT("Source Colour"),			//MAT_BLEND_OP_SRC_COLOUR,
			wxT("One Minus Source Colour"),	//MAT_BLEND_OP_INVSRC_COLOUR,
			wxT("Dest Colour"),				//MAT_BLEND_OP_DEST_COLOUR,
			wxT("One Minus Dest Colour"),	//MAT_BLEND_OP_INVDEST_COLOUR,
			wxT("Source Alpha"),			//MAT_BLEND_OP_SRC_ALPHA,
			wxT("One Minus Source Alpha"),	//MAT_BLEND_OP_INVSRC_ALPHA,
			wxT("Dest Alpha"),				//MAT_BLEND_OP_DEST_ALPHA,
			wxT("One Minus Dest Alpha"),	//MAT_BLEND_OP_INVDEST_ALPHA,
			NULL
		};
		pPropertyGrid->Append( new wxEnumProperty( SRCBLENDOP_PROPNAME, wxPG_LABEL, enumBlendOP, NULL, pMat->GetBlendOPSrc() ) );
		pPropertyGrid->Append( new wxEnumProperty( DSTBLENDOP_PROPNAME, wxPG_LABEL, enumBlendOP, NULL, pMat->GetBlendOPDst() ) );
		
		pPropertyGrid->Append( new wxPropertyCategory( "Colour" ) );
		wxColour diffuse( pMat->GetDiffuse().r*255, pMat->GetDiffuse().g*255, pMat->GetDiffuse().b*255, pMat->GetDiffuse().a*255 );
		pPropertyGrid->Append( new wxColourProperty( DIFFUSE_PROPNAME, wxPG_LABEL, diffuse ) );
		wxColour ambient( pMat->GetAmbient().r*255, pMat->GetAmbient().g*255, pMat->GetAmbient().b*255, pMat->GetAmbient().a*255 );
		pPropertyGrid->Append( new wxColourProperty( AMBIENT_PROPNAME, wxPG_LABEL, ambient ) );
		wxColour specular( pMat->GetSpecular().r*255, pMat->GetSpecular().g*255, pMat->GetSpecular().b*255, pMat->GetSpecular().a*255 );
		pPropertyGrid->Append( new wxColourProperty( SPECULAR_PROPNAME, wxPG_LABEL, specular ) );
		wxColour emissive( pMat->GetEmissive().r*255, pMat->GetEmissive().g*255, pMat->GetEmissive().b*255, pMat->GetEmissive().a*255 );
		pPropertyGrid->Append( new wxColourProperty( EMISSIVE_PROPNAME, wxPG_LABEL, emissive ) );

		pPropertyGrid->Append( new wxPropertyCategory( "Shader Info" ) );
		pPropertyGrid->Append( new wxLongStringProperty( HLSLCODE_PROPNAME, wxPG_LABEL, pMat->GetShaderCode() ) );

		pPropertyGrid->Append( new wxPropertyCategory( "Texture Samplers" ) );
		TextureInfo pinfolist;
		for ( u32 i = 0; i < pScene_->GetTextureCount(); ++i )
		{
			pinfolist.filenames_.Add( pScene_->GetTexture( i )->filename_.c_str() );
			pinfolist.hTex_.Add( i );
		}
		pinfolist.filenames_.Add( "UNDEFINED" );
		pinfolist.hTex_.Add( Data::INVALIDTEXTUREHANDLE );
		for ( u32 i = 0; i < Data::Material::MAX_DIFFUSE_TEXTURES; ++i )
		{
			wxString propname;
			propname.Printf( "%s %d", DIFFUSETEX_PROPNAME, i+1 );
			pPropertyGrid->Append( new wxEnumProperty( propname, wxPG_LABEL, pinfolist.filenames_, pinfolist.hTex_, pMat->GetDiffuseTextureHandle( i ) ) );
		}
		pPropertyGrid->Append( new wxEnumProperty( NORMALTEX_PROPNAME, wxPG_LABEL, pinfolist.filenames_, pinfolist.hTex_, pMat->GetNormalTextureHandle() ) );
		pPropertyGrid->Append( new wxEnumProperty( SPECULARMAP_PROPNAME, wxPG_LABEL, pinfolist.filenames_, pinfolist.hTex_, pMat->GetSpecularTextureHandle() ) );
		pPropertyGrid->Append( new wxEnumProperty( LIGHTMAP_PROPNAME, wxPG_LABEL, pinfolist.filenames_, pinfolist.hTex_, pMat->GetLightMapTextureHandle() ) );

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MaterialListView::UpdateMaterialView()
	{
		ClearAll();
		for ( u32 i = 0; i < pScene_->GetMaterialCount(); ++i )
		{
			Data::Material* pMat = pScene_->GetMaterial( i );
			wxListItem item;
			item.SetData( pMat );
			item.SetText( pMat->GetName() );
			InsertItem( item );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MaterialListView::OnPropertyGridCellChanging( wxPropertyGridEvent& evt )
	{
		//Don't need to veto/validate most things currently, done by UI
		if ( evt.GetPropertyName().Cmp( NAME_PROPNAME ) == 0 )
		{
			if ( evt.GetPropertyValue().GetString().IsEmpty() )
			{
				evt.Veto( true );
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool MaterialListView::OnPropertyGridCellChanged( wxPropertyGridEvent& evt )
	{
		bool redraw = false;
		wxString postfix;

		if ( evt.GetPropertyName().Cmp( NAME_PROPNAME ) == 0 )
		{
			pCurrentMat_->SetName( evt.GetPropertyValue().GetString().c_str() );
		}
		else if ( evt.GetPropertyName().Cmp( TWOSIDE_PROPNAME ) == 0 )
		{
			pCurrentMat_->SetTwoSided( evt.GetPropertyValue().GetBool() );
		}
		else if ( evt.GetPropertyName().Cmp( HLSLCODE_PROPNAME ) == 0 )
		{
			pCurrentMat_->SetShaderCode( evt.GetPropertyValue().GetString().c_str() );
		}
		else if ( evt.GetPropertyName().Cmp( WIREFRAME_PROPNAME ) == 0 )
		{
			pCurrentMat_->SetWireframe( evt.GetPropertyValue().GetBool() );
		}
		else if ( evt.GetPropertyName().Cmp( DEPTHTEST_PROPNAME ) == 0 )
		{
			pCurrentMat_->SetZDepthTest( evt.GetPropertyValue().GetBool() );
		}
		else if ( evt.GetPropertyName().Cmp( DEPTHWRITE_PROPNAME ) == 0 )
		{
			pCurrentMat_->SetZWrite( evt.GetPropertyValue().GetBool() );
		}
		else if ( evt.GetPropertyName().Cmp( ZTESTOP_PROPNAME ) == 0 )
		{
			pCurrentMat_->SetZTestState( (Data::ZTestState)evt.GetPropertyValue().GetInteger() );
		}
		else if ( evt.GetPropertyName().Cmp( BLENDFUNC_PROPNAME ) == 0 )
		{
			pCurrentMat_->SetBlendFunction( (Data::BlendFunction)evt.GetPropertyValue().GetInteger() );
		}
		else if ( evt.GetPropertyName().Cmp( SRCBLENDOP_PROPNAME ) == 0 )
		{
			pCurrentMat_->SetBlendOPSrc( (Data::BlendOP)evt.GetPropertyValue().GetInteger() );
		}
		else if ( evt.GetPropertyName().Cmp( DSTBLENDOP_PROPNAME ) == 0 )
		{
			pCurrentMat_->SetBlendOPDst( (Data::BlendOP)evt.GetPropertyValue().GetInteger() );
		}
		else if ( evt.GetPropertyName().Cmp( DIFFUSE_PROPNAME ) == 0 )
		{
			wxAny v = evt.GetProperty()->GetValue();
			wxColour c = v.As< wxColour >();
			Data::Colour cc;
			cc.r = c.Red()*255.0f;
			cc.b = c.Green()*255.0f;
			cc.g = c.Blue()*255.0f;
			cc.a = 1.0f;
			pCurrentMat_->SetDiffuse( cc );
		}
		else if ( evt.GetPropertyName().Cmp( AMBIENT_PROPNAME ) == 0 )
		{
			wxAny v = evt.GetProperty()->GetValue();
			wxColour c = v.As< wxColour >();
			Data::Colour cc;
			cc.r = c.Red()*255.0f;
			cc.b = c.Green()*255.0f;
			cc.g = c.Blue()*255.0f;
			cc.a = 1.0f;
			pCurrentMat_->SetAmbient( cc );
		}
		else if ( evt.GetPropertyName().Cmp( SPECULAR_PROPNAME ) == 0 )
		{
			wxAny v = evt.GetProperty()->GetValue();
			wxColour c = v.As< wxColour >();
			Data::Colour cc;
			cc.r = c.Red()*255.0f;
			cc.b = c.Green()*255.0f;
			cc.g = c.Blue()*255.0f;
			cc.a = 1.0f;
			pCurrentMat_->SetSpecular( cc );
		}
		else if ( evt.GetPropertyName().Cmp( EMISSIVE_PROPNAME ) == 0 )
		{
			wxAny v = evt.GetProperty()->GetValue();
			wxColour c = v.As< wxColour >();
			Data::Colour cc;
			cc.r = c.Red()*255.0f;
			cc.b = c.Green()*255.0f;
			cc.g = c.Blue()*255.0f;
			cc.a = 1.0f;
			pCurrentMat_->SetEmissive( cc );
		}
		else if ( evt.GetPropertyName().StartsWith( DIFFUSETEX_PROPNAME, &postfix ) )
		{
			unsigned long idx;
			if ( postfix.ToULong( &idx ) )
			{
				pCurrentMat_->SetDiffuseTextureHandle( evt.GetPropertyValue().GetInteger(), idx-1 );
			}
		}
		else if ( evt.GetPropertyName().Cmp( NORMALTEX_PROPNAME ) == 0 )
		{
			pCurrentMat_->SetNormalTextureHandle( evt.GetPropertyValue().GetInteger() );
		}
		else if ( evt.GetPropertyName().Cmp( SPECULARMAP_PROPNAME ) == 0 )
		{
			pCurrentMat_->SetSpecularTextureHandle( evt.GetPropertyValue().GetInteger() );
		}
		else if ( evt.GetPropertyName().Cmp( LIGHTMAP_PROPNAME ) == 0 )
		{
			pCurrentMat_->SetSpecularTextureHandle( evt.GetPropertyValue().GetInteger() );
		}

		UpdateMaterialView();
		return redraw;
	}

}
