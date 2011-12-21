/********************************************************************

	filename: 	TextureListView.h	
	
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

#ifndef TEXTURELISTVIEW_H__
#define TEXTURELISTVIEW_H__

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
	class TextureListView : public wxListCtrl,
							public IPropertyGridChangeHandler
	{
	public:
		TextureListView(
			HScene::SceneGraph* pScene,
			wxWindow* parent, 
			wxWindowID id, 
			wxPoint position = wxDefaultPosition, 
			wxSize size = wxDefaultSize
			);
		virtual ~TextureListView();

		void UpdatePropertyGrid( wxListEvent& evt, wxPropertyGrid* pPropertyGrid );
		void UpdateTextureView();

		void OnPropertyGridCellChanging( wxPropertyGridEvent& evt );
		bool OnPropertyGridCellChanged( wxPropertyGridEvent& evt );

	private:

		static const char* FILE_PROP_NAME;
		static const char* FORMAT_PROP_NAME;

		u32						hCurrentTexture_;
		HScene::SceneGraph*		pSceneGraph_;

	};

}

#endif // TEXTURELISTVIEW_H__