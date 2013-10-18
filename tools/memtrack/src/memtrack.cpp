/********************************************************************

	filename: 	memtrack.cpp	
	
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

#include "precompiled.h"
#include "memtrack.h"

IMPLEMENT_APP(MemTrackApp);

wxDEFINE_EVENT(uiEVT_SERVER_THREAD_UPDATE, wxThreadEvent);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool MemTrackApp::OnInit()
{
    wxInitAllImageHandlers();

    MemTrackMainFrame* frame = new MemTrackMainFrame();
    frame->Show();
    SetTopWindow(frame);

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MemTrackMainFrame, wxFrame)
    EVT_MENU(wxID_OPEN, MemTrackMainFrame::evtOpen)
    EVT_TOGGLEBUTTON(uiID_CONNECT_CHANGE, MemTrackMainFrame::evtConnectChange)
    EVT_CLOSE(MemTrackMainFrame::evtClose)  
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemTrackMainFrame::initFrame()
{
    Bind(uiEVT_SERVER_THREAD_UPDATE, &MemTrackMainFrame::evtServerThreadUpdate, this, wxID_ANY);

    wxMenuBar* menubar = new wxMenuBar();

    wxMenu* filemenu = new wxMenu();
    filemenu->Append(wxID_OPEN, "&Open");
//     filemenu->Append(wxID_SAVE, "&Save");
//     filemenu->Append(wxID_SAVEAS, "Save &As");
//     filemenu->Append(uiID_IMPORT, "&Import Memory Log");

    menubar->Append(filemenu, "&File");

    SetMenuBar(menubar);

    wxNotebook* notebook = new wxNotebook(this, wxID_ANY);

    leakListPage_ = new MemLeakPage(notebook, wxID_ANY);
    makerTreePage_= new MemMarkerPage(notebook, wxID_ANY);

    notebook->InsertPage(0, leakListPage_, "Leaks");
    notebook->InsertPage(1, makerTreePage_, "Markers");

    wxToolBar* toolbar=CreateToolBar(/*wxTB_HORZ_TEXT*/);
//     wxBoxSizer* toolbarsizer=new wxBoxSizer(wxHORIZONTAL);
//     toolbarsizer->Add(new wxStaticText(toolbar, wxID_ANY, "Server IP Address"));
//     toolbarsizer->Add(new wxTextCtrl(toolbar, wxID_ANY));
//     toolbarsizer->Add(new wxStaticText(toolbar, wxID_ANY, "Server Port"));
//     toolbarsizer->Add(new wxTextCtrl(toolbar, wxID_ANY));
//     toolbarsizer->Add(new wxToggleButton(toolbar, wxID_ANY, "Connect/Disconnect"));
//     toolbarsizer->Add(new wxStaticText(toolbar, wxID_ANY, "Disconnected"));

    serverIP_=new wxTextCtrl(toolbar, wxID_ANY);
    portNum_=new wxTextCtrl(toolbar, wxID_ANY, "1000", wxDefaultPosition, wxDefaultSize, 0, wxIntegerValidator<short>());
    connectToggle_=new wxToggleButton(toolbar, uiID_CONNECT_CHANGE, "Connect/Disconnect");
    connectionStateText_=new wxStaticText(toolbar, wxID_ANY, wxEmptyString);

    toolbar->AddControl(new wxStaticText(toolbar, wxID_ANY, "Server IP Address :"));
    toolbar->AddControl(serverIP_, "Server IP Address");
    toolbar->AddControl(new wxStaticText(toolbar, wxID_ANY, "Server IP Port :"));
    toolbar->AddControl(portNum_, "Server IP Port");
    toolbar->AddControl(connectToggle_);
    toolbar->AddControl(connectionStateText_);
    toolbar->Realize();
    
    networkThread_ = new GameClientThread(this, &dispatchQueue_, &serverConnectQueue_);

    //Heart::proto::ServiceRegister msg;
    //msg.set_sevicename("console.log");

    /*client_=enet_host_create(nullptr, 1, 1, 0, 0);

    ENetAddress hostaddress={0};
    //enet_address_set_host(&hostaddress, "192.168.1.201");
    enet_address_set_host(&hostaddress, "James-PC");
    hostaddress.port=8335;

    peer_=enet_host_connect(client_, &hostaddress, 1, 0);

    ENetEvent event;
    while(enet_host_service(client_, &event, 0)>=0) {
        switch(event.type) {
        case ENET_EVENT_TYPE_NONE: break;
        case ENET_EVENT_TYPE_CONNECT: {
            uint address=event.peer->address.host;
            //hcPrintf("Connection from %u.%u.%u.%u : %u", (address&0xFF00000000)>>24, (address&0xFF000000)>>16, (address&&0xFF00)>>8, (address&0xFF), enetevent.peer->address.port);
            //peers_.push_back(enetevent.peer);
        } break;
        case ENET_EVENT_TYPE_DISCONNECT: {
            uint address=event.peer->address.host;
            //hcPrintf("Disconnect from %u.%u.%u.%u : %u", (address&0xFF00000000)>>24, (address&0xFF000000)>>16, (address&&0xFF00)>>8, (address&0xFF), enetevent.peer->address.port);
        } break;
        case ENET_EVENT_TYPE_RECEIVE: {

        } break;
        default: break;
        }
    }*/
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemTrackMainFrame::evtClose( wxCloseEvent& evt )
{
    ServerConnectMessage msg = {0};
    msg.exit_=true;
    serverConnectQueue_.Post(msg);
    networkThread_->Wait(wxTHREAD_WAIT_DEFAULT);
    evt.Skip();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemTrackMainFrame::evtOpen(wxCommandEvent& evt)
{
    wxFileDialog fileopen(this, 
        "Open MemLog file", "", "",
        "Text files (*.txt)|*.txt", 
        wxFD_OPEN|wxFD_FILE_MUST_EXIST);

    if (fileopen.ShowModal() == wxID_CANCEL) return;

    wxWindowDisabler disableAll;
    wxBusyInfo wait("Opening Log, Please wait...");

    memLog_.clear();
    Callstack::clearSymbolMap();
    parseMemLog(fileopen.GetPath().c_str(), &memLog_, &parserFileAccess_);

    leakListPage_->updateMemLeaks(&memLog_);
    makerTreePage_->updateMarkerTree(&memLog_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemTrackMainFrame::evtConnectChange(wxCommandEvent& evt) {
    ServerConnectMessage msg = {0};
    msg.connect_=connectToggle_->GetValue();

    if (msg.connect_) {
        enet_address_set_host(&msg.address_, serverIP_->GetValue().c_str());
        long port;
        if (portNum_->GetValue().ToLong(&port)) {
            msg.address_.port=(short)port;
        }
    }
    serverConnectQueue_.Post(msg);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemTrackMainFrame::evtServerThreadUpdate(wxThreadEvent& evt) {
    ConnectionStateUpdate update=evt.GetPayload<ConnectionStateUpdate>();
    connectionStateText_->SetLabelText(update.getReadableStateMsg());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

wxThread::ExitCode GameClientThread::Entry() {
    ServerConnectMessage cntmsg;
    bool complete=false;
    ENetHost*       client_=nullptr;
    ENetPeer*       peer_=nullptr;
    ENetAddress     hostaddress_={0};
    bool            attemptConnection_=false;
    bool            disconnecting=false;
    enet_initialize();
    client_=enet_host_create(nullptr, 1, 1, 0, 0);

    while (!complete) {
        disconnecting=false;
        wxThreadEvent* wxevent=new wxThreadEvent(uiEVT_SERVER_THREAD_UPDATE);
        wxevent->SetPayload(ConnectionStateUpdate("Disconnected"));
        wxQueueEvent(msgHandler_, wxevent);

        connectionQueue_->Receive(cntmsg);
        attemptConnection_=cntmsg.connect_;
        if (cntmsg.exit_) {
            complete=true;
            attemptConnection_=false;
        }

        while (attemptConnection_ || peer_) {
            if (!peer_) {
                peer_=enet_host_connect(client_, &cntmsg.address_, 1, 0);
                wxevent=new wxThreadEvent(uiEVT_SERVER_THREAD_UPDATE);
                wxevent->SetPayload(ConnectionStateUpdate("Connecting"));
                wxQueueEvent(msgHandler_, wxevent);
            }

            if (!peer_) {
                wxSleep(1);
            } else {
                ENetEvent event;
                if (enet_host_service(client_, &event, 0)>=0) {
                    switch(event.type) {
                    case ENET_EVENT_TYPE_NONE: break;
                    case ENET_EVENT_TYPE_CONNECT: {
                        uint address=event.peer->address.host;
                        //hcPrintf("Connection from %u.%u.%u.%u : %u", (address&0xFF00000000)>>24, (address&0xFF000000)>>16, (address&&0xFF00)>>8, (address&0xFF), enetevent.peer->address.port);
                        //peers_.push_back(enetevent.peer);
                        wxevent=new wxThreadEvent(uiEVT_SERVER_THREAD_UPDATE);
                        wxevent->SetPayload(ConnectionStateUpdate("Connected"));
                        wxQueueEvent(msgHandler_, wxevent);
                    } break;
                    case ENET_EVENT_TYPE_DISCONNECT: {
                        uint address=event.peer->address.host;
                        //connection failed/is gone. break out and try again...
                        if (peer_) {
                            peer_=nullptr;
                            wxevent=new wxThreadEvent(uiEVT_SERVER_THREAD_UPDATE);
                            wxevent->SetPayload(ConnectionStateUpdate("Disconnected"));
                            wxQueueEvent(msgHandler_, wxevent);
                        }
                        //hcPrintf("Disconnect from %u.%u.%u.%u : %u", (address&0xFF00000000)>>24, (address&0xFF000000)>>16, (address&&0xFF00)>>8, (address&0xFF), enetevent.peer->address.port);
                    } break;
                    case ENET_EVENT_TYPE_RECEIVE: {

                    } break;
                    default: break;
                    }
                }
            }
            connectionQueue_->ReceiveTimeout(1, cntmsg);
            attemptConnection_=cntmsg.connect_;
            if (cntmsg.exit_) {
                complete=true;
                attemptConnection_=false;
            }
            if (peer_ && !attemptConnection_ && !disconnecting) {
                enet_peer_disconnect(peer_, 0);
                wxevent=new wxThreadEvent(uiEVT_SERVER_THREAD_UPDATE);
                wxevent->SetPayload(ConnectionStateUpdate("Disconnecting"));
                wxQueueEvent(msgHandler_, wxevent);
                disconnecting=true;
            }
        }
        
    }

    enet_host_destroy(client_);
    enet_deinitialize();
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GameClientThread::prepareGameNetworkMessage(::google::protobuf::MessageLite* msglite, const char* commandName, hBool reliable) {
    hUint flags = 0;
    flags |= reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
    hUint datasize=msglite->ByteSize();
    std::string msgstr=msglite->SerializeAsString();
    ENetPacket* packet=enet_packet_create(hNullptr, datasize+Heart::hNetPacketHeader::s_packetHeaderByteSize, flags);
    Heart::hNetPacketHeader* hdr=(Heart::hNetPacketHeader*)packet->data;
    hdr->type_ =Heart::hNetPacketHeader::eCommand;
    hdr->seqID_=commandCounter_;
    hdr->nameID_=cyStringCRC32(commandName);
    memcpy(((hByte*)packet->data)+Heart::hNetPacketHeader::s_packetHeaderByteSize, msgstr.c_str(), datasize);

    dispatchQueue_->Post(packet);

    ++commandCounter_;
}
