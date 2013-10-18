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

struct ServerConnectMessage
{
    ENetAddress address_;
    bool        connect_;//true if a connect is required. false to disconnect
    bool        exit_;
};

class ConnectionStateUpdate
{
public:
    ConnectionStateUpdate() {}
    ConnectionStateUpdate(const char* statemsg)
        : readableMsg_(statemsg)
    {}
    const char* getReadableStateMsg() const { return readableMsg_.c_str(); }
private:

    wxString readableMsg_;
};

wxDECLARE_EVENT(uiEVT_SERVER_THREAD_UPDATE, wxThreadEvent);

class GameClientThread : public wxThread
{
public:
    GameClientThread(wxFrame* handler, wxMessageQueue<ENetPacket*>* dispatchqueue, wxMessageQueue<ServerConnectMessage>* connectionqueue)
        : wxThread(wxTHREAD_JOINABLE)
        , commandCounter_(0)
        , msgHandler_(handler)
        , dispatchQueue_(dispatchqueue)
        , connectionQueue_(connectionqueue)
    {
        Create();
        Run();
    }

    void prepareGameNetworkMessage(::google::protobuf::MessageLite* msglite, const char* commandName, hBool reliable);

private:

    virtual ExitCode Entry();

    uint                                    commandCounter_;
    wxFrame*                                msgHandler_;
    wxMessageQueue<ENetPacket*>*            dispatchQueue_;
    wxMessageQueue<ServerConnectMessage>*   connectionQueue_;
};

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
    void            evtConnectChange(wxCommandEvent& evt);
    void            evtServerThreadUpdate(wxThreadEvent& evt);
    void            evtClose(wxCloseEvent& evt);

    wxFileConfig    fileConfig_;
    wxFileHistory   fileHistory_;
    MemLog          memLog_;
    IODevice        parserFileAccess_;
    MemLeakPage*    leakListPage_;
    MemMarkerPage*  makerTreePage_;
    wxTextCtrl*     serverIP_;
    wxTextCtrl*     portNum_;
    wxToggleButton* connectToggle_;
    wxStaticText*   connectionStateText_;

    GameClientThread*           networkThread_;
    wxMessageQueue<ENetPacket*> dispatchQueue_;
    wxMessageQueue<ServerConnectMessage> serverConnectQueue_;

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