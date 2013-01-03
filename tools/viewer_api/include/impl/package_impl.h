/********************************************************************

    filename:   package_impl.h  
    
    Copyright (c) 2:1:2013 James Moran
    
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

#ifndef PACKAGE_IMPL_H__
#define PACKAGE_IMPL_H__

#include "viewer_api_config.h"
#include "package_system.h"
#include "boost/smart_ptr.hpp"
#include "boost/filesystem.hpp"
#include <string>
#include <map>

VAPI_PRIVATE_HEADER();

class vResource;

class vPackage::vImpl
{
    typedef std::map< std::string, boost::shared_ptr< vResource > > ResourceMap;

public:
    const char* getName() const { return name_.c_str(); }
    size_t     getHeapSize() const { return heapSize_; }
    void       setHeapSize(size_t bytes) { heapSize_ = bytes; }
    size_t     getResourceCount() const  { return resources_.size(); }
    vResource* getResourceInfo(size_t res) const;
    vResource* getResourceInfoByName(const char* asset);
    vResource* addResourceInfo(const char* name, vResourceTypeID type);
    vErrorCode removeResourceInfo(vResource* res);
    vErrorCode addPackageLink(vResource* res, vResource* linkedRes);
    //non-public interface
    vErrorCode                      loadFromConfig(boost::filesystem::path xmlpath, vPackageSystem* pkgSys);
    vErrorCode                      resolveResourceLinks(vPackageSystem* pkgSys);
    vErrorCode                      serialise(const vPackageSystem& pkgsys) const;
    const boost::filesystem::path&  getPackagePath() const { return packagePath_; }

private:

    std::string             name_;
    boost::filesystem::path packagePath_;
    boost::filesystem::path xmlPath_;
    boost::filesystem::path lnkPath_;
    size_t                  heapSize_;
    ResourceMap             resources_;
};

#endif // PACKAGE_IMPL_H__