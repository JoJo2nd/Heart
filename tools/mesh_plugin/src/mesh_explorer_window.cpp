/********************************************************************

    filename:   mesh_explorer_window.cpp  
    
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

#include "plugin_defines.h"
#include "mesh_explorer_window.h"
#include "mesh_container.h"
#include "mesh_lod_level.h"
#include "wx/generic/statbmpg.h"
#include "wx/listctrl.h"
#include "wx/stattext.h"
#include "wx/filepicker.h"
#include "wx/combobox.h"
#include "wx/checkbox.h"
#include "wx/choicdlg.h"
#include "wx/progdlg.h"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "wx/msgdlg.h"
#include "viewer_api.h"
#include <fstream>
#include <sstream>
#include "material_utils.h"

wxAuiPaneInfo MeshExplorerWindow::s_defaultPaneInfo;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MeshExplorerWindow::MeshExplorerWindow(wxWindow* parent, vMenuIDProvider* idProvider, wxMenu* baseMenu, vPackageSystem* pkgSys) 
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL)
{
    s_defaultPaneInfo.Name("Mesh Inspector");
    s_defaultPaneInfo.Caption("Mesh Inspector");
    s_defaultPaneInfo.Float();
    s_defaultPaneInfo.Floatable(true);
    s_defaultPaneInfo.CloseButton(true);
    s_defaultPaneInfo.MaximizeButton(true);
    s_defaultPaneInfo.MinimizeButton(true);
    s_defaultPaneInfo.CaptionVisible(true);
    s_defaultPaneInfo.Show(true);
    s_defaultPaneInfo.MinSize(400, 350); 

    pkgSystem_=pkgSys;

    packageDropdown_=new wxComboBox(
        this,
        idProvider->aquireMenuID(MENUID_PACKAGEDROPDOWN),
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        0,
        NULL,
        wxCB_DROPDOWN|wxCB_READONLY);
    resNameEntry_=new wxTextCtrl(
        this,
        idProvider->aquireMenuID(MENUID_RESNAMEENTRY),
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        0);//TODO: all caps validator?
    resNameEntry_->SetMaxLength(30);
    lodFilePick_=new wxFilePickerCtrl(
        this, 
        idProvider->aquireMenuID(MENUID_LODFILEPICKER), 
        "", 
        "Select Mesh File...", 
        "*.dae;*.obj;*.md5;*.3ds;*.ase;*.pk3;*.blend", 
        wxDefaultPosition, 
        wxSize(500,-1), 
        wxFLP_SMALL|wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL);
    lodLevelDropdown_=new wxComboBox(
        this,
        idProvider->aquireMenuID(MENUID_LODLEVELDROPDOWN),
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        0,
        NULL,
        wxCB_DROPDOWN|wxCB_READONLY);
    addLodLevelBtn_=new wxButton(
        this, 
        idProvider->aquireMenuID(MENUID_ADDLODLEVEL), 
        "Add Extra LOD Level");
    meshNodeList_=new wxListView(
        this,
        idProvider->aquireMenuID(MENUID_MESHNODELIST));
    meshNodeList_->InsertColumn(0, "LOD Meshes");
    matList_=new wxListView(
        this,
        idProvider->aquireMenuID(MENUID_MATLIST));
    meshNodeList_->InsertColumn(0, "All Materials in IMF");
    matSelectDropdown_=new wxComboBox(
        this,
        idProvider->aquireMenuID(MENUID_MATSELECTDROPDOWN),
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        0,
        NULL,
        wxCB_DROPDOWN|wxCB_READONLY);

    wxBoxSizer* topSizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* packageSizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* lodSizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* lodPickSizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* matPickSizer=new wxBoxSizer(wxHORIZONTAL);
    packageSizer->Add(packageDropdown_,  4, wxSTRETCH_NOT);
    packageSizer->Add(resNameEntry_, 6,     wxSTRETCH_NOT);
    lodSizer->Add(lodLevelDropdown_, 1, wxSTRETCH_NOT);
    lodSizer->Add(new wxStaticText(this, wxID_ANY, "LOD LEVEL"), 1, wxALL|wxSHAPED);
    lodSizer->Add(addLodLevelBtn_, 1, wxSTRETCH_NOT);
    lodPickSizer->Add(lodFilePick_, 1, wxSTRETCH_NOT);
    matPickSizer->Add(matSelectDropdown_, 1, wxSTRETCH_NOT);
    matPickSizer->Add(new wxStaticText(this, wxID_ANY, "Replacement Material"), 1, wxALL|wxSHAPED);
    topSizer->Add(packageSizer, 5, wxALL|wxEXPAND);
    topSizer->AddSpacer(5);
    topSizer->Add(lodSizer, 5, wxALL|wxEXPAND);
    topSizer->AddSpacer(5);
    topSizer->Add(lodPickSizer, 5, wxSTRETCH_NOT);
    topSizer->AddSpacer(5);
    topSizer->Add(new wxStaticText(this, wxID_ANY, "Mesh Nodes"), 1, wxALL|wxSHAPED);
    topSizer->Add(meshNodeList_, 30, wxALL|wxEXPAND);
    topSizer->AddSpacer(5);
    topSizer->Add(new wxStaticText(this, wxID_ANY, "Mesh Materials"), 1, wxALL|wxSHAPED);
    topSizer->Add(matList_, 30, wxALL|wxEXPAND);
    topSizer->AddSpacer(5);
    topSizer->Add(matPickSizer, 10, wxSTRETCH_NOT);

    SetSizer(topSizer);

    packageDropdown_->Bind(wxEVT_SET_FOCUS, &MeshExplorerWindow::onPkgSelectFocus, this, idProvider->getMenuID(MENUID_PACKAGEDROPDOWN));
    packageDropdown_->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED , &MeshExplorerWindow::onPkgSelectItem, this, idProvider->getMenuID(MENUID_PACKAGEDROPDOWN));
    lodFilePick_->Bind(wxEVT_COMMAND_FILEPICKER_CHANGED, &MeshExplorerWindow::onSourceFileChange, this, idProvider->getMenuID(MENUID_LODFILEPICKER));
    lodLevelDropdown_->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &MeshExplorerWindow::onLodLevelSelect, this, idProvider->getMenuID(MENUID_LODLEVELDROPDOWN));
    addLodLevelBtn_->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MeshExplorerWindow::onAddLodClick, this, idProvider->getMenuID(MENUID_ADDLODLEVEL));
    matSelectDropdown_->Bind(wxEVT_SET_FOCUS, &MeshExplorerWindow::onMatSelectFocus, this, idProvider->getMenuID(MENUID_MATSELECTDROPDOWN));
    matSelectDropdown_->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &MeshExplorerWindow::onMatSelectItem, this, idProvider->getMenuID(MENUID_MATSELECTDROPDOWN));
    resNameEntry_->Bind(wxEVT_COMMAND_TEXT_UPDATED, &MeshExplorerWindow::onResNameTextEntry, this, idProvider->getMenuID(MENUID_RESNAMEENTRY));
    matList_->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &MeshExplorerWindow::onMatListItemSelect, this, idProvider->getMenuID(MENUID_MATLIST));
    baseMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, &MeshExplorerWindow::onNew, this, idProvider->getMenuID(MENUID_NEW));
    baseMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, &MeshExplorerWindow::onLoad, this, idProvider->getMenuID(MENUID_LOAD));
    baseMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, &MeshExplorerWindow::onSave, this, idProvider->getMenuID(MENUID_SAVE));
    baseMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, &MeshExplorerWindow::onExport, this, idProvider->getMenuID(MENUID_EXPORT));

    wxArrayString selectionItems;
    std::vector<vResource*> matResources;
    matResID_=pkgSystem_->getTypeID("MFX");
    for(size_t i=0,n=pkgSystem_->getPackageCount(); i<n; ++i) {
        selectionItems.push_back(pkgSystem_->getPackage(i)->getName());
    }
    packageDropdown_->Set(selectionItems);
    packageDropdown_->Select(0);
    pkgSystem_->getResourcesOfType(matResID_, &matResources);
    selectionItems.clear();
    for (size_t i=0,n=matResources.size(); i<n; ++i) {
        selectionItems.push_back(matResources[i]->getFullAssetName());
    }
    matSelectDropdown_->Set(selectionItems);

    if (!currentMesh_) {
        packageDropdown_->Enable(false);
        resNameEntry_->Enable(false);
        lodFilePick_->Enable(false);
        lodLevelDropdown_->Enable(false);
        addLodLevelBtn_->Enable(false);
        meshNodeList_->Enable(false);
        matList_->Enable(false);
        matSelectDropdown_->Enable(false);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MeshExplorerWindow::~MeshExplorerWindow() {

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::onNew(wxCommandEvent& evt) {
    wxBusyCursor waitCursor;
    currentMesh_=boost::shared_ptr<MeshContainer>(new MeshContainer());
    currentMesh_->appendNewLodLevel();
    refreshView(0);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::onLoad(wxCommandEvent& evt) {
    wxFileDialog openFileDialog(this, _("Open IMF file"), "", "",
        "IMF files (*.imf)|*.imf", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxBusyCursor waitCursor;
    currentMesh_=boost::shared_ptr<MeshContainer>(new MeshContainer());
    std::ifstream ifs(openFileDialog.GetPath().ToStdString());
    boost::archive::text_iarchive ia(ifs);
    MeshContainer& tdb = *currentMesh_;
    ia >> tdb;
    refreshView(0);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::onSave(wxCommandEvent& evt) {
    wxFileDialog saveFileDialog(this, _("Save IMF file"), "", "",
        "IMF files (*.imf)|*.imf", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxBusyCursor waitCursor;
    std::ofstream ofs(saveFileDialog.GetPath().ToStdString());
    boost::archive::text_oarchive oa(ofs);
    const MeshContainer& tdb = *currentMesh_;
    oa << tdb;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::onExport(wxCommandEvent& evt) {
    if (currentMesh_) {
        vPackage* pkg=pkgSystem_->getPackage(currentMesh_->getExportPackageName().c_str());
        if (!pkg) {
            wxMessageDialog(this, "Failed to file package to export to", "ERROR").ShowModal();
            return;
        }
        vResource* res=pkg->getResourceInfoByName(currentMesh_->getExportResourceName().c_str());
        if (res) {
            wxString msg;
            msg.Printf("This will overwrite resource %s.%s?", 
                currentMesh_->getExportPackageName().c_str(), currentMesh_->getExportResourceName().c_str());
            if (wxMessageDialog(this, msg, "Are You Sure?", wxYES_NO).ShowModal() == wxID_NO) {
                return;
            }
            
            wxFileDialog saveFileDialog(this, _("Export MDF file"), "", "",
                "MDF files (*.mdf)|*.mdf", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
            if (saveFileDialog.ShowModal() == wxID_CANCEL) {
                return;
            }
            wxBusyCursor waitingCursor;
            MeshExportResult exRes=currentMesh_->exportToMDF(saveFileDialog.GetPath().ToStdString(), pkgSystem_);
            if (!exRes.exportOK) {
                wxMessageDialog(this, exRes.errors, "Export Faild?").ShowModal();
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::refreshView(size_t selectedLod, size_t excludeFlags/* = 0*/) {
    if (!currentMesh_) {
        packageDropdown_->Enable(false);
        resNameEntry_->Enable(false);
        lodFilePick_->Enable(false);
        lodLevelDropdown_->Enable(false);
        addLodLevelBtn_->Enable(false);
        meshNodeList_->Enable(false);
        matList_->Enable(false);
        matSelectDropdown_->Enable(false);
        return;
    }
    wxArrayString selectionItems;
    std::vector<vResource*> matResources;
    wxString itemStr;
    const MeshLodLevel* lodLevel;

    packageDropdown_->Enable(true);
    resNameEntry_->Enable(true);
    lodFilePick_->Enable(true);
    lodLevelDropdown_->Enable(true);
    addLodLevelBtn_->Enable(true);
    meshNodeList_->Enable(true);
    matList_->Enable(true);
    matSelectDropdown_->Enable(true);

    selectionItems.clear();
    for (size_t i=0,n=currentMesh_->getLodCount(); i<n; ++i) {
        itemStr.clear();
        itemStr << i;
        selectionItems.push_back(itemStr);
    }
    if (!(excludeFlags&eExcludeLodSelect)){
        lodLevelDropdown_->Set(selectionItems);
        if (selectedLod >= currentMesh_->getLodCount()) {
            selectedLod=currentMesh_->getLodCount()-1;
        }
        lodLevelDropdown_->Select(selectedLod);
    }
    lodLevel=&currentMesh_->getLodLevel(selectedLod);

    if (!(excludeFlags&eExcludeLodFilePicker)) {
        lodLevel=&currentMesh_->getLodLevel(selectedLod);
        wxFileName filename;
        filename.SetFullName(lodLevel->getSceneName());
        lodFilePick_->SetFileName(filename);
    }

    meshNodeList_->DeleteAllItems();
    const aiScene* scene=lodLevel->getScene();
    size_t count=0;
    if (scene) {
        for (size_t i=0,n=scene->mNumMeshes; i<n; ++i) {
            meshNodeList_->InsertItem(0, scene->mMeshes[i]->mName.C_Str());
        }
    }

    matList_->ClearAll();
    matList_->AppendColumn("Mesh Materials");
    count=0;
    for (MaterialRemap::const_iterator i=currentMesh_->getMaterialRemap().begin(),n=currentMesh_->getMaterialRemap().end(); 
        i!=n; ++i, ++count) {
        matList_->InsertItem(0, i->first);
    }
    matList_->SetColumnWidth(0, wxLIST_AUTOSIZE);

    packageDropdown_->SetSelection(packageDropdown_->FindString(currentMesh_->getExportPackageName()));
    resNameEntry_->ChangeValue(currentMesh_->getExportResourceName());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::onPkgSelectFocus(wxFocusEvent& evt) {
    evt.Skip();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::onPkgSelectItem(wxCommandEvent& evt) {
    currentMesh_->setExportPackageName(evt.GetString());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::onSourceFileChange(wxFileDirPickerEvent& evt) {
    const MeshLodLevel* lodlevel=getCurrentLod();
    if (!lodlevel) return;

    wxBusyCursor waitCursor;
    currentMesh_->importMeshIntoLod(getCurrentLodIdx(), evt.GetPath().ToStdString());
    refreshView(getCurrentLodIdx(), eExcludeLodFilePicker);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::onLodLevelSelect(wxCommandEvent& evt) {
    refreshView(evt.GetSelection(), eExcludeLodSelect);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::onMatSelectFocus(wxFocusEvent& evt) {
    evt.Skip();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::onMatSelectItem(wxCommandEvent& evt) {
    std::vector< std::string > toreassign;
    int selcnt=matList_->GetSelectedItemCount();
    for(int sel=matList_->GetFirstSelected(); sel>=0; sel=matList_->GetNextSelected(sel)) {
        toreassign.push_back(matList_->GetItemText(sel).ToStdString());
    }
    currentMesh_->setMaterialRemap(toreassign, evt.GetString().ToStdString());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::onAddLodClick(wxCommandEvent& evt) {
    currentMesh_->appendNewLodLevel();
    refreshView(getCurrentLodIdx());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::onResNameTextEntry(wxCommandEvent& evt) {
    currentMesh_->setExportResourceName(resNameEntry_->GetValue().c_str());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MeshExplorerWindow::onMatListItemSelect(wxListEvent& evt) {
    std::string str=matList_->GetItemText(evt.GetIndex()).ToStdString();
    MaterialRemap::const_iterator itr=currentMesh_->getMaterialRemap().find(str);
    if (itr!=currentMesh_->getMaterialRemap().end()) {
        matSelectDropdown_->SetSelection(matSelectDropdown_->FindString(itr->second));
    } else {
        matSelectDropdown_->SetSelection(wxNOT_FOUND);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int MeshExplorerWindow::getCurrentLodIdx() const {
    int selection=lodLevelDropdown_->GetCurrentSelection();
    if ((size_t)selection>currentMesh_->getLodCount()) {
        selection=0;
    }
    return selection;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const MeshLodLevel* MeshExplorerWindow::getCurrentLod() const {
    return &currentMesh_->getLodLevel(getCurrentLodIdx());
} 