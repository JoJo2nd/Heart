/********************************************************************

	filename: 	UIDefs.h	
	
	Copyright (c) 19:1:2011 James Moran
	
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

#ifndef UIDEFS_H__
#define UIDEFS_H__

#include "wx/wx.h"

namespace UI
{
	enum UI_ID
	{
		ID_MIN = wxID_HIGHEST,

		ID_TEST,

		//class: MainFrame
		ID_IMPORTSCENE,
		ID_EXPORTSCENE,
		ID_SHOWLOG,
		ID_SCENEVIEWTREE,
		ID_EXPANDSCENE,
		ID_HIDESCENE,
		ID_DELETESCENENODE,
		ID_MATERIALLIST,
		ID_TEXTURELIST,
		ID_MESHLIST,
		ID_PROPS,
		ID_ADDTEXTURE,

		//class: ImportSceneDialog
		ID_JOINIDENTICAL,
		ID_FINDINSTANCES,
		ID_OPTIMISEMESH,
		ID_OPTIMISEGRAPH,
		ID_NODENAMES,

		ID_MAX
	};
}

#endif // UIDEFS_H__