/********************************************************************

	filename: 	MaterialListView.h	
	
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

#include "wx\wx.h"
#include "wx\listctrl.h"
#include "SceneDatabase.h"
#include "IPropertyGridChangeHandler.h"

class wxPropertyGrid;

namespace HScene
{
	class SceneGraph;
}

namespace UI
{
	class MaterialListView : public wxListCtrl,
							 public IPropertyGridChangeHandler
	{
	public:
		MaterialListView(
			HScene::SceneGraph* pScene,
			wxWindow* parent, 
			wxWindowID id, 
			wxPropertyGrid* pPropertyGrid,
			wxPoint position = wxDefaultPosition, 
			wxSize size = wxDefaultSize
			);
		virtual ~MaterialListView();

		void UpdatePropertyGrid( wxListEvent& evt, wxPropertyGrid* pPropertyGrid );
		void UpdateMaterialView();
		void OnPropertyGridCellChanging( wxPropertyGridEvent& evt );
		bool OnPropertyGridCellChanged( wxPropertyGridEvent& evt );

	private:

		static const char* NAME_PROPNAME;
		static const char* TWOSIDE_PROPNAME;
		static const char* WIREFRAME_PROPNAME;
		static const char* DEPTHTEST_PROPNAME;
		static const char* DEPTHWRITE_PROPNAME;
		static const char* ZTESTOP_PROPNAME;
		static const char* BLENDFUNC_PROPNAME;
		static const char* SRCBLENDOP_PROPNAME;
		static const char* DSTBLENDOP_PROPNAME;
		static const char* DIFFUSE_PROPNAME;
		static const char* AMBIENT_PROPNAME;
		static const char* SPECULAR_PROPNAME;
		static const char* EMISSIVE_PROPNAME;
		static const char* HLSLCODE_PROPNAME;
		static const char* DIFFUSETEX_PROPNAME;
		static const char* NORMALTEX_PROPNAME;
		static const char* SPECULARMAP_PROPNAME;
		static const char* LIGHTMAP_PROPNAME;

		static u32 EnumerateScenes( Data::HSceneHandle hScene, void* pUser );
		static u32 EnumerateTexture( Data::HTextureHandle hTex, void* pUser );

		wxPropertyGrid*			pPropertyGrid_;
		Data::Material*			pCurrentMat_;
		HScene::SceneGraph*		pScene_;
	};
}