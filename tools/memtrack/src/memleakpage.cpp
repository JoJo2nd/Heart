/********************************************************************

	filename: 	memleakpage.cpp	
	
	Copyright (c) 19:11:2012 James Moran
	
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
#include "memleakpage.h"
#include "memlog.h"

BEGIN_EVENT_TABLE(MemLeakPage, wxNotebookPage)
    EVT_LIST_ITEM_SELECTED(uiID_GLOBALLEAKLIST, MemLeakPage::evtListItemSelected)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MemLeakPage::MemLeakPage(wxWindow* parent, wxWindowID id) 
    : wxNotebookPage(parent, id)
{
    boxSizer_ = new wxBoxSizer(wxHORIZONTAL);
    list_ = new wxListCtrl(this, uiID_GLOBALLEAKLIST, wxDefaultPosition, wxDefaultSize, wxLC_SINGLE_SEL|wxLC_REPORT);
    callstackText_ = new wxStaticText(this, uiID_CALLSTACK_TEXT, "---");

    boxSizer_->Add(list_, 55, wxEXPAND|wxALL);
    boxSizer_->Add(callstackText_, 45, wxEXPAND|wxALL);

    SetSizer(boxSizer_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLeakPage::updateMemLeaks(MemLog* memlog)
{
    leaksVector_.clear();
    list_->ClearAll();
    list_->InsertColumn(0,"Leaks...");
//     list_->InsertColumn(1,"File (line)");
//     list_->InsertColumn(2,"Size");
    
    memlog->getAllMemoryLeaks(&leaksVector_);
    for (uint i = 0, c = leaksVector_.size(); i < c; ++i)
    {
        wxListItem it1,it2,it3;
        wxString ittxt,txt;
        
        ittxt.Printf("0x%016LLX - ", leaksVector_[i].address_);
        txt = ittxt;
        ittxt.Printf("%s [%u] - ", leaksVector_[i].backtrace_.sourcePath_.c_str(), leaksVector_[i].backtrace_.line_);
        txt += ittxt;
        ittxt.Printf("%llu", leaksVector_[i].size_);
        txt += ittxt;
        it1.SetText(txt);
        it1.SetId(i);
        it1.SetColumn(0);
        list_->InsertItem(it1);
    }

    list_->SetColumnWidth(0, wxLIST_AUTOSIZE);
   
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLeakPage::evtListItemSelected(wxListEvent& evt)
{
    uint id = evt.GetItem().GetId();

    if (id < leaksVector_.size()) {
        wxString ittxt,txt = "Leak Callstack...\n";
        std::string sym;

        ittxt.Printf("Alloc Called at := %s [%u]\n", leaksVector_[id].backtrace_.sourcePath_.c_str(), leaksVector_[id].backtrace_.line_);
        txt += ittxt;

        for (uint i = leaksVector_[id].backtrace_.backtrace_.size()-1, c = leaksVector_[id].backtrace_.backtrace_.size(); i < c; --i)
        {
            bool found = Callstack::addressSymbolLookup(leaksVector_[id].backtrace_.backtrace_[i], &sym);
            ittxt.Printf("%u) %s\n", i, found ? sym.c_str() : "Missing Symbol");
            txt += ittxt;
        }

        callstackText_->SetLabel(txt);
    } else {
        callstackText_->SetLabel("---");
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MemMarkerPage, wxNotebookPage)
    EVT_TREE_SEL_CHANGED(uiID_MARKER_PAGE_TREE, MemMarkerPage::onMarkerTreeItemSelect)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MemMarkerPage::MemMarkerPage(wxWindow* parent, wxWindowID id) 
    : wxNotebookPage(parent, id)
{
    wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
    markerTree_ = new wxTreeCtrl(this, uiID_MARKER_PAGE_TREE);
    callstackText_ = new wxStaticText(this, uiID_MARKER_PAGE_CALLSTACK_TEXT, "---");

    boxSizer->Add(markerTree_, 60, wxEXPAND|wxALL);
    boxSizer->Add(callstackText_, 40, wxEXPAND|wxALL);

    SetSizer(boxSizer);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemMarkerPage::updateMarkerTree(MemLog* memlog) {
    markerTree_->DeleteAllItems();
    
    wxTreeItemId rootItemId = markerTree_->AddRoot("MemoryAllocs");
    for (auto i=memlog->getFirstMarker(), n=memlog->getLastMarker(); i!=n; ++i) {
        updateMarkerTree(rootItemId, (*i));
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemMarkerPage::updateMarkerTree(wxTreeItemId id, MemLogMarker* marker) {
    std::vector<AllocRecord> leakRecords;
    wxTreeItemId markerItemId = markerTree_->AppendItem(id, marker->getName());
    marker->getAliveAllocsExclusive(&leakRecords);

    for (size_t i=0, n=leakRecords.size(); i<n; ++i) {
        const AllocRecord& ar=leakRecords[i];
        wxString ittxt,txt;

        ittxt.Printf("0x%016LLX - ", ar.address_);
        txt = ittxt;
        ittxt.Printf("%s [%u] - ", ar.backtrace_.sourcePath_.c_str(), ar.backtrace_.line_);
        txt += ittxt;
        ittxt.Printf("%llu", ar.size_);
        txt += ittxt;

        MarkerTreeAllocRecord* itemdata=new MarkerTreeAllocRecord();
        itemdata->record=ar;
        markerTree_->AppendItem(markerItemId, txt, -1, -1, itemdata);
    }


    for (uint ic=0, nc=marker->getChildCount(); ic<nc; ++ic) {
        updateMarkerTree(markerItemId, marker->getChild(ic));
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemMarkerPage::onMarkerTreeItemSelect(wxTreeEvent& evt) {
    MarkerTreeAllocRecord* mtar=static_cast<MarkerTreeAllocRecord*>(markerTree_->GetItemData(evt.GetItem()));
    if (mtar) {
        wxString ittxt,txt = "Leak Callstack...\n";
        std::string sym;

        ittxt.Printf("Alloc Called at := %s [%u]\n", mtar->record.backtrace_.sourcePath_.c_str(), mtar->record.backtrace_.line_);
        txt += ittxt;

        for (uint i = mtar->record.backtrace_.backtrace_.size()-1, c = mtar->record.backtrace_.backtrace_.size(); i < c; --i)
        {
            bool found = Callstack::addressSymbolLookup(mtar->record.backtrace_.backtrace_[i], &sym);
            ittxt.Printf("%u) %s\n", i, found ? sym.c_str() : "Missing Symbol");
            txt += ittxt;
        }

        callstackText_->SetLabel(txt);
    } else {
        callstackText_->SetLabel("---");
    }
}
