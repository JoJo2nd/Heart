/********************************************************************

	filename: 	memleakpage.h	
	
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