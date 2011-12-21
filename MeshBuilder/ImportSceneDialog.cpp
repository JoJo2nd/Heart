/********************************************************************

	filename: 	ImportSceneDialog.cpp	
	
	Copyright (c) 16:1:2011 James Moran
	
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
#include "ImportSceneDialog.h"
#include "UIDefs.h"
#include "Resource.h"
#include "SceneGraph.h"
#include <wx/combobox.h>

namespace UI
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	BEGIN_EVENT_TABLE( ImportSceneDialog, wxDialog )
		EVT_BUTTON( wxID_OPEN, ImportSceneDialog::OnOpen )
		EVT_CHECKBOX( ID_JOINIDENTICAL, ImportSceneDialog::OnCheckBoxChange )
		EVT_CHECKBOX( ID_FINDINSTANCES, ImportSceneDialog::OnCheckBoxChange )
		EVT_CHECKBOX( ID_OPTIMISEMESH, ImportSceneDialog::OnCheckBoxChange )
		EVT_CHECKBOX( ID_OPTIMISEGRAPH, ImportSceneDialog::OnCheckBoxChange )
		EVT_COMBOBOX( ID_NODENAMES, ImportSceneDialog::OnComboBoxSelect )
	END_EVENT_TABLE()

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	ImportSceneDialog::ImportSceneDialog( 
		HScene::SceneGraph* pScene, 
		wxWindow* parent, 
		wxWindowID id,
		const wxString* pFileName ) :
			wxDialog( 
				parent, 
				id, 
				"Import Scene Options", 
				wxDefaultPosition, 
				wxDefaultSize, 
				wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX )
			,pRoot_( NULL )
			,pScene_( pScene )
			,joinIdentical_( false )
			,findInstances_( false )
			,optimiseMesh_( false )
			,optimiseGraph_( false )
			,validFileName_( false )
			,calcTangentSpace_( false )
	{
		pTopSizer_ = new wxBoxSizer( wxVERTICAL );
		
		pFilenameSizer_ = new wxBoxSizer( wxHORIZONTAL );
		pFilename_ = new wxTextCtrl( 
			this, 
			wxID_ANY, 
			"", 
			wxDefaultPosition, 
			wxSize( 200, 25 ), 
			wxTE_READONLY | wxALIGN_RIGHT );
		pOpenBut_ = new wxButton( 
			this,
			wxID_OPEN,
			"Open Scene" );
		pFilenameSizer_->Add( pFilename_, 70, wxSHAPED | wxALL, 2 );
		pFilenameSizer_->Add( pOpenBut_, 30, wxSHAPED | wxALL | wxALIGN_RIGHT, 2 );



		pJoinIdentCheck_ = new wxCheckBox( 
			this,
			ID_JOINIDENTICAL,
			"Join Identical Vertices'" );
		pFindInstanceCheck_ = new wxCheckBox( 
			this,
			ID_FINDINSTANCES,
			"Find Mesh Instances" );
		pOptimiseMeshCheck_ = new wxCheckBox( 
			this,
			ID_OPTIMISEMESH,
			"Optimise Meshes" );
		pOptimiseGraphCheck_ = new wxCheckBox( 
			this,
			ID_OPTIMISEGRAPH,
			"Optimise Graph" );
		tangentSpaceCheck_ = new wxCheckBox( 
			this,
			ID_OPTIMISEGRAPH,
			"Calculate Tangents/Binormals" );

		pSceneNodeSizer_ = new wxBoxSizer( wxHORIZONTAL );
		pStatic_ = new wxStaticText( 
			this, 
			wxID_ANY, 
			"Add Scene As Child of:",
			wxDefaultPosition, 
			wxSize( 150, 25 ), 
			wxALIGN_RIGHT );
		pNodeNames_ = new wxComboBox( 
			this, 
			ID_NODENAMES, 
			"", 
			wxDefaultPosition, 
			wxDefaultSize, 
			NULL,
			0, 
			wxCB_READONLY | wxCB_SORT );
		HScene::GetNodeList( pScene_, &nodes_ );
		for ( u32 i = 0; i < nodes_.size(); ++i )
		{
			pNodeNames_->Append( nodes_[i]->pName() );
		}
		pNodeNames_->SetStringSelection( pScene_->pRoot()->pName() );
		pRoot_ = FindNodeInList( pNodeNames_->GetValue(), nodes_ );

		pSceneNodeSizer_->Add( pStatic_, 40, wxSHAPED | wxALL | wxALIGN_RIGHT, 2 );
		pSceneNodeSizer_->Add( pNodeNames_, 60, wxSHAPED | wxALL, 2 );

		pOKCancelSizer_  = new wxBoxSizer( wxHORIZONTAL );
		pOK_ = new wxButton(
			this, 
			wxID_OK, 
			"OK" );
		pOK_->Disable();
		pCancel_ = new wxButton( 
			this,
			wxID_CANCEL,
			"Cancel" );
		pOKCancelSizer_->Add( pOK_, 1, wxEXPAND | wxALL | wxALIGN_CENTER_HORIZONTAL, 2 );
		pOKCancelSizer_->Add( pCancel_, 1, wxEXPAND | wxALL | wxALIGN_CENTER_HORIZONTAL, 2 );

		pTopSizer_->Add( pFilenameSizer_ );
		pTopSizer_->Add( pSceneNodeSizer_ );
		pTopSizer_->Add( pJoinIdentCheck_, 1, wxEXPAND | wxALL, 2 );
		pTopSizer_->Add( pFindInstanceCheck_, 1, wxEXPAND | wxALL, 2 );
		pTopSizer_->Add( pOptimiseMeshCheck_, 1, wxEXPAND | wxALL, 2 );
		pTopSizer_->Add( pOptimiseGraphCheck_, 1, wxEXPAND | wxALL, 2 );
		pTopSizer_->Add( tangentSpaceCheck_, 1, wxEXPAND | wxALL, 2 );
		pTopSizer_->Add( pOKCancelSizer_ );

		SetAutoLayout( true );
		SetSizer( pTopSizer_ );
		pTopSizer_->Fit( this );
		pTopSizer_->SetSizeHints( this );

		if ( pFileName && !pFileName->IsEmpty() )
		{
			filename_ = *pFileName;
			pFilename_->SetValue( *pFileName );
			pOK_->Enable();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	ImportSceneDialog::~ImportSceneDialog()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ImportSceneDialog::OnOpen( wxCommandEvent& evt )
	{
		aiString sz;
		aiGetExtensionList(&sz);

		std::string filetypes = "Collada Assets|*.dae";
		//filetypes += sz.data;

		wxFileDialog openFileDialog = new wxFileDialog( 
			this, 
			"Open File", 
			"", //default file
			"", //default path
			filetypes.c_str(), 
			wxID_OPEN );

		if ( openFileDialog.ShowModal() == wxID_OK )
		{
			filename_ += openFileDialog.GetDirectory();
			filename_ += "\\";
			filename_ += openFileDialog.GetFilename();
		}

		if ( !filename_.empty() )
		{
			pFilename_->SetValue( filename_ );
			pTopSizer_->Fit( this );
			pTopSizer_->SetSizeHints( this );
			pOK_->Enable();
		}
		else 
		{
			pOK_->Disable();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ImportSceneDialog::OnCheckBoxChange( wxCommandEvent& evt )
	{
		joinIdentical_ = pJoinIdentCheck_->GetValue();
		findInstances_ = pFindInstanceCheck_->GetValue();
		optimiseMesh_ = pOptimiseMeshCheck_->GetValue();
		optimiseGraph_ = pOptimiseGraphCheck_->GetValue();
		calcTangentSpace_ = tangentSpaceCheck_->GetValue();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ImportSceneDialog::OnComboBoxSelect( wxCommandEvent& evt )
	{
		pRoot_ = FindNodeInList( pNodeNames_->GetValue(), nodes_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	HScene::Node* ImportSceneDialog::FindNodeInList( const wxString& name, const std::vector< HScene::Node* >& nodes )
	{
		for ( std::vector< HScene::Node* >::const_iterator i = nodes.begin(), iend = nodes.end(); 
			  i != iend; ++i )
		{
			if ( name.Cmp( (*i)->pName() ) == 0 )
			{
				return *i;
			}
		}

		return NULL;
	}

}