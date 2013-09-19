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

IMPLEMENT_APP(ViewerApp);

static ViewerMainFrame* g_mainFrame;

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

void ViewerMainFrame::initFrame(const wxString& heartpath, const wxString& pluginPaths)
{
    g_mainFrame = this;
    auiManager_ = new wxAuiManager(this, wxAUI_MGR_DEFAULT | wxAUI_MGR_TRANSPARENT_DRAG);

    menuBar_ = new wxMenuBar();

    wxMenu* filemenu = new wxMenu();
    filemenu->AppendSeparator();
    filemenu->Append(cuiID_SHOWCONSOLE, "Show &Console");

    menuBar_->Append(filemenu, "&File");

    SetMenuBar(menuBar_);

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

#if 0
    hHeartEngineCallbacks callbacks = {0};
    callbacks.overrideFileRoot_ = NULL; 
    if (heartpath.length()>0) {
        callbacks.overrideFileRoot_ = heartpath.c_str();
        dataPath_ = heartpath.ToStdWstring();
    }
    else {
        wxDirDialog dirselector(NULL, "Selete Game Running Directory");
        if (dirselector.ShowModal() == wxID_OK) {
            dataPath_ = dirselector.GetPath();
        } else {
            dataPath_ = boost::filesystem::current_path();
        }
    }
    pathString_ = dataPath_.generic_string();
    callbacks.overrideFileRoot_ = pathString_.c_str();
    callbacks.consoleCallback_ = &ViewerMainFrame::consoleMsgCallback;
    callbacks.consoleCallbackUser_ = this;
    callbacks.mainRender_ = &ViewerMainFrame::renderCallback;
    heart_ = hHeartInitEngine(&callbacks, wxGetInstance(), renderFrame->GetHWND());


    Heart::hRenderer* renderer = heart_->GetRenderer();
    Heart::hRenderMaterialManager* matMgr=renderer->GetMaterialManager();
    hUint32 w = renderer->GetWidth();
    hUint32 h = renderer->GetHeight();
    hFloat aspect = (hFloat)w/(hFloat)h;
    Heart::hRenderViewportTargetSetup rtDesc={0};
    Heart::hTexture* bb=matMgr->getGlobalTexture("back_buffer");
    Heart::hTexture* db=matMgr->getGlobalTexture("depth_buffer");
    Heart::hTextureFormat dfmt=Heart::eTextureFormat_D32_float;
    Heart::hRenderTargetView* rtv=NULL;
    Heart::hDepthStencilView* dsv=NULL;
    Heart::hRenderTargetViewDesc rtvd;
    Heart::hDepthStencilViewDesc dsvd;
    hZeroMem(&rtvd, sizeof(rtvd));
    hZeroMem(&dsvd, sizeof(dsvd));
    rtvd.format_=bb->getTextureFormat();
    rtvd.resourceType_=bb->getRenderType();
    //hcAssert(bb->getRenderType()==eRenderResourceType_Tex2D);
    rtvd.tex2D_.topMip_=0;
    rtvd.tex2D_.mipLevels_=~0;
    dsvd.format_=dfmt;
    dsvd.resourceType_=db->getRenderType();
    //hcAssert(db->getRenderType()==eRenderResourceType_Tex2D);
    dsvd.tex2D_.topMip_=0;
    dsvd.tex2D_.mipLevels_=~0;
    renderer->createRenderTargetView(bb, rtvd, &rtv);
    renderer->createDepthStencilView(db, dsvd, &dsv);
    rtDesc.nTargets_=1;
    rtDesc.targetTex_=bb;
    rtDesc.targets_[0]=rtv;
    rtDesc.depth_=dsv;

    Heart::hRelativeViewport vp;
    vp.x= 0.f;
    vp.y= 0.f;
    vp.w= 1.f;
    vp.h= 1.f;
    Heart::hMatrix vm = Heart::hMatrixFunc::identity();;
    camera_.Initialise(renderer);
    camera_.bindRenderTargetSetup(rtDesc);
    camera_.SetFieldOfView(45.f);
    camera_.SetOrthoParams(-1.f, 1.f, 1.f, -1.f, 0.f, 1000.f);
    camera_.SetViewMatrix(vm);
    camera_.setViewport(vp);
    camera_.SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("main"));

    // The camera hold refs to this
    rtv->DecRef();
    dsv->DecRef();
#endif

    timer_.start();

    moduleSystem_.initialiseAndLoadPlugins(
        auiManager_, 
        this, 
        menuBar_, 
        pluginPaths.ToStdString(),
        boost::filesystem::current_path());

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ViewerMainFrame::evtClose( wxCloseEvent& evt )
{
    timer_.Stop();
    moduleSystem_.shutdown();

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

void ViewerMainFrame::consoleMsgCallback(const char* msg, void* user)
{
    // Can't go to control here because this callback happens on any thread
    // so buffer the result
    ((ViewerMainFrame*)user)->timer_.flushConsoleText(msg);
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

void RenderTimer::Notify()
{
}
