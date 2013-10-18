/********************************************************************

    filename:   texture_management_window.h  
    
    Copyright (c) 4:1:2013 James Moran
    
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

#ifndef TEXTURE_MANAGEMENT_WINDOW_H__
#define TEXTURE_MANAGEMENT_WINDOW_H__

#include "wx/menu.h"
#include "wx/msgdlg.h"
#include "wx/aui/aui.h"

class vMenuIDProvider;
class vPackageSystem;
class vPackage;
class wxListView;
class wxListEvent;
class wxStaticText;
class wxFilePickerCtrl;
class wxComboBox;
class wxCheckBox;
class wxGenericStaticBitmap;
class TextureDataStore;
class TextureDBItem;

class TextureManagementWindow : public wxPanel
{
public:
    TextureManagementWindow(
            wxWindow* parent, 
            vMenuIDProvider* idProvider, 
            wxMenu* baseMenu, 
            TextureDataStore* texDB, 
            const char* textureDatabasePath)
        : wxPanel(parent, wxID_ANY)
        , texDB_(texDB)
        , textureDatabasePath_(textureDatabasePath)
        , resourceTextValidator_(wxFILTER_INCLUDE_CHAR_LIST)
    {
        initFrame(idProvider, baseMenu);
    }
    ~TextureManagementWindow() 
    {}

    static const wxAuiPaneInfo& getDefaultAuiPaneInfo() { return s_defaultPaneInfo; }
    void updateView(const TextureDataStore& texDB);

private:

    void initFrame(vMenuIDProvider* idProvider, wxMenu* baseMenu);
    void updateControlView(const TextureDBItem& it);
    void buildTextures(TextureDBItem** textures, size_t count);
    void addTextureResource(vPackage* pkg, const wxString& filename);
    void doTextureDatabaseSave();

    //events
    void evtSaveDB(wxCommandEvent& evt);
    void evtExportDB(wxCommandEvent& evt);
    void evtBuild(wxCommandEvent& evt);
    void evtBuildSingle(wxCommandEvent& evt);
    void evtSelected(wxListEvent& evt);
    void evtDeselected(wxListEvent& evt);
    void evtApply(wxCommandEvent& evt);

    void updateSelection();
    void updateDataStoreItem(uint64 dataptr, bool multiupdate);

    void evtConvertCheck(wxCommandEvent& evt);
    void evtDropFiles(wxDropFilesEvent& evt);
    void evtKeyDown(wxListEvent& evt);
    void evtSearchEvent(wxCommandEvent& evt);


    static wxAuiPaneInfo    s_defaultPaneInfo;
    static wxString         s_formatEnumStrs[];
    static wxString         s_qualityEnumStrs[];
    static wxString         s_wrapModeEnumStrs[];
    static wxString         s_mipmapFilterEnumStrs[];
    static wxString         s_alphaModeEnum[];

    std::string             textureDatabasePath_;
    wxTextValidator         resourceTextValidator_;
    wxTextValidator         wildcardTextValidator_;
    TextureDataStore*       texDB_;
    wxListView*             texListView_;
    wxGenericStaticBitmap*  previewImage_;
    wxTextCtrl*             packageName_;
    wxTextCtrl*             resourceName_;
    wxTextCtrl*             packageSearch_;
    wxTextCtrl*             resourceSearch_;
    wxTextCtrl*             pathSearch_;
    wxButton*               applyButton_;
    wxFilePickerCtrl*       inputPathPicker_;
    wxFilePickerCtrl*       outputPathPicker_;
    wxComboBox*             formatSelect_;
    wxComboBox*             qualitySelect_;
    wxComboBox*             wrapSelect_;
    wxComboBox*             mipSelect_;
    wxComboBox*             alphaSelect_;
    wxCheckBox*             convertSelect_;
    wxCheckBox*             enableMipsSelect_;
    wxCheckBox*             enableNormalMapMode_;
    wxCheckBox*             enableGammaCorrection_;
    wxButton*               buildButton_;

    std::vector< boost::shared_ptr<TextureDBItem> > foundItems_;
    bool                    wasMultiselect_;
};

#endif // TEXTURE_MANAGEMENT_WINDOW_H__