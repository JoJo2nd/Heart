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
class TextureDatabase;
class TextureDBItem;

class TextureManagementWindow : public wxPanel
{
public:
    TextureManagementWindow(
            wxWindow* parent, 
            vMenuIDProvider* idProvider, 
            wxMenu* baseMenu, 
            TextureDatabase* texDB, 
            vPackageSystem* pkgSys, 
            const char* textureDatabasePath)
        : wxPanel(parent, wxID_ANY)
        , texDB_(texDB)
        , pkgSys_(pkgSys)
        , textureDatabasePath_(textureDatabasePath)
    {
        initFrame(idProvider, baseMenu);
    }
    ~TextureManagementWindow() 
    {}

    static const wxAuiPaneInfo& getDefaultAuiPaneInfo() { return s_defaultPaneInfo; }
    void updateView(const TextureDatabase& texDB);

private:

    void initFrame(vMenuIDProvider* idProvider, wxMenu* baseMenu);
    void updateControlView(const TextureDBItem& it);
    void buildTextures(TextureDBItem** textures, size_t count);
    void addTextureResource(vPackage* pkg, const wxString& filename);
    void doTextureDatabaseSave();

    //events
    void evtSaveDB(wxCommandEvent& evt);
    void evtBuild(wxCommandEvent& evt);
    void evtBuildSingle(wxCommandEvent& evt);
    void evtSelected(wxListEvent& evt);
    void evtApply(wxCommandEvent& evt);
    void evtConvertCheck(wxCommandEvent& evt);
    void evtDropFiles(wxDropFilesEvent& evt);
    void evtKeyDown(wxListEvent& evt);


    static wxAuiPaneInfo    s_defaultPaneInfo;
    static wxString         s_formatEnumStrs[];
    static wxString         s_qualityEnumStrs[];
    static wxString         s_wrapModeEnumStrs[];
    static wxString         s_mipmapFilterEnumStrs[];
    static wxString         s_alphaModeEnum[];

    std::string             textureDatabasePath_;
    TextureDatabase*        texDB_;
    vPackageSystem*         pkgSys_;
    wxListView*             texListView_;
    wxGenericStaticBitmap*  previewImage_;
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
};

#endif // TEXTURE_MANAGEMENT_WINDOW_H__