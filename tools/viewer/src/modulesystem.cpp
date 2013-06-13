/********************************************************************

    filename:   modulesystem.cpp  
    
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

#include "precompiled.h"
#include "modulesystem.h"
#include "menuidprovider.h"
#include "texture/texture_module.h"
#include "mesh/mesh_module.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ModuleSystem::ModuleSystem()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ModuleSystem::~ModuleSystem()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ModuleSystem::initialiseAndLoadPlugins(
    wxAuiManager* auiManager, 
    wxWindow* parentWnd, 
    wxMenuBar* menubar, 
    const std::string& pluginPaths, 
    boost::filesystem::path& databasePath)
{
    vModuleInitStruct initData;
    initData.aui = auiManager;
    initData.menu = menubar;
    initData.parent = parentWnd;

    //TODO: improve this!!!! Factory?
    loadedPlugIns_.resize(2);
    loadedPlugIns_[0].module_ = new TextureModule();
    loadedPlugIns_[0].actionStack_=new vActionStack();
    loadedPlugIns_[0].menuIDProvider_=new MenuIDProvider();
    boost::filesystem::path datapath=databasePath/"CONFIG"/loadedPlugIns_[0].module_->getModuleName();
    loadedPlugIns_[0].dataPath_=datapath.generic_string();
    boost::filesystem::create_directories(datapath);
    initData.actionStack=loadedPlugIns_[0].actionStack_;
    initData.menuIDProvider=loadedPlugIns_[0].menuIDProvider_;
    initData.dataPath=loadedPlugIns_[0].dataPath_.c_str();
    loadedPlugIns_[0].module_->initialise(initData);

    loadedPlugIns_[1].module_ = new MeshModule();
    loadedPlugIns_[1].actionStack_=new vActionStack();
    loadedPlugIns_[1].menuIDProvider_=new MenuIDProvider();
    datapath=databasePath/"CONFIG"/loadedPlugIns_[1].module_->getModuleName();
    loadedPlugIns_[1].dataPath_=datapath.generic_string();
    boost::filesystem::create_directories(datapath);
    initData.actionStack=loadedPlugIns_[1].actionStack_;
    initData.menuIDProvider=loadedPlugIns_[1].menuIDProvider_;
    initData.dataPath=loadedPlugIns_[1].dataPath_.c_str();
    loadedPlugIns_[1].module_->initialise(initData);

    /*
    std::string str=pluginPaths;
    size_t marker;
    while((marker=str.find_first_of(';')) != std::string::npos) {
        boost::filesystem::path pipath = str.substr(marker+1, str.find_first_of(';', marker+1)-(marker+1));
        plugInPaths_.push_back(pipath);
        str=str.substr(marker+1);
        ConsoleLog::logString("Module", "Found plugin path %s", pipath.generic_string().c_str());
    }

    //Add working directory
    plugInPaths_.push_back(boost::filesystem::current_path());

    //Search for dlls, but not doing recursive search
    for(PathVectorType::iterator i=plugInPaths_.begin(),c=plugInPaths_.end(); i!=c; ++i) {
        if (!boost::filesystem::is_directory(*i)) continue;
        boost::filesystem::directory_iterator ditr(*i),ditrend;
        for(;ditr!=ditrend; ++ditr) {
            boost::filesystem::path fn = *ditr;
            if(fn.extension() == ".dll" || fn.extension() == ".DLL") {
                HMODULE sharedlib = LoadLibrary(fn.c_str());
                if (sharedlib) {
                    ConsoleLog::logString("Module","Loaded library %s", fn.generic_string().c_str());
                    PlugInEntryProc entry=(PlugInEntryProc)GetProcAddress(sharedlib, "viewer_plugin_factory_create");
                    if (entry) {
                        ConsoleLog::logString("Module","Found proc address for viewer_plugin_factory_create()[0x%08X] in %s", entry, fn.generic_string().c_str());
                        LoadedModuleInfo newmodule;
                        newmodule.loadedLib_=sharedlib;
                        newmodule.entryProc_=entry;
                        newmodule.module_=NULL;
                        newmodule.actionStack_=NULL;

                        loadedPlugIns_.push_back(newmodule);
                    }
                    else {
                        ConsoleLog::logString("Module","Unloaded library %s. Not a valid plugin", fn.generic_string().c_str());
                        FreeLibrary(sharedlib);
                    }
                }
            }
        }
    }

    vModuleInitStruct initData;
    initData.aui = auiManager;
    initData.menu = menubar;
    initData.parent = parentWnd;
    initData.pgkSystem = pkgsys;
    for (size_t i=0,c=loadedPlugIns_.size(); i<c; ++i) {
        loadedPlugIns_[i].module_ = loadedPlugIns_[i].entryProc_();
        loadedPlugIns_[i].actionStack_=new vActionStack();
        loadedPlugIns_[i].menuIDProvider_=new MenuIDProvider();
        boost::filesystem::path datapath=databasePath/"CONFIG"/loadedPlugIns_[i].module_->getModuleName();
        loadedPlugIns_[i].dataPath_=datapath.generic_string();
        boost::filesystem::create_directories(datapath);
        initData.actionStack=loadedPlugIns_[i].actionStack_;
        initData.menuIDProvider=loadedPlugIns_[i].menuIDProvider_;
        initData.dataPath=loadedPlugIns_[i].dataPath_.c_str();
        loadedPlugIns_[i].module_->initialise(initData);
    }
    */
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ModuleSystem::shutdown() {
    for (size_t i=0,c=loadedPlugIns_.size(); i<c; ++i) {
        loadedPlugIns_[i].module_->deactvate();
    }

    for (size_t i=0,c=loadedPlugIns_.size(); i<c; ++i) {
        loadedPlugIns_[i].module_->destroy();
    }

    for (size_t i=0,c=loadedPlugIns_.size(); i<c; ++i) {
        delete loadedPlugIns_[i].module_;
        loadedPlugIns_[i].module_=NULL;
        delete loadedPlugIns_[i].actionStack_;
        loadedPlugIns_[i].actionStack_=NULL;
        delete loadedPlugIns_[i].menuIDProvider_;
        loadedPlugIns_[i].menuIDProvider_=NULL;
    }

    loadedPlugIns_.clear();
}
