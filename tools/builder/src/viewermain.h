/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#ifndef VIEWERMAIN_H__
#define VIEWERMAIN_H__

class RenderTimer : public wxTimer
{
public:
    RenderTimer()
        : wxTimer()
    {
    }
    void Notify();
    void start() {
        wxTimer::Start(10);
    }
    void flushConsoleText(const char* msg) {
        wxMutexLocker l(access_);
        consoleOutput_ += msg;
    }

private:
    wxString             consoleOutput_;
    wxMutex              access_;
};

class ConsoleLog;

class ViewerMainFrame : public wxFrame
{
public:
    ViewerMainFrame(const wxString& heartpath, const wxString& pluginPaths)
        : wxFrame(NULL, wxID_ANY, "Viewer", wxDefaultPosition, wxSize(1280, 720))
        , fileConfig_("Viewer", "", "viewer.cfg", "viewer.cfg")
    {
        initFrame(heartpath, pluginPaths);
    }
    ~ViewerMainFrame();

private:

    typedef std::map<wxString, wxString> SavedPaneInfo;

    static void luaThread(ViewerMainFrame* arg);
    static void consoleMsgCallback(const char* msg, uint len, void* this_ptr);
    //static void renderCallback(Heart::hHeartEngine* engine);
    void consoleInput(const char* msg);
    void initFrame(const wxString& heartpath, const wxString& pluginPaths);
    void dockPaneRegister(wxWindow* pane, const wxString&, const wxAuiPaneInfo&);

    //Events
    DECLARE_EVENT_TABLE();
    void            evtShowConsole(wxCommandEvent& evt);
    void            evtDoDataBuild(wxCommandEvent& evt);
    void            evtDoSelectDataInput(wxCommandEvent& evt);
    void            evtDoSelectDataOutput(wxCommandEvent& evt);
    void            evtOnPaneClose(wxAuiManagerEvent& evt);
    void            evtClose(wxCloseEvent& evt);

    wxAuiManager*           auiManager_;
    ConsoleLog*             consoleLog_;
    wxFileConfig            fileConfig_;
    wxFileHistory           fileHistory_;
    wxMenuBar*              menuBar_;
    RenderTimer             timer_;
    boost::filesystem::path dataPath_;
    boost::filesystem::path outputPath_;
    std::string             pathString_;
    SavedPaneInfo           paneSavedLayouts_;
    std::thread             luaThread_;
    std::atomic_bool        exitSignal_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class ViewerApp : public wxApp
{
public:

    bool OnInit();
    virtual void OnInitCmdLine(wxCmdLineParser& parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);

    wxString    heartPath_;
    wxString    pluginPaths_;
};
#endif // VIEWERMAIN_H__
