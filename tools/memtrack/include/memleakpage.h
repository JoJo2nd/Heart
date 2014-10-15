/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#ifndef MEMLEAKPAGE_H__
#define MEMLEAKPAGE_H__

#include "records.h"

class MemLog;
class MemLogMarker;

class MemLeakPage : public wxNotebookPage
{
public:
    MemLeakPage(wxWindow* parent, wxWindowID id);

    void updateMemLeaks(MemLog*);

private:

    typedef std::vector<AllocRecord>    AllocVectorType;

    //Events
    DECLARE_EVENT_TABLE();
    void evtListItemSelected(wxListEvent& evt);

    wxBoxSizer*      boxSizer_;
    wxListCtrl*      list_;
    wxStaticText*    callstackText_;
    AllocVectorType  leaksVector_;
};

class MemMarkerPage : public wxNotebookPage
{
public:
    MemMarkerPage(wxWindow* parent, wxWindowID id);

    void updateMarkerTree(MemLog*);

private:

    class MarkerTreeAllocRecord : public wxTreeItemData
    {
    public:
        AllocRecord record;
    };

    DECLARE_EVENT_TABLE();

    void updateMarkerTree(wxTreeItemId, MemLogMarker*);
    void onMarkerTreeItemSelect(wxTreeEvent& evt);

    wxTreeCtrl*      markerTree_;
    wxStaticText*    callstackText_;
};

#endif // MEMLEAKPAGE_H__