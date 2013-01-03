/********************************************************************

    filename:   consolelog.cpp  
    
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
#include "consolelog.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(ConsoleLog, wxPanel)
    EVT_BUTTON(cuiID_CONSOLESUMBIT, ConsoleLog::evtConsoleSubmit)
    EVT_SIZE(ConsoleLog::evtResize)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ConsoleLog::~ConsoleLog()
{
    outputConn_.disconnect();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ConsoleLog::initFrame()
{
    goodSize_ = wxSize(~0, ~0);

    mainSizer_ = new wxFlexGridSizer(2, 1, 0, 0);
    mainSizer_->AddGrowableRow(0);
    mainSizer_->AddGrowableCol(0);
    lowerSizer_ = new wxFlexGridSizer(1, 2, 0, 0);
    lowerSizer_->AddGrowableRow(0);
    lowerSizer_->AddGrowableCol(0);

    logTextCtrl_ = new wxTextCtrl(this, cuiID_CONSOLELOG, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_RICH2 | wxHSCROLL | wxTE_READONLY);
    inputCtrl_ = new wxTextCtrl(this, cuiID_CONSOLEINPUT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_RICH2 | wxHSCROLL);
    submitButton_ = new wxButton(this, cuiID_CONSOLESUMBIT, "Submit");

    lowerSizer_->Add(inputCtrl_, 5, wxALL | wxEXPAND);
    lowerSizer_->Add(submitButton_, 0, wxALL | wxFIXED_MINSIZE | wxEXPAND);
    lowerSizer_->SetSizeHints(this);

    mainSizer_->Add(logTextCtrl_, 3, wxALL | wxALIGN_TOP | wxEXPAND);
    mainSizer_->Add(lowerSizer_, 0, wxALL | wxALIGN_BOTTOM | wxEXPAND);
    mainSizer_->SetSizeHints(this);

    SetSizerAndFit(mainSizer_);
    SetMinSize(goodSize_ = GetSize());

    outputConn_ = evt_consoleOutputSignal.connect(boost::bind(&ConsoleLog::consoleOutputString, this, _1));

    wxAuiPaneInfo paneinfo;
    paneinfo.Name("Console Window");
    paneinfo.Caption("Console Window");
    paneinfo.Float();
    paneinfo.Floatable(true);
    paneinfo.CloseButton(true);
    paneinfo.CaptionVisible(true);
    paneinfo.MinSize(300, 200);

    evt_registerAuiPane(this, "Console Window", paneinfo);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ConsoleLog::consoleOutputString(const hChar* msg)
{
    logTextCtrl_->AppendText(msg);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ConsoleLog::evtConsoleSubmit(wxCommandEvent& event)
{
    wxString inputStr;
    for (uint i = 0, c = inputCtrl_->GetNumberOfLines(); i < c; ++i) {
        inputStr += inputCtrl_->GetLineText(i);
        inputStr += "\n";
    }
    if (inputStr.length() > 1) {
        evt_consoleInputSignal(inputStr.c_str());
    }
    inputCtrl_->Clear();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ConsoleLog::evtResize(wxSizeEvent& evt)
{
    evt.Skip();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ConsoleLog::logString(const char* channel, const char* msg, ...)
{
    char buffer1[2046],buffer2[4096];

    va_list marker;
    va_start(marker, msg );
    vsnprintf_s(buffer1, 2046, msg, marker);

    _snprintf(buffer2, 4096, "[%s] %s\n", channel, buffer1);
    evt_consoleOutputSignal(buffer2);
    va_end( marker );
}
