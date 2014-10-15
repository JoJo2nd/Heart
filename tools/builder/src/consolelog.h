/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#ifndef CONSOLELOG_H__
#define CONSOLELOG_H__

class ConsoleLog : public wxPanel
{
public:
    ConsoleLog(wxWindow* parent, wxAuiManager* auiManager) 
        : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(400, 400))
        , auiManager_(auiManager)
    {
        initFrame();
    }
    ~ConsoleLog();

private:
    void            consoleOutputString(const char* msg);
    void            initFrame();

    void            evtConsoleSubmit(wxCommandEvent& event);
    void            evtConsoleLog(wxCommandEvent& event);
    void            evtResize(wxSizeEvent& evt);

    DECLARE_EVENT_TABLE();

    wxAuiManager*       auiManager_;
    wxTextCtrl*         logTextCtrl_;
    wxTextCtrl*         inputCtrl_;
    wxButton*           submitButton_;
    wxFlexGridSizer*    mainSizer_;
    wxFlexGridSizer*    lowerSizer_;
    boost::signals2::connection outputConn_;
    wxSize              goodSize_;
};

// how to declare a custom event. this can go in a header
DECLARE_LOCAL_EVENT_TYPE(wxEVT_CONSOLE_STRING, -1)

#endif // CONSOLELOG_H__