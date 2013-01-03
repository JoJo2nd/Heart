/********************************************************************

    filename:   modulesystem.h  
    
    Copyright (c) 3:1:2013 James Moran
    
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

#ifndef MODULESYSTEM_H__
#define MODULESYSTEM_H__

#include "viewer_api.h" 
#include "consolelog.h"
#include <string>
#include <vector>

struct ModuleDesc
{
    std::string name;
    boost::shared_ptr< vModuleBase > module;
};

class ModuleSystem
{
public:
    ModuleSystem();
    ~ModuleSystem();

    void initialiseAndLoadPlugins(wxAuiManager* auiManager, wxMenuBar* menubar,const std::string& pluginPaths);
    void getModuleList(std::vector< ModuleDesc >* outarray) const;
    void switchToModule();
    boost::shared_ptr< vModuleBase > getActiveModule() const;

private:

    typedef vModuleBase* (*PlugInEntryProc)();
    struct LoadedModuleInfo 
    {
        HMODULE         loadedLib_;
        PlugInEntryProc entryProc_;
        vModuleBase*    module_;
        vActionStack*   actionStack_;
    };
    typedef std::vector< boost::filesystem::path > PathVectorType;
    typedef std::vector< LoadedModuleInfo > ModuleVectorType;

    PathVectorType   plugInPaths_;
    ModuleVectorType loadedPlugIns_;
};

#endif // MODULESYSTEM_H__