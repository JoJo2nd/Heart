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

#include "plugin_defines.h"
#include "viewer_api.h"
#include "wx/event.h"

class TextureModule : public vModuleBase
{
public:
    TextureModule() {}
    ~TextureModule() {}
    const char*  getModuleName() const { return PLUGIN_NAME; }
    void initialise(const vModuleInitStruct& initdata);
    void destroy();
    void activate();
    void constantUpdate();
    void activeUpdate();
    void engineUpdate(Heart::hHeartEngine*);
    void engineRender(Heart::hHeartEngine*);
    void deactvate();

    void evtTest1(wxCommandEvent& evt);
    void evtTest2(wxCommandEvent& evt);
};

#endif // TEXTURE_MODULE_H__