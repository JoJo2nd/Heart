/********************************************************************

	filename: 	SceneGraphTreeViewCtrl.cpp
	
	Copyright (c) 2011/02/21 James Moran
	
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
#include "SceneGraphTreeViewCtrl.h"
#include "SceneGraph.h"
#include "SceneNode.h"
#include "SceneDatabase.h"
#include "wx\propgrid\propgrid.h"
#include "wx\propgrid\advprops.h"
#include "SceneNodeLight.h"
#include "SceneNodeCamera.h"
#include "SceneNodeMeshCollection.h"


namespace UI
{


	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	SceneGraphTreeViewCtrl::SceneGraphTreeViewCtrl( HScene::SceneGraph* pScene, wxWindow* parent, wxWindowID id ) :
		wxTreeCtrl( 
			   parent, 
			   id, 
			   wxPoint( 0, 0 ), 
			   wxSize( 200, 480 ), 
			   wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_MULTIPLE )
	   ,pScene_( pScene )
	{

	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	SceneGraphTreeViewCtrl::~SceneGraphTreeViewCtrl()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void SceneGraphTreeViewCtrl::UpdatePropertyGrid( wxTreeEvent& evt, wxPropertyGrid* pPropertyGrid )
	{
		pPropertyGrid->Clear();

		if (!(HScene::NodeTreeHook*)GetItemData(evt.GetItem()))
			return;

		selectedNode_ = ((HScene::NodeTreeHook*)GetItemData(evt.GetItem()))->pHook_;

		if (!selectedNode_)
			return;

		//TODO: Make these names const, etc...
		//Node Properties
		pPropertyGrid->Append( new wxPropertyCategory( "Scene Node Properties" ) );

		pPropertyGrid->Append( new wxPropertyCategory( "General" ) );
		pPropertyGrid->Append( new wxStringProperty( "Name", wxPG_LABEL, selectedNode_->pName() ) );

		D3DXVECTOR3 sca,trans;
		D3DXQUATERNION rot;

		D3DXMatrixDecompose( &sca, &rot, &trans, &selectedNode_->matrix_ );
	
		pPropertyGrid->Append( new wxPropertyCategory( "Position" ) );
		pPropertyGrid->Append( new wxFloatProperty( "x", "position.x", trans.x ) );
		pPropertyGrid->Append( new wxFloatProperty( "y", "position.y", trans.y ) );
		pPropertyGrid->Append( new wxFloatProperty( "z", "position.z", trans.z ) );

		pPropertyGrid->Append( new wxPropertyCategory( "Scale" ) );
		pPropertyGrid->Append( new wxFloatProperty( "x", "scale.x", sca.x ) );
		pPropertyGrid->Append( new wxFloatProperty( "y", "scale.y", sca.y ) );
		pPropertyGrid->Append( new wxFloatProperty( "z", "scale.z", sca.z ) );

		pPropertyGrid->Append( new wxPropertyCategory( "Rotation" ) );
		pPropertyGrid->Append( new wxFloatProperty( "x", "rot.x", rot.x ) );
		pPropertyGrid->Append( new wxFloatProperty( "y", "rot.y", rot.y ) );
		pPropertyGrid->Append( new wxFloatProperty( "z", "rot.z", rot.z ) );
		pPropertyGrid->Append( new wxFloatProperty( "w", "rot.w", rot.w ) );

		pPropertyGrid->Append( new wxPropertyCategory( "Misc." ) );
		pPropertyGrid->Append( new wxBoolProperty( "Casts Shadow", "node.casts", selectedNode_->GetCastShadows() ) );
		pPropertyGrid->SetPropertyAttribute( "node.casts", wxPG_BOOL_USE_CHECKBOX, true );

		//SetPropertyBackgroundColour
		//wxLIGHT_GREY
		pPropertyGrid->Append( new wxPropertyCategory( "AABB" ) );
		pPropertyGrid->Append( new wxFloatProperty( "Centre.x", wxPG_LABEL, selectedNode_->GetAABB()->c.x ) );
		pPropertyGrid->SetPropertyBackgroundColour( "Centre.x", *wxLIGHT_GREY );
		pPropertyGrid->SetPropertyReadOnly( "Centre.x", true );
		pPropertyGrid->Append( new wxFloatProperty( "Centre.y", wxPG_LABEL, selectedNode_->GetAABB()->c.y ) );
		pPropertyGrid->SetPropertyBackgroundColour( "Centre.y", *wxLIGHT_GREY );
		pPropertyGrid->SetPropertyReadOnly( "Centre.y", true );
		pPropertyGrid->Append( new wxFloatProperty( "Centre.z", wxPG_LABEL, selectedNode_->GetAABB()->c.z ) );
		pPropertyGrid->SetPropertyBackgroundColour( "Centre.z", *wxLIGHT_GREY );
		pPropertyGrid->SetPropertyReadOnly( "Centre.z", true );
		pPropertyGrid->Append( new wxFloatProperty( "Radius.x", wxPG_LABEL, selectedNode_->GetAABB()->r[0] ) );
		pPropertyGrid->SetPropertyBackgroundColour( "Radius.x", *wxLIGHT_GREY );
		pPropertyGrid->SetPropertyReadOnly( "Radius.x", true );
		pPropertyGrid->Append( new wxFloatProperty( "Radius.y", wxPG_LABEL, selectedNode_->GetAABB()->r[1] ) );
		pPropertyGrid->SetPropertyBackgroundColour( "Radius.y", *wxLIGHT_GREY );
		pPropertyGrid->SetPropertyReadOnly( "Radius.y", true );
		pPropertyGrid->Append( new wxFloatProperty( "Radius.z", wxPG_LABEL, selectedNode_->GetAABB()->r[2] ) );
		pPropertyGrid->SetPropertyBackgroundColour( "Radius.z", *wxLIGHT_GREY );
		pPropertyGrid->SetPropertyReadOnly( "Radius.z", true );
		

		pPropertyGrid->Append( new wxPropertyCategory( selectedNode_->NodeTypeName() ) );

		if ( strcmp( selectedNode_->NodeTypeName(), HScene::Locator::StaticNodeTypeName ) == 0 )
		{

		}
		else if ( strcmp( selectedNode_->NodeTypeName(), HScene::Camera::StaticNodeTypeName ) == 0 )
		{
			HScene::Camera* cam = (HScene::Camera*)selectedNode_;

			pPropertyGrid->Append( new wxFloatProperty( "Aspect", "cam.aspect", cam->aspect_ ) );
			pPropertyGrid->Append( new wxFloatProperty( "Near Plane", "cam.near", cam->nearPlane_ ) );
			pPropertyGrid->Append( new wxFloatProperty( "Far Plane", "cam.far", cam->farPlane_ ) );
			pPropertyGrid->Append( new wxFloatProperty( "Field of View", "cam.fov", HRadsToDegs( cam->fov_ ) ) );
		}
		else if ( strcmp( selectedNode_->NodeTypeName(), HScene::MeshCollection::StaticNodeTypeName ) == 0 )
		{

		}
		else if ( strcmp( selectedNode_->NodeTypeName(), HScene::Light::StaticNodeTypeName ) == 0 )
		{
			HScene::Light* light = (HScene::Light*)selectedNode_;
			using namespace HScene;
			
			wxArrayString lightTypeName;	
			wxArrayInt 	  lightTypeID;

			lightTypeName.Add( GET_ENUM_STRING( LightType, LightType_Direction ) );
			lightTypeID.Add( LightType_Direction );
			lightTypeName.Add( GET_ENUM_STRING( LightType, LightType_Spot ) );
			lightTypeID.Add( LightType_Spot );
			lightTypeName.Add( GET_ENUM_STRING( LightType, LightType_Point ) );
			lightTypeID.Add( LightType_Point );

			pPropertyGrid->Append( new wxEnumProperty( "Type", wxPG_LABEL, lightTypeName, lightTypeID, light->lightType_ ) );
			pPropertyGrid->AppendIn( HScene::Light::StaticNodeTypeName, new wxFloatProperty( "Light Brightness", "lig.power", light->brightness_ ) );

			switch ( light->lightType_ )
			{
			case LightType_Direction:
				break;
			case LightType_Point:
				pPropertyGrid->AppendIn( HScene::Light::StaticNodeTypeName, new wxFloatProperty( "Inner Radius", "lig.minR", light->minRadius_ ) );
				pPropertyGrid->AppendIn( HScene::Light::StaticNodeTypeName, new wxFloatProperty( "Outer Radius", "lig.maxR", light->maxRadius_ ) );
				break;
			case LightType_Spot:
				pPropertyGrid->AppendIn( HScene::Light::StaticNodeTypeName, new wxFloatProperty( "Inner Cone", "lig.inner", HRadsToDegs( light->innerCone_ ) ) );
				pPropertyGrid->AppendIn( HScene::Light::StaticNodeTypeName, new wxFloatProperty( "Outer Cone", "lig.outer", HRadsToDegs( light->outerCone_ ) ) );
				pPropertyGrid->AppendIn( HScene::Light::StaticNodeTypeName, new wxFloatProperty( "Light Falloff", "lig.falloff", light->falloff_ ) );
				break;
			}

			wxColour diffuse( light->diffuse_.r*255, light->diffuse_.g*255, light->diffuse_.b*255, light->diffuse_.a*255 );
			pPropertyGrid->AppendIn( HScene::Light::StaticNodeTypeName, new wxColourProperty( "Diffuse", "lig.diffuse", diffuse ) );
			wxColour specular( light->specular_.r*255, light->specular_.g*255, light->specular_.b*255, light->specular_.a*255 );
			pPropertyGrid->AppendIn( HScene::Light::StaticNodeTypeName, new wxColourProperty( "Specular", "lig.specular", specular ) );
			wxColour ambient( light->ambient_.r*255, light->ambient_.g*255, light->ambient_.b*255, light->ambient_.a*255 );
			pPropertyGrid->AppendIn( HScene::Light::StaticNodeTypeName, new wxColourProperty( "Ambient", "lig.ambient", ambient ) );
			
		}

		pPropertyGrid->Append( new wxPropertyCategory( "Custom Properties" ) );

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void SceneGraphTreeViewCtrl::OnPropertyGridCellChanging( wxPropertyGridEvent& evt )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool SceneGraphTreeViewCtrl::OnPropertyGridCellChanged( wxPropertyGridEvent& evt )
	{
		bool redraw = false;

		using namespace HScene;
		D3DXVECTOR3 sca,trans;
		D3DXQUATERNION rot;
		D3DXMatrixDecompose( &sca, &rot, &trans, &selectedNode_->matrix_ );


		if ( evt.GetPropertyName().Cmp( "Name" ) == 0 )
		{
			wxString oldname = selectedNode_->pName();
			wxString newname = evt.GetPropertyValue().GetString();
			u32 namecount = CountMatchingNames( pScene_, newname.c_str() );;
			for ( u32 i = 1; namecount != 0; ++i )
			{
				newname.Printf( "%s_%03d", oldname.c_str(), i );
				namecount = CountMatchingNames( pScene_, newname.c_str() );
			}

			selectedNode_->pName( newname.c_str() );

			redraw = false;
		}
		else if ( evt.GetPropertyName().Cmp( "position.x" ) == 0 )
		{
			trans.x = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "position.y" ) == 0 )
		{
			trans.y = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "position.z" ) == 0 )
		{
			trans.z = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "scale.x" ) == 0 )
		{
			sca.x = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "scale.y" ) == 0 )
		{
			sca.y = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "scale.z" ) == 0 )
		{
			sca.z = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "rot.x" ) == 0 )
		{
			rot.x = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "rot.y" ) == 0 )
		{
			rot.y = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "rot.z" ) == 0 )
		{
			rot.z = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "rot.z" ) == 0 )
		{
			rot.w = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "node.casts" ) == 0 )
		{
			selectedNode_->SetCastShadows( evt.GetPropertyValue().GetBool() );
		}
		else if ( evt.GetPropertyName().Cmp( "Type" ) == 0 )
		{
			Light* lig = (Light*)selectedNode_;
			lig->lightType_ = (LightType)evt.GetPropertyValue().GetInteger();
		}
		else if ( evt.GetPropertyName().Cmp( "lig.inner" ) == 0 )
		{
			Light* lig = (Light*)selectedNode_;
			lig->innerCone_ = HDegsToRads( (float)evt.GetPropertyValue().GetDouble() );
		}
		else if ( evt.GetPropertyName().Cmp( "lig.outer" ) == 0 )
		{
			Light* lig = (Light*)selectedNode_;
			lig->outerCone_ = HDegsToRads( (float)evt.GetPropertyValue().GetDouble() );
		}
		else if ( evt.GetPropertyName().Cmp( "lig.minR" ) == 0 )
		{
			Light* lig = (Light*)selectedNode_;
			lig->minRadius_ = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "lig.maxR" ) == 0 )
		{
			Light* lig = (Light*)selectedNode_;
			lig->maxRadius_ = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "lig.falloff" ) == 0 )
		{
			Light* lig = (Light*)selectedNode_;
			lig->falloff_ = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "lig.power" ) == 0 )
		{
			Light* lig = (Light*)selectedNode_;
			lig->brightness_ = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "lig.diffuse" ) == 0 )
		{
			Light* lig = (Light*)selectedNode_;
			wxAny v = evt.GetProperty()->GetValue();
			wxColour c = v.As< wxColour >();
			lig->diffuse_.r = c.Red()*(1.0/255.0f);
			lig->diffuse_.b = c.Green()*(1.0/255.0f);
			lig->diffuse_.g = c.Blue()*(1.0/255.0f);
			lig->diffuse_.a = 1.0f;
		}
		else if ( evt.GetPropertyName().Cmp( "lig.specular" ) == 0 )
		{
			Light* lig = (Light*)selectedNode_;
			wxAny v = evt.GetProperty()->GetValue();
			wxColour c = v.As< wxColour >();
			lig->specular_.r = c.Red()*(1.0/255.0f);
			lig->specular_.b = c.Green()*(1.0/255.0f);
			lig->specular_.g = c.Blue()*(1.0/255.0f);
			lig->specular_.a = 1.0f;
		}
		else if ( evt.GetPropertyName().Cmp( "lig.ambient" ) == 0 )
		{
			Light* lig = (Light*)selectedNode_;
			wxAny v = evt.GetProperty()->GetValue();
			wxColour c = v.As< wxColour >();
			lig->ambient_.r = c.Red()*(1.0/255.0f);
			lig->ambient_.b = c.Green()*(1.0/255.0f);
			lig->ambient_.g = c.Blue()*(1.0/255.0f);
			lig->ambient_.a = 1.0f;
		}
		else if ( evt.GetPropertyName().Cmp( "cam.aspect" ) == 0 )
		{
			Camera* cam = (Camera*)selectedNode_;
			cam->aspect_ = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "cam.near" ) == 0 )
		{
			Camera* cam = (Camera*)selectedNode_;
			cam->nearPlane_ = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "cam.far" ) == 0 )
		{
			Camera* cam = (Camera*)selectedNode_;
			cam->farPlane_ = (float)evt.GetPropertyValue().GetDouble();
		}
		else if ( evt.GetPropertyName().Cmp( "cam.fov" ) == 0 )
		{
			Camera* cam = (Camera*)selectedNode_;
			cam->fov_ = HDegsToRads( (float)evt.GetPropertyValue().GetDouble() );
		}



		D3DXMatrixTransformation( &selectedNode_->matrix_, NULL, NULL, &sca, NULL, &rot, &trans );

		return redraw;
	}

}