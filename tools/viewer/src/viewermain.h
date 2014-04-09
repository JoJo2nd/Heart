/********************************************************************

    filename:   viewermain.h  
    
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
    void            evtOnPaneClose(wxAuiManagerEvent& evt);
    void            evtClose(wxCloseEvent& evt);

    wxAuiManager*           auiManager_;
    ConsoleLog*             consoleLog_;
    wxFileConfig            fileConfig_;
    wxFileHistory           fileHistory_;
    wxMenuBar*              menuBar_;
    RenderTimer             timer_;
    boost::filesystem::path dataPath_;
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
