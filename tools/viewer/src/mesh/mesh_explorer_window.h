/********************************************************************

    filename:   mesh_explorer_window.h  
    
    Copyright (c) 19:1:2013 James Moran
    
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

#ifndef MESH_EXPLORER_WINDOW_H__
#define MESH_EXPLORER_WINDOW_H__

#include "wx/menu.h"
#include "wx/msgdlg.h"
#include "wx/aui/aui.h"
#include "boost/smart_ptr.hpp"

class MeshContainer;
class MeshLodLevel;
class vMenuIDProvider;
class vPackageSystem;
class wxListView;
class wxListEvent;
class wxStaticText;
class wxFilePickerCtrl;
class wxComboBox;
class wxCheckBox;
class wxTextCtrl;
class wxListCtrl;
class wxFileDirPickerEvent;

class MeshExplorerWindow : public wxPanel
{
public:
    MeshExplorerWindow(
        wxWindow* parent, 
        vMenuIDProvider* idProvider, 
        wxMenu* baseMenu);
    ~MeshExplorerWindow();

    static const wxAuiPaneInfo& getDefaultAuiPaneInfo() { return s_defaultPaneInfo; }

private:

    enum ExcludeFlags
    {
        eExcludeLodFilePicker   = 1 << 1,
        eExcludeLodSelect       = 1 << 2,
    };

    static wxAuiPaneInfo                s_defaultPaneInfo;
    //
    const MeshLodLevel* getCurrentLod() const;
    int                 getCurrentLodIdx() const;
    //events
    void onNew(wxCommandEvent& evt);
    void onLoad(wxCommandEvent& evt);
    void onSave(wxCommandEvent& evt);
    void onExport(wxCommandEvent& evt);
    void onSourceFileChange(wxFileDirPickerEvent& evt);
    void onMaterialBindChange(wxFileDirPickerEvent& evt);
    void onLodLevelSelect(wxCommandEvent& evt);
    void onAddLodClick(wxCommandEvent& evt);
    void onYZAxisSwapCheck(wxCommandEvent& evt);
    void refreshView(size_t selectedLod, size_t excludeFlags = 0);

    boost::shared_ptr< MeshContainer >  currentMesh_;
    wxFilePickerCtrl*                   materialBindingPicker_;
    wxFilePickerCtrl*                   lodFilePick_;
    wxComboBox*                         lodLevelDropdown_;
    wxButton*                           addLodLevelBtn_;
    wxTextCtrl*                         meshInfoText_;
    wxCheckBox*                         yzAxisSwapCheck_;
};

#endif // MESH_EXPLORER_WINDOW_H__