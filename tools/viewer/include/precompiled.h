/********************************************************************

	filename: 	precompiled.h	
	
	Copyright (c) 9:11:2012 James Moran
	
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
#pragma once

#ifndef PRECOMPILED_H__
#define PRECOMPILED_H__

//include windows first then winundef.h otherwise we get some strange compile errors
#include <windows.h>
#include "wx/msw/winundef.h"
//order of these 1st 3 is important

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

#include "heart.h"

#ifndef _CRTDBG_MAP_ALLOC
#   define _CRTDBG_MAP_ALLOC
#endif

#include "wx/wx.h"
#include "wx/app.h"
#include "wx/cmdline.h"
#include "wx/busyinfo.h"
#include "wx/menu.h"
#include "wx/aui/aui.h"
#include "wx/treectrl.h"
#include "wx/propgrid/propgrid.h"
#include "wx/fileconf.h"
#include "wx/msgdlg.h"
#include "wx/filedlg.h"
#include "wx/wfstream.h"
#include "wx/filehistory.h"
#include "wx/progdlg.h"
#include "wx/richmsgdlg.h"
#include "wx/listctrl.h"
#include "wx/gbsizer.h"
#include "wx/generic/statbmpg.h"
#include "wx/listctrl.h"
#include "wx/stattext.h"
#include "wx/filepicker.h"
#include "wx/combobox.h"
#include "wx/checkbox.h"
#include "wx/choicdlg.h"
#include "wx/event.h"

#include "boost/cstdint.hpp"
#include "boost/system/error_code.hpp"
#include "boost/filesystem.hpp"
#include "boost/signals2.hpp"
#include "boost/smart_ptr.hpp"
#include "boost/foreach.hpp"
#include "boost/crc.hpp"
#include "boost/bimap.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/version.hpp"
#include "boost/serialization/split_member.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/list.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/shared_ptr.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "nvtt/nvtt.h"

#include "freeimage.h"

#define boost_foreach    BOOST_FOREACH

extern "C"
{
    WXDLLIMPEXP_BASE HINSTANCE wxGetInstance();
}

#include "common/module.h"
#include "common/action_stack.h"

#include "uieventsids.h"

typedef unsigned int        uint;
typedef unsigned long long  uint64;

extern boost::signals2::signal< void (const hChar*) > evt_consoleOutputSignal;
extern boost::signals2::signal< void (const hChar*) > evt_consoleInputSignal;
extern boost::signals2::signal< void () > evt_mainWindowCreate;
extern boost::signals2::signal< void (wxWindow*, const wxString&, const wxAuiPaneInfo&) > evt_registerAuiPane;

#endif // PRECOMPILED_H__