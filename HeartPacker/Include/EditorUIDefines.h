/********************************************************************

	filename: 	EditorUIDefines.h	
	
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
#ifndef EDITORUIDEFINES_H__
#define EDITORUIDEFINES_H__

namespace UIID
{
inline unsigned int GenerateUniqueMenuID()
{
    static unsigned int IDGen = 0;

    return ++IDGen;
}
}

#define DEFINE_UNIQUE_MENU_ID() static unsigned int GetMenuID() { static const unsigned int id = UIID::GenerateUniqueMenuID(); return id; } 
#define DEFINE_UNIQUE_MENU_CUSTOM_ID( MenuName ) static unsigned int GetMenuID##MenuName() { static const unsigned int id = UIID::GenerateUniqueMenuID(); return id; } 

#define DEFINE_WINDOW_PANE_INSTANCE( kl ) \
	public: \
		static kl* GetInstance() { return instance_; }\
		static const char* GetWindowPaneName() { static const char* name = #kl; return name; }\
        DEFINE_UNIQUE_MENU_ID() \
		const wxAuiPaneInfo& GetDefaultPaneInfo() { return defaultPaneInfo_; }\
	private:\
		static kl* instance_;\
		wxAuiPaneInfo defaultPaneInfo_

#define DECLARE_WINDOW_PANE_INSTANCE( kl ) kl* kl::instance_ = NULL

enum UIEventID
{
	UIID_SHOWDATALAYOUT,
	UIID_PROPERTIESPANE,
	UIID_SAVEWINDOWLAYOUT,
    UIID_LOADWINDOWLAYOUT,
    UIID_SETLAYOUTASDEFAULT,
    UIID_NEWGAMEDATADATABASE,
    UIID_OPENGAMEDATADATABASE,
    UIID_SAVEGAMEDATADATABASE,
    UIID_LOADGAMEDATADATABASE,

    UIID_BUILDALL,
    UIID_CLEAN,
    UIID_SETOUTPUT,

    UIID_MAX
};

#define RESOURCE_NONE_TYPE_NAME ("NONE")

#include "UIPropertyID.h"

#endif // EDITORUIDEFINES_H__