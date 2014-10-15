/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#ifndef PRECOMPILED_H__
#define PRECOMPILED_H__

//include windows first then winundef.h otherwise we get some strange compile errors
#include <winsock2.h>
#include <windows.h>
#include "wx/msw/winundef.h"
//order of these 1st 3 is important

#include "memtracktypes.h"

#include "wx/wx.h"
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

#include "boost/filesystem.hpp"

#include "uidefines.h"

#include "enet/enet.h"

#endif // PRECOMPILED_H__