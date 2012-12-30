/********************************************************************

	filename: 	memtrack.h	
	
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

#ifndef MEMTRACK_H__
#define MEMTRACK_H__

#include "memlog.h"
#include "ioaccess.h"
#include "memleakpage.h"

class MemTrackMainFrame : public wxFrame
{
public:
    MemTrackMainFrame()
        : wxFrame(NULL, wxID_ANY, "MainFrame")
        , fileConfig_("MemTrack", "", "memtrack.cfg", "memtrack.cfg")
    {
        initFrame();
    }

private:

    void            initFrame();

    //Events
    DECLARE_EVENT_TABLE();
    void            evtOpen(wxCommandEvent& evt);
    void            evtClose(wxCloseEvent& evt);

    wxFileConfig    fileConfig_;
    wxFileHistory   fileHistory_;
    MemLog          memLog_;
    IODevice        parserFileAccess_;
    MemLeakPage*    leakListPage_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class MemTrackApp : public wxApp
{
public:

    bool OnInit();
};

#endif // MEMTRACK_H__