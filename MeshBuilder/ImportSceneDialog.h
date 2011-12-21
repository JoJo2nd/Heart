/********************************************************************

	filename: 	ImportSceneDialog.h	
	
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

#ifndef IMPORTSCENEDIALOG_H__
#define IMPORTSCENEDIALOG_H__

#include "stdafx.h"
#include "wx/wx.h"

namespace HScene
{
	class SceneGraph;
	class Node;
}

namespace UI
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class ImportSceneDialog : public wxDialog
	{
		DECLARE_EVENT_TABLE();
	public:
		ImportSceneDialog( HScene::SceneGraph* pScene, wxWindow* parent, wxWindowID id, const wxString* pFileName = NULL );
		~ImportSceneDialog();

		bool			JoinIdentical() const { return joinIdentical_; }
		bool			FindInstances() const { return findInstances_; }
		bool			OptimiseMesh() const { return optimiseMesh_; }
		bool			OptimiseGraph() const { return optimiseGraph_; }
		bool			CalcTangentsBinormals() const { return calcTangentSpace_; }
		const char*		GetFileName() const { return filename_.c_str(); }
		HScene::Node*	pParentNode() const { return pRoot_; }

	protected:

		void			OnOpen( wxCommandEvent& evt );
		void			OnCheckBoxChange( wxCommandEvent& evt );
		void			OnComboBoxSelect( wxCommandEvent& evt );

	private:

		HScene::Node*	FindNodeInList( const wxString& name, const std::vector< HScene::Node* >& nodes );

		bool joinIdentical_;
		bool findInstances_;
		bool optimiseMesh_;
		bool optimiseGraph_;
		bool validFileName_;
		bool calcTangentSpace_;
		HScene::SceneGraph* pScene_;
		HScene::Node* pRoot_;
		std::vector< HScene::Node* > nodes_;
		std::string filename_;
		wxBoxSizer* pTopSizer_;
		wxBoxSizer* pFilenameSizer_;
		wxBoxSizer* pOKCancelSizer_;
		wxBoxSizer* pSceneNodeSizer_;
		wxTextCtrl* pFilename_;
		wxStaticText* pStatic_;
		wxComboBox* pNodeNames_;
		wxButton* pOpenBut_;
		wxCheckBox* pJoinIdentCheck_;
		wxCheckBox* pFindInstanceCheck_;
		wxCheckBox* pOptimiseMeshCheck_;
		wxCheckBox* pOptimiseGraphCheck_;
		wxCheckBox* tangentSpaceCheck_;
		wxButton* pOK_;
		wxButton* pCancel_;
	};

}

#endif // IMPORTSCENEDIALOG_H__