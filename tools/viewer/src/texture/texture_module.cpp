/********************************************************************

    filename:   texture_module.cpp  
    
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

#include "precompiled.h"
#include "texture/texture_module.h"
#include "texture/texture_management_window.h"
#include <fstream>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureModule::initialise(const vModuleInitStruct& initdata)
{
    menuIDProvider_ = initdata.menuIDProvider;
    aui_ = initdata.aui;
    actionStack_ = initdata.actionStack;
    parent_ = initdata.parent;

    textureDatabase_=new TextureDataStore();

    wxMenu* menu = new wxMenu();
    menu->Append(menuIDProvider_->aquireMenuID(MENUID_SHOW_TEXTURE_MANAGEMENT), "Show Texture Management");

    menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &TextureModule::evtShowTexManagement, this, menuIDProvider_->getMenuID(MENUID_SHOW_TEXTURE_MANAGEMENT));

    initdata.menu->Append(menu, "Texture Manager");

    std::string textureDBPath=initdata.dataPath;
    textureDBPath+="/textures.tdb";

    std::ifstream ofs(textureDBPath);
    if (ofs.is_open()) {
        boost::archive::text_iarchive oa(ofs);

        TextureDataStore& tdb = *textureDatabase_;
        oa >> tdb;
    }

    texManagerWnd_ = new TextureManagementWindow(
        parent_, 
        menuIDProvider_, 
        menu, 
        textureDatabase_, 
        textureDBPath.c_str());
    initdata.aui->AddPane(texManagerWnd_, TextureManagementWindow::getDefaultAuiPaneInfo());

    initdata.aui->Update();

    texManagerWnd_->updateView(*textureDatabase_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureModule::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureModule::activate()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureModule::constantUpdate()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureModule::activeUpdate()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureModule::engineUpdate(Heart::hHeartEngine*)
{

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureModule::engineRender(Heart::hHeartEngine*)
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureModule::deactvate()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TextureModule::evtShowTexManagement(wxCommandEvent& evt)
{
    wxAuiPaneInfo& info=aui_->GetPane(TextureManagementWindow::getDefaultAuiPaneInfo().name);
    info.Show(true);
    aui_->Update();
}
