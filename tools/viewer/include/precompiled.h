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

#include "heart.h"

#include "wx/wx.h"
#ifdef __WXMSW__
#include "wx/msw/private.h"
#endif
#include "wx/app.h"
#include "wx/cmdline.h"
#include "wx/busyinfo.h"
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
#include "wx/msgdlg.h"

#include "boost/filesystem.hpp"
#include "boost/signals2.hpp"
#include "boost/smart_ptr.hpp"
#include "boost/foreach.hpp"
#include "boost/crc.hpp"
#include "boost/bimap.hpp"

#define boost_foreach    BOOST_FOREACH

#include "viewer_api.h"

#include "uieventsids.h"

typedef unsigned int uint;

#endif // PRECOMPILED_H__