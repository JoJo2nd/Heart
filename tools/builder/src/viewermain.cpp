/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/


#include "precompiled/precompiled.h"
#include "common/ui_id.h"
#include "viewermain.h"
#include "consolelog.h"
#include "pkg_viewer.h"
#include "build_system.h"

IMPLEMENT_APP(ViewerApp);

namespace {
    ui::ID ID_SHOWCONSOLE = ui::marshallNameToID("SHOWCONSOLE");
    ui::ID ID_BUILDDATA = ui::marshallNameToID("BUILDDATA");
    ui::ID ID_INPUTDATA = ui::marshallNameToID("SETINPUTDATA");
    ui::ID ID_OUTPUTDATA = ui::marshallNameToID("SETOUTPUTDATA");
    ViewerMainFrame* g_mainFrame = nullptr;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool ViewerApp::OnInit()
{
    if (!wxApp::OnInit()) return false;

    wxInitAllImageHandlers();

    ViewerMainFrame* frame = new ViewerMainFrame(heartPath_, pluginPaths_);
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
#if 0 // !!JM todo: windows warns about putenv, todo handle better.
    if (pluginPaths_.length()>0) {
        wxString pathev="path=";
        pathev+=getenv("path");
        pathev+=pluginPaths_;
        // !!JM todo: windows warns about putenv, todo handle better.
        std::unique_ptr<char*> data;
        data = new char[pathev.length()+1];
        data[pathev.length()] = 0;
        memcpy(data.get(), pathev.c_str(), pathev.length());
        putenv(data.get());
    }
#endif

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(ViewerMainFrame, wxFrame)
    EVT_MENU(ID_SHOWCONSOLE, ViewerMainFrame::evtShowConsole)
    EVT_MENU(ID_BUILDDATA, ViewerMainFrame::evtDoDataBuild)
    EVT_MENU(ID_INPUTDATA, ViewerMainFrame::evtDoSelectDataInput)
    EVT_MENU(ID_OUTPUTDATA, ViewerMainFrame::evtDoSelectDataOutput)
    EVT_AUI_PANE_CLOSE(ViewerMainFrame::evtOnPaneClose)
    EVT_CLOSE(ViewerMainFrame::evtClose)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ViewerMainFrame::~ViewerMainFrame()
{
    auiManager_->UnInit();
    delete auiManager_; 
    auiManager_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::initFrame(const wxString& heartpath, const wxString& pluginPaths)
{
    g_mainFrame = this;
    auiManager_ = new wxAuiManager(this, wxAUI_MGR_DEFAULT | wxAUI_MGR_TRANSPARENT_DRAG);

    menuBar_ = new wxMenuBar();

    wxMenu* filemenu = new wxMenu();
    filemenu->Append(ID_BUILDDATA, uiLoc("Build Game Data..."));
    filemenu->Append(ID_INPUTDATA, uiLoc("Set Game Data Input Directory..."));
    filemenu->Append(ID_OUTPUTDATA, uiLoc("Set Game Data Output Directory..."));

    menuBar_->Append(filemenu, uiLoc("&File"));

    SetMenuBar(menuBar_);

    auto* renderFrame = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(1280, 720));

    {
        wxAuiPaneInfo paneinfo;
        paneinfo.Caption(uiLoc("ViewerMain"));
        paneinfo.CaptionVisible(true);
        paneinfo.CenterPane();
        auiManager_->AddPane(renderFrame, paneinfo);
    }

    consoleLog_ = new ConsoleLog(renderFrame, auiManager_);
    PkgViewer* packageViewer = new PkgViewer(renderFrame, auiManager_, menuBar_);

    renderFrame->AddPage(consoleLog_, uiLoc("Console"));
    renderFrame->AddPage(packageViewer, uiLoc("Pkg Viewer"));

    auiManager_->Update();

    exitSignal_.store(false);

    timer_.start();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::evtClose( wxCloseEvent& evt )
{
    timer_.Stop();
    evt.Skip();
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

void ViewerMainFrame::consoleMsgCallback(const char* msg, uint len, void* in_this_ptr)
{
    // Can't go to control here because this callback happens on any thread
    // so buffer the result
    ViewerMainFrame* this_ptr = (ViewerMainFrame*)in_this_ptr;
    wxCommandEvent evt(wxEVT_CONSOLE_STRING, wxID_ANY);
    evt.SetString(msg);
    this_ptr->consoleLog_->GetEventHandler()->AddPendingEvent(evt);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::consoleInput(const char* msg)
{
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

void ViewerMainFrame::luaThread(ViewerMainFrame* arg) {
    lua_State* L = luaL_newstate();

    while (!arg->exitSignal_) {

    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::evtDoDataBuild(wxCommandEvent& evt) {
    if (build::isBuildingData()) {
        wxMessageDialog msg(this, uiLoc("Build is already running.\nPlease wait for it to complete."));
        msg.ShowModal();
        return;
    }
    boost::system::error_code error;
    if (boost::filesystem::is_directory(dataPath_, error) == false) {
        evtDoSelectDataInput(evt);
    }
    if (boost::filesystem::is_directory(dataPath_, error) && boost::filesystem::is_directory(outputPath_, error) == false) {
        evtDoSelectDataOutput(evt);
    }
    if (boost::filesystem::is_directory(dataPath_, error) && boost::filesystem::is_directory(outputPath_, error)) {
        build::beginDataBuild(dataPath_.generic_string(), outputPath_.generic_string(), &ViewerMainFrame::consoleMsgCallback, this);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::evtDoSelectDataInput(wxCommandEvent& evt) {
    wxDirDialog dir_select(this, uiLoc("Select Input Data Directory..."), "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dir_select.ShowModal() != wxCANCEL) {
        dataPath_ = dir_select.GetPath();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::evtDoSelectDataOutput(wxCommandEvent& evt) {
    wxDirDialog dir_select(this, uiLoc("Select Output Data Directory..."), "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dir_select.ShowModal() != wxCANCEL) {
        outputPath_ = dir_select.GetPath();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void RenderTimer::Notify()
{
}
