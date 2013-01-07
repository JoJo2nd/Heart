/********************************************************************

    filename:   texture_management_window.cpp  
    
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

#include "plugin_defines.h"
#include "texture_management_window.h"
#include "texture_database.h"
#include "viewer_api.h"
#include "wx/generic/statbmpg.h"
#include "wx/listctrl.h"
#include "wx/stattext.h"
#include "wx/filepicker.h"
#include "wx/combobox.h"
#include "wx/checkbox.h"
#include "wx/choicdlg.h"
#include "wx/progdlg.h"
#include "boost/filesystem.hpp"
#include "freeimage.h"
#include <fstream>
#include <algorithm>
#include <sstream>

wxAuiPaneInfo TextureManagementWindow::s_defaultPaneInfo;
wxString TextureManagementWindow::s_formatEnumStrs[] = {
    // No compression.       
    "Uncompressed",//Format_RGBA     
    "DXT1",//Format_DXT1
    "DXT1 Alpha", //Format_DXT1a
    "DXT3", //Format_DXT3
    "DXT5", //Format_DXT5
    "DXT5 Normal", //Format_DXT5n Compressed HILO: R=1, G=y, B=0, A=x 
};
wxString TextureManagementWindow::s_qualityEnumStrs[] = {
    "Fastest",
    "Normal",
    "Production",
    "Highest",
};
wxString TextureManagementWindow::s_wrapModeEnumStrs[] = {
    "Clamp",
    "Repeat",
    "Mirror",
};
wxString TextureManagementWindow::s_mipmapFilterEnumStrs[] = {
    "Box",
    "Triangle",
    "Kaiser",
};
wxString TextureManagementWindow::s_alphaModeEnum[]={
    "None",
    "Transparency",
    "Pre-multiplied",
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureManagementWindow::initFrame(vMenuIDProvider* idProvider, wxMenu* baseMenu)
{
    s_defaultPaneInfo.Name("Texture Management");
    s_defaultPaneInfo.Caption("Texture Management");
    s_defaultPaneInfo.Float();
    s_defaultPaneInfo.Floatable(true);
    s_defaultPaneInfo.CloseButton(true);
    s_defaultPaneInfo.MaximizeButton(true);
    s_defaultPaneInfo.MinimizeButton(true);
    s_defaultPaneInfo.CaptionVisible(true);
    s_defaultPaneInfo.Show(false);
    s_defaultPaneInfo.MinSize(800, 350); 

    wxMenu* menu=new wxMenu(); 
    menu->Append(idProvider->aquireMenuID(MENUID_SAVE_TEXTURE_DB), "&Save Database");
    menu->Append(idProvider->aquireMenuID(MENUID_COMPRESS_TEXTURES), "Build & Compress Textures...");

    menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &TextureManagementWindow::evtSaveDB, this, idProvider->getMenuID(MENUID_SAVE_TEXTURE_DB));
    menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &TextureManagementWindow::evtBuild, this, idProvider->getMenuID(MENUID_COMPRESS_TEXTURES));

    baseMenu->AppendSubMenu(menu, "Database");

    texListView_=new wxListView(this,idProvider->aquireMenuID(MENUID_TEXTURE_SELECT));
    texListView_->DragAcceptFiles(true);

    inputPathPicker_=new wxFilePickerCtrl(
        this, 
        wxID_ANY, 
        "", 
        "Select Image File...", 
        "*.png;*.tga;*.jpg;*.dds", 
        wxDefaultPosition, 
        wxSize(500,-1), 
        wxFLP_SMALL|wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL);
    outputPathPicker_=new wxFilePickerCtrl(
        this, 
        wxID_ANY, 
        "",   
        "Select Image File...", 
        "*.dds", 
        wxDefaultPosition, 
        wxSize(500,-1), 
        wxFLP_SMALL|wxFLP_OVERWRITE_PROMPT|wxFLP_SAVE|wxFLP_USE_TEXTCTRL);
    formatSelect_=new wxComboBox(
        this,
        wxID_ANY,
        s_formatEnumStrs[0],
        wxDefaultPosition,
        wxDefaultSize,
        sizeof(s_formatEnumStrs)/sizeof(s_formatEnumStrs[0]),
        s_formatEnumStrs,
        wxCB_DROPDOWN|wxCB_READONLY);
    qualitySelect_=new wxComboBox(
        this,
        wxID_ANY,
        s_qualityEnumStrs[0],
        wxDefaultPosition,
        wxDefaultSize,
        sizeof(s_qualityEnumStrs)/sizeof(s_qualityEnumStrs[0]),
        s_qualityEnumStrs,
        wxCB_DROPDOWN|wxCB_READONLY);
    wrapSelect_=new wxComboBox(
        this,
        wxID_ANY,
        s_wrapModeEnumStrs[0],
        wxDefaultPosition,
        wxDefaultSize,
        sizeof(s_wrapModeEnumStrs)/sizeof(s_wrapModeEnumStrs[0]),
        s_wrapModeEnumStrs,
        wxCB_DROPDOWN|wxCB_READONLY);
    mipSelect_=new wxComboBox(
        this,
        wxID_ANY,
        s_mipmapFilterEnumStrs[0],
        wxDefaultPosition,
        wxDefaultSize,
        sizeof(s_mipmapFilterEnumStrs)/sizeof(s_mipmapFilterEnumStrs[0]),
        s_mipmapFilterEnumStrs,
        wxCB_DROPDOWN|wxCB_READONLY);
    alphaSelect_=new wxComboBox(
        this,
        wxID_ANY,
        s_alphaModeEnum[0],
        wxDefaultPosition,
        wxDefaultSize,
        sizeof(s_alphaModeEnum)/sizeof(s_alphaModeEnum[0]),
        s_alphaModeEnum,
        wxCB_DROPDOWN|wxCB_READONLY);
    convertSelect_=new wxCheckBox(this, idProvider->aquireMenuID(MENUID_CONVERT), "Enable Compress && Convert");
    enableMipsSelect_=new wxCheckBox(this, wxID_ANY, "Enable Mip-Maps");
    enableNormalMapMode_=new wxCheckBox(this, wxID_ANY, "Enable Normal Map");
    enableGammaCorrection_=new wxCheckBox(this, wxID_ANY, "Enable Gamma Correction");
    applyButton_=new wxButton(this, idProvider->aquireMenuID(MENUID_APPLY_TEXTURESETTINGS), "Apply");
    buildButton_=new wxButton(this, idProvider->aquireMenuID(MENUID_BUILD_SINGLE), "Build Selected");

    //previewImage_=new wxGenericStaticBitmap(this, wxID_ANY, wxBitmap());

    wxBoxSizer* topSizer=new wxBoxSizer(wxHORIZONTAL);
    topSizer->Add(texListView_, 35, wxALL|wxEXPAND);

    wxFlexGridSizer* rightSizer=new wxFlexGridSizer(2,wxSize(5,5)); 
    rightSizer->AddGrowableCol(1);
    rightSizer->Add(new wxStaticText(this, wxID_ANY, "Input File"), wxALL|wxEXPAND); 
    rightSizer->Add(inputPathPicker_, wxALL|wxEXPAND);

    rightSizer->Add(new wxStaticText(this, wxID_ANY, ""), wxALL|wxEXPAND); 
    rightSizer->Add(convertSelect_, wxALL|wxEXPAND);

    rightSizer->Add(new wxStaticText(this, wxID_ANY, "Output File"), wxALL|wxEXPAND); 
    rightSizer->Add(outputPathPicker_, wxALL|wxEXPAND);

    rightSizer->Add(new wxStaticText(this, wxID_ANY, ""), wxALL|wxEXPAND); 
    rightSizer->Add(enableMipsSelect_, wxALL|wxEXPAND);

    rightSizer->Add(new wxStaticText(this, wxID_ANY, ""), wxALL|wxEXPAND); 
    rightSizer->Add(enableNormalMapMode_, wxALL|wxEXPAND);

    rightSizer->Add(new wxStaticText(this, wxID_ANY, ""), wxALL|wxEXPAND); 
    rightSizer->Add(enableGammaCorrection_, wxALL|wxEXPAND);

    rightSizer->Add(new wxStaticText(this, wxID_ANY, "Output Format"), wxALL|wxEXPAND); 
    rightSizer->Add(formatSelect_, wxALL|wxEXPAND);

    rightSizer->Add(new wxStaticText(this, wxID_ANY, "Compression Quailty"), wxALL|wxEXPAND); 
    rightSizer->Add(qualitySelect_, wxALL|wxEXPAND);

    rightSizer->Add(new wxStaticText(this, wxID_ANY, "Image Wrapping"), wxALL|wxEXPAND); 
    rightSizer->Add(wrapSelect_, wxALL|wxEXPAND);

    rightSizer->Add(new wxStaticText(this, wxID_ANY, "Mipmap Mode"), wxALL|wxEXPAND); 
    rightSizer->Add(mipSelect_, wxALL|wxEXPAND);

    rightSizer->Add(new wxStaticText(this, wxID_ANY, "Alpha Mode"), wxALL|wxEXPAND); 
    rightSizer->Add(alphaSelect_, wxALL|wxEXPAND);

    rightSizer->Add(applyButton_, wxALL|wxEXPAND);
    rightSizer->Add(buildButton_, wxALL|wxEXPAND);

    topSizer->Add(rightSizer, 55);

    SetSizer(topSizer);

    texListView_->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &TextureManagementWindow::evtSelected, this, idProvider->getMenuID(MENUID_TEXTURE_SELECT));
    texListView_->Connect(wxEVT_DROP_FILES, wxDropFilesEventHandler(TextureManagementWindow::evtDropFiles), NULL, this);// idProvider->getMenuID(MENUID_TEXTURE_SELECT));
    texListView_->Bind(wxEVT_COMMAND_LIST_KEY_DOWN, &TextureManagementWindow::evtKeyDown, this, idProvider->getMenuID(MENUID_TEXTURE_SELECT));
    applyButton_->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureManagementWindow::evtApply, this, idProvider->getMenuID(MENUID_APPLY_TEXTURESETTINGS));
    buildButton_->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureManagementWindow::evtBuildSingle, this, idProvider->getMenuID(MENUID_BUILD_SINGLE));
    convertSelect_->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextureManagementWindow::evtConvertCheck, this, idProvider->getMenuID(MENUID_CONVERT));

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureManagementWindow::updateView(const TextureDatabase& texDB)
{
    texListView_->ClearAll();
    texListView_->AppendColumn("Texture Resources");
    for(TextureDatabase::TextureDBConstItr i=texDB.getFirstItem(); i!=texDB.getEndItem(); ++i) {
        texListView_->InsertItem(0, i->second.getResourceName());
    }

    texListView_->SetColumnWidth(0, wxLIST_AUTOSIZE);
    //texListView_->SortItems();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureManagementWindow::updateControlView(const TextureDBItem& it)
{
    int selcnt=texListView_->GetSelectedItemCount();

    inputPathPicker_->SetPath(it.getInputPath());
    inputPathPicker_->SetInitialDirectory(it.getInputPath());
    outputPathPicker_->SetPath(it.getOutputPath());
    outputPathPicker_->SetInitialDirectory(it.getOutputPath());
    formatSelect_->SetStringSelection(s_formatEnumStrs[it.getFormat()]);
    qualitySelect_->SetStringSelection(s_qualityEnumStrs[it.getQuality()]);
    wrapSelect_->SetStringSelection(s_wrapModeEnumStrs[it.getWrapMode()]);
    mipSelect_->SetStringSelection(s_mipmapFilterEnumStrs[it.getMipFilter()]);
    alphaSelect_->SetStringSelection(s_alphaModeEnum[it.getAlphaMode()]);
    convertSelect_->SetValue(it.getConvertAndCompress());
    enableMipsSelect_->SetValue(it.getEnableMips());
    enableNormalMapMode_->SetValue(it.getNormalMap());
    if (it.getInputGamma()>1.f && it.getOutputGamma()>1.f) {
        enableGammaCorrection_->SetValue(true);
    }
    else {
        enableGammaCorrection_->SetValue(false);
    }

    bool enable=it.getConvertAndCompress();

    enableGammaCorrection_->Enable(true);
    formatSelect_->Enable(enable);
    qualitySelect_->Enable(enable);
    wrapSelect_->Enable(enable);
    mipSelect_->Enable(enable);
    alphaSelect_->Enable(enable);
    enableMipsSelect_->Enable(enable);
    enableNormalMapMode_->Enable(enable);

    if (selcnt>1) {
        inputPathPicker_->Enable(false);
        outputPathPicker_->Enable(false);
    }
    else {
        inputPathPicker_->Enable(true);
        outputPathPicker_->Enable(enable);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureManagementWindow::addTextureResource(vPackage* pkg, const wxString& filepath)
{
    vResourceTypeID textypeid=pkgSys_->getTypeID("TEX");
    boost::filesystem::path inputpath(filepath.c_str());
    std::string filename=inputpath.stem().generic_string();
    wxString resname;
    std::transform(filename.begin(), filename.end(), filename.begin(), toupper);
    bool added=false;
    int cnt=0;
    vResource* res=NULL;

    do {
        if (cnt>0) {
            resname.Printf("%s_%d", filename.c_str(), cnt);
        }
        else {
            resname=filename;
        }

        res=pkg->getResourceInfoByName(resname.c_str());
        if (!res) {
            res=pkg->addResourceInfo(resname.c_str(), textypeid);
            added=true;
        }
        ++cnt;
    } while(!added);

    res->setInputFilePath(filepath.c_str());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureManagementWindow::doTextureDatabaseSave()
{
    texDB_->validateAndConsolidate();

    std::ofstream ofs(textureDatabasePath_);
    boost::archive::text_oarchive oa(ofs);

    const TextureDatabase& tdb = *texDB_;
    oa << tdb;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureManagementWindow::evtSaveDB(wxCommandEvent& evt)
{
    doTextureDatabaseSave();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureManagementWindow::evtBuild(wxCommandEvent& evt)
{
    int selcnt=texListView_->GetSelectedItemCount();
    std::vector< TextureDBItem* > items;
    TextureDatabase::TextureDBConstItr e=texDB_->getEndItem();
    for(TextureDatabase::TextureDBItr i=texDB_->getFirstItemMutable(); i!=e; ++i) {
        items.push_back(&i->second);
    }

    buildTextures(&items[0], items.size());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureManagementWindow::evtSelected(wxListEvent& evt)
{
    TextureDBItem* it=texDB_->getTextureItem(evt.GetItem().GetText());
    if (it) {
        updateControlView(*it);
    }
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureManagementWindow::evtApply(wxCommandEvent& evt)
{
    int selcnt=texListView_->GetSelectedItemCount();
    for(int sel=texListView_->GetFirstSelected(); sel>=0; sel=texListView_->GetNextSelected(sel)) {
        TextureDBItem* it=texDB_->getTextureItem(texListView_->GetItemText(sel).c_str());
        if (selcnt==1) {
            it->setInputPath(inputPathPicker_->GetPath());
            it->setOutputPath(outputPathPicker_->GetPath());
        }
        it->setConvertAndCompress(convertSelect_->IsChecked());
        it->setFormat((nvtt::Format)formatSelect_->GetSelection());
        it->setQuality((nvtt::Quality)qualitySelect_->GetSelection());
        it->setWrapMode((nvtt::WrapMode)wrapSelect_->GetSelection());
        it->setMipFilter((nvtt::MipmapFilter)mipSelect_->GetSelection());
        it->setAlphaMode((nvtt::AlphaMode)alphaSelect_->GetSelection());
        it->setEnableMips(enableMipsSelect_->IsChecked());
        it->setNormalMap(enableNormalMapMode_->IsChecked());
        it->setInputGamma(enableGammaCorrection_->IsChecked() ? 2.2f : 1.f);
        it->setOutputGamma(enableGammaCorrection_->IsChecked() ? 2.2f : 1.f);
        it->flushToResource();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureManagementWindow::evtConvertCheck(wxCommandEvent& evt)
{
    bool enable=evt.IsChecked();
    enableGammaCorrection_->Enable(true);

    if (enable && outputPathPicker_->GetPath().Length() == 0) {
        boost::filesystem::path op=inputPathPicker_->GetPath();
        op.replace_extension(boost::filesystem::path(L".DDS"));
        outputPathPicker_->SetPath(op.generic_string());
    }
    outputPathPicker_->Enable(enable);
    formatSelect_->Enable(enable);
    qualitySelect_->Enable(enable);
    wrapSelect_->Enable(enable);
    mipSelect_->Enable(enable);
    alphaSelect_->Enable(enable);
    enableMipsSelect_->Enable(enable);
    enableNormalMapMode_->Enable(enable);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureManagementWindow::evtDropFiles(wxDropFilesEvent& evt)
{
    wxArrayString pkgsNames;
    size_t pkgcount=pkgSys_->getPackageCount();
    int sels;
    pkgsNames.resize(pkgcount);
    for(size_t i=0; i<pkgcount; ++i) {
        pkgsNames[i]=pkgSys_->getPackage(i)->getName();
    }

    wxSingleChoiceDialog pkgs(this, "Select A Package To Add Textures To...", "Select A Package", pkgsNames, (void**)NULL, wxOK|wxCANCEL);
    if (pkgs.ShowModal() == wxID_OK) {
        sels=pkgs.GetSelection();

        vPackage* pkg=pkgSys_->getPackage(sels);
        int filecount=evt.GetNumberOfFiles();
        wxString* filenames=evt.GetFiles();
        for(int i=0; i<filecount; ++i) {
            addTextureResource(pkg, filenames[i]);
        }

        texDB_->validateAndConsolidate();
        updateView(*texDB_);

        doTextureDatabaseSave();
    }
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureManagementWindow::evtBuildSingle(wxCommandEvent& evt)
{
    int selcnt=texListView_->GetSelectedItemCount();
    TextureDBItem** items = (TextureDBItem**)alloca(sizeof(TextureDBItem*)*selcnt);
    for(int sel=texListView_->GetFirstSelected(),i=0; sel>=0; sel=texListView_->GetNextSelected(sel), ++i) {
        items[i]=texDB_->getTextureItem(texListView_->GetItemText(sel).c_str());
    }

    buildTextures(items, selcnt);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureManagementWindow::evtKeyDown(wxListEvent& evt)
{
    if (evt.GetKeyCode() == WXK_DELETE || evt.GetKeyCode() == WXK_BACK) {
        bool somefailed=false;
        int selcnt=texListView_->GetSelectedItemCount();
        wxString msg;
        msg.Printf("Are you sure you want to delete these %i texture(s)?", selcnt);
        if (wxMessageDialog(this, msg, "Delete?", wxYES_NO).ShowModal() == wxID_NO) {
            evt.Skip();
            return;
        }

        for(int sel=texListView_->GetFirstSelected(); sel>=0; sel=texListView_->GetNextSelected(sel)) {
            TextureDBItem* it=texDB_->getTextureItem(texListView_->GetItemText(sel).c_str());
            vPackage* pkg=pkgSys_->getPackage(it->getResource()->getPackageName());
            if (pkg) {
                vErrorCode er=pkg->removeResourceInfo(it->getResource());
                somefailed |= er != vOK;
            }
        }

        if (somefailed) {
            wxMessageDialog(this, "Some textures couldn't be deleted because they are in use by other resources."
                "You need to remove any resources using the texture first").ShowModal();
        }

        texDB_->validateAndConsolidate();
        updateView(*texDB_);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace FreeImageFileIO
{
unsigned DLL_CALLCONV
read_proc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
        return fread(buffer, size, count, (FILE *)handle);
}

unsigned DLL_CALLCONV
write_proc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
        return fwrite(buffer, size, count, (FILE *)handle);
}

int DLL_CALLCONV
seek_proc(fi_handle handle, long offset, int origin) {
        return fseek((FILE *)handle, offset, origin);
}

long DLL_CALLCONV
tell_proc(fi_handle handle) {
        return ftell((FILE *)handle);
}
}


void TextureManagementWindow::buildTextures(TextureDBItem** textures, size_t count)
{
    wxProgressDialog progress( _T("Building..."), _T("Building Textures..."), count, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE);
    wxString msg;
    FreeImageIO fiIO;
    fiIO.read_proc=&FreeImageFileIO::read_proc;
    fiIO.seek_proc=&FreeImageFileIO::seek_proc;
    fiIO.tell_proc=&FreeImageFileIO::tell_proc;
    fiIO.write_proc=&FreeImageFileIO::write_proc;

    for(size_t i=0; i<count; ++i) {
        TextureDBItem* it=textures[i];
        if (!it->getConvertAndCompress()) continue;
        boost::filesystem::path ip=it->getInputPath();
        if (it->getConvertAndCompress() && strlen(it->getOutputPath()) == 0) {
            boost::filesystem::path op=it->getInputPath();
            op.replace_extension(boost::filesystem::path(L".DDS"));
            it->setOutputPath(op.generic_string().c_str());
        }

        msg.Printf("Building %s (from %s --> %s)",it->getResourceName(), it->getInputPath(), it->getOutputPath());
        progress.Update(i, msg);

        FILE* file=fopen(it->getInputPath(), "rb");
        if (file) {
            FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromHandle(&fiIO, (fi_handle)file, 0);
            if(fif != FIF_UNKNOWN) {
                // load from the file handle
                FIBITMAP* dib = FreeImage_LoadFromHandle(fif, &fiIO, (fi_handle)file, 0);
                unsigned int width=FreeImage_GetWidth(dib);
                unsigned int height=FreeImage_GetHeight(dib);
                unsigned int bpp=FreeImage_GetBPP(dib);
                size_t bytes=width*height*4;
                unsigned char* data=new unsigned char[bytes];

                //Free image stores upside down and in BGRA so re-arrange
                switch(bpp) {
                case 32: {
                    for(size_t y=height-1, d=0; y<height; --y) {
                        unsigned char* scanline=FreeImage_GetScanLine(dib, y);
                        for(size_t x=0, slw=(width*4); x<slw; x+=4) {
                            data[d++]=scanline[x+0];
                            data[d++]=scanline[x+1];
                            data[d++]=scanline[x+2];
                            data[d++]=scanline[x+3];
                        }
                    }
                    break;
                }
                case 24: {
                    for(size_t y=height-1, d=0; y<height; --y) {
                        unsigned char* scanline=FreeImage_GetScanLine(dib, y);
                        for(size_t x=0, slw=(width*3); x<slw; x+=3) {
                            data[d++]=scanline[x+0];
                            data[d++]=scanline[x+1];
                            data[d++]=scanline[x+2];
                            data[d++]=0xFF;
                        }
                    }
                    break;
                }
                case 8: {
                    for(size_t y=height-1, d=0; y<height; --y) {
                        unsigned char* scanline=FreeImage_GetScanLine(dib, y);
                        for(size_t x=0; x<width; ++x) {
                            for (size_t i=0; i<4; ++i) {
                                data[d++]=scanline[x];
                            }
                        }
                    }
                    break;
                }
                default:
                    //TODO: error
                    continue;
                    break;
                }

                nvtt::InputOptions inputOptions;
                inputOptions.setTextureLayout(nvtt::TextureType_2D, width, height);
                inputOptions.setMipmapFilter(it->getMipFilter());
                inputOptions.setKaiserParameters(it->getKaiserWidth(), it->getKaiserStretch(), it->getKaiserAlpha());
                inputOptions.setMipmapGeneration(it->getEnableMips());
                inputOptions.setGamma(it->getInputGamma(),it->getOutputGamma());
                inputOptions.setMipmapData(data, width, height);

                nvtt::OutputOptions outputOptions;
                outputOptions.setFileName(it->getOutputPath());

                nvtt::CompressionOptions compresserOptions;
                compresserOptions.setFormat(it->getFormat());
                compresserOptions.setQuality(it->getQuality());

                nvtt::Compressor compressor;
                compressor.process(inputOptions, compresserOptions, outputOptions);

                it->flushToResource();
                FreeImage_Unload(dib);
                delete[] data; data=NULL;
            }
            fclose(file);
        }
        else {
            //error
        }
    }
}
