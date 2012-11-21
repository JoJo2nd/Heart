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
    EVT_CLOSE(MemTrackMainFrame::evtClose)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemTrackMainFrame::initFrame()
{
    wxMenuBar* menubar = new wxMenuBar();

    wxMenu* filemenu = new wxMenu();
    filemenu->Append(wxID_OPEN, "&Open");
    filemenu->Append(wxID_SAVE, "&Save");
    filemenu->Append(wxID_SAVEAS, "Save &As");
    filemenu->Append(uiID_IMPORT, "&Import Memory Log");

    menubar->Append(filemenu, "&File");

    SetMenuBar(menubar);

    wxNotebook* notebook = new wxNotebook(this, wxID_ANY);

    leakListPage_ = new MemLeakPage(notebook, wxID_ANY);
    notebook->InsertPage(0, leakListPage_, "Leaks");
    notebook->InsertPage(1, new wxNotebookPage(notebook, wxID_ANY), "Markers");
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemTrackMainFrame::evtClose( wxCloseEvent& evt )
{
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
}
