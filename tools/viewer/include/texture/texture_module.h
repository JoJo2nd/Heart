/********************************************************************

    filename:   texture_module.h  
    
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

#ifndef TEXTURE_MODULE_H__
#define TEXTURE_MODULE_H__

#include "texture/texture_data_store.h"
#include "common/module.h"

class TextureManagementWindow;

enum TextureModuleMenuID
{
    MENUID_SHOW_TEXTURE_MANAGEMENT,
    MENUID_SAVE_TEXTURE_DB,
    MENUID_ADD_DELETE_TEXTURES,
    MENUID_COMPRESS_TEXTURES,
    MENUID_EXPORT_TEXTURE_DATABASE,
    MENUID_TEXTURE_SELECT,
    MENUID_CONVERT,
    MENUID_APPLY_TEXTURESETTINGS,
    MENUID_BUILD_SINGLE,
    MENUID_PACKAGE_SEARCH,
    MENUID_RESOURCE_SEARCH,
    MENUID_FILENAME_SEARCH,

    MENUID_MAX
};

class TextureModule : public vModuleBase
{
public:
    TextureModule() {}
    ~TextureModule() {}
    const char*  getModuleName() const { return "Texture Module"; }
    void initialise(const vModuleInitStruct& initdata);
    void destroy();
    void activate();
    void constantUpdate();
    void activeUpdate();
    void engineUpdate(Heart::hHeartEngine*);
    void engineRender(Heart::hHeartEngine*);
    void deactvate();

private:
    void evtShowTexManagement(wxCommandEvent& evt);

    wxAuiManager*           aui_;
    vActionStack*           actionStack_;
    wxWindow*               parent_;
    vMenuIDProvider*        menuIDProvider_;
    TextureDataStore*        textureDatabase_;
    TextureManagementWindow* texManagerWnd_;
};

#endif // TEXTURE_MODULE_H__