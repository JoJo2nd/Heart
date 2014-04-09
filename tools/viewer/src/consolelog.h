/********************************************************************

    filename:   consolelog.h  
    
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
DECLARE_EVENT_TYPE(wxEVT_CONSOLE_STRING, -1)

#endif // CONSOLELOG_H__