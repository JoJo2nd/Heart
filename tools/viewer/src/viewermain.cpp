/********************************************************************

    filename:   viewermain.cpp  
    
    Copyright (c) 28:12:2012 James Moran
    
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
#include "viewermain.h"
#include "consolelog.h"

extern boost::signals2::signal< void (const hChar*) > evt_consoleOutputSignal;
extern boost::signals2::signal< void (const hChar*) > evt_consoleInputSignal;
extern boost::signals2::signal< void () > evt_mainWindowCreate;
extern boost::signals2::signal< void (wxWindow*, const wxString&, const wxAuiPaneInfo&) > evt_registerAuiPane;

IMPLEMENT_APP(ViewerApp);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool ViewerApp::OnInit()
{
    if (!wxApp::OnInit()) return false;

    wxInitAllImageHandlers();

    ViewerMainFrame* frame = new ViewerMainFrame(heartPath_);
    frame->Show();
    SetTopWindow(frame);

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
        { wxCMD_LINE_OPTION, "h", "heartlibpath", "Directory to find heart engine libraries", wxCMD_LINE_VAL_STRING, 0 },
        { wxCMD_LINE_OPTION, "p", "pluginpaths", "Directories to search for plug-in libraries, seperated by ';'", wxCMD_LINE_VAL_STRING, 0 },
        { wxCMD_LINE_NONE }
    };

    parser.SetDesc(cmdLineDesc);
    parser.SetSwitchChars (wxT("-"));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool ViewerApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    wxString tmp,wrk;

    if (parser.Found("heartlibpath", &heartPath_)) {
        heartPath_=boost::filesystem::absolute(heartPath_.ToStdString()).c_str();
    }
    else {
        heartPath_="";
    }

    pluginPaths_="";
    if (parser.Found("pluginpaths", &tmp)) {
        while(tmp.length()>0) {
            wrk=tmp.BeforeFirst(';');
            tmp=tmp.AfterFirst(';');
            pluginPaths_+=";";
            pluginPaths_+=boost::filesystem::absolute(wrk.ToStdString()).c_str();
        }
    }

    if (pluginPaths_.length()>0) {
        wxString pathev="path=";
        pathev+=getenv("path");
        pathev+=pluginPaths_;
        _putenv(pathev.c_str());
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(ViewerMainFrame, wxFrame)
    EVT_MENU(wxID_OPEN, ViewerMainFrame::evtOpen)
    EVT_MENU(cuiID_SHOWCONSOLE, ViewerMainFrame::evtShowConsole)
    EVT_AUI_PANE_CLOSE(ViewerMainFrame::evtOnPaneClose)
    EVT_CLOSE(ViewerMainFrame::evtClose)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ViewerMainFrame::~ViewerMainFrame()
{
    auiManager_->UnInit();
    delete auiManager_; auiManager_ = NULL;
    boost_foreach(boost::signals2::connection c, connnections_) {
        c.disconnect();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::initFrame(const wxString& heartpath)
{
    auiManager_ = new wxAuiManager(this, wxAUI_MGR_DEFAULT | wxAUI_MGR_TRANSPARENT_DRAG);

    wxMenuBar* menubar = new wxMenuBar();

    wxMenu* filemenu = new wxMenu();
    filemenu->Append(wxID_OPEN, "&Open");
    filemenu->Append(wxID_SAVE, "&Save");
    filemenu->Append(wxID_SAVEAS, "Save &As");
    filemenu->AppendSeparator();
    filemenu->Append(cuiID_SHOWCONSOLE, "Show &Console");

    menubar->Append(filemenu, "&File");

    SetMenuBar(menubar);

    wxPanel* renderFrame = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(1280, 720));

    {
        wxAuiPaneInfo paneinfo;
        paneinfo.Caption("Render Window");
        paneinfo.CaptionVisible(true);
        paneinfo.CenterPane();
        auiManager_->AddPane(renderFrame, paneinfo);
    }

    auiManager_->Update();

    connnections_.push_back(evt_consoleInputSignal.connect(boost::bind(&ViewerMainFrame::consoleInput, this, _1)));
    connnections_.push_back(evt_registerAuiPane.connect(boost::bind(&ViewerMainFrame::dockPaneRegister, this, _1, _2, _3)));

    ConsoleLog* consoleLog = new ConsoleLog(this);

    hHeartEngineCallbacks callbacks = {0};
    callbacks.overrideFileRoot_ = NULL; 
    if (heartpath.length()>0) {
        callbacks.overrideFileRoot_ = heartpath.c_str();
    }
    callbacks.consoleCallback_ = &ViewerMainFrame::consoleMsgCallback;
    callbacks.consoleCallbackUser_ = this;
    heart_ = hHeartInitEngine(&callbacks, wxGetInstance(), renderFrame->GetHWND());

    timer_.start(heart_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::evtClose( wxCloseEvent& evt )
{
    evt.Skip();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::evtOpen(wxCommandEvent& evt)
{
    wxFileDialog fileopen(this, 
        "Open MemLog file", "", "",
        "Text files (*.txt)|*.txt", 
        wxFD_OPEN|wxFD_FILE_MUST_EXIST);

    if (fileopen.ShowModal() == wxID_CANCEL) return;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::evtShowConsole(wxCommandEvent& evt)
{
    wxAuiPaneInfo& info = auiManager_->GetPane("Console Window");
    if (info.IsOk()) {
        info.Show();
        auiManager_->Update();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::consoleMsgCallback(const hChar* msg, void* user)
{
    // Can't go to control here because this callback happens on any thread
    // so buffer the result
    ((ViewerMainFrame*)user)->timer_.flushConsoleText(msg);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::consoleInput(const hChar* msg)
{
    heart_->GetConsole()->ExecuteBuffer(msg);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::dockPaneRegister(wxWindow* pane, const wxString& name, const wxAuiPaneInfo& info)
{
    auiManager_->AddPane(pane, info);

    // update view
    auiManager_->Update();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::evtOnPaneClose(wxAuiManagerEvent& evt)
{
    wxAuiPaneInfo* info = evt.GetPane();

    // update view
    auiManager_->Update();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void RenderTimer::Notify()
{
    if (heart_) {
        {
            wxMutexLocker l(access_);
            if (consoleOutput_.length() > 0) {
                evt_consoleOutputSignal(consoleOutput_.c_str());
                consoleOutput_.clear();
            }
        }

        hHeartDoMainUpdate(heart_);
    }
}
