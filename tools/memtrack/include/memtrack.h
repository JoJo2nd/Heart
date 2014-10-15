/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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
    MemMarkerPage*  makerTreePage_;

    ENetHost*       client_;
    ENetPeer*       peer_;
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