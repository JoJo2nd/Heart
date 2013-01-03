/********************************************************************

    filename:   package_system_impl.h  
    
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

#ifndef PACKAGE_SYSTEM_IMPL_H__
#define PACKAGE_SYSTEM_IMPL_H__

#include "viewer_api_config.h"
#include "package_system.h"
#include "boost/smart_ptr.hpp"
#include "boost/filesystem.hpp"
#include "boost/crc.hpp"
#include <map>

VAPI_PRIVATE_HEADER();

class vPackage;

class vPackageSystem::vImpl
{
    typedef std::map< std::string, boost::shared_ptr< vPackage > > PackageMapType;
    typedef std::map< boost::crc_32_type::value_type, std::string > TypeExtMapType;
public:
    vImpl(vPackageSystem* interface)
        : interface_(interface)
    {
    }
    ~vImpl()
    {
    }

    bool        initialiseSystem(const wchar_t* datapath);
    bool        serialise();
    size_t      getPackageCount() const { return packages_.size(); }
    vPackage*   getPackage(size_t pkg);
    vPackage*   getPackage(const char* pkg);
    vResourceTypeID registerType(const char* ext);
    const char*    getTypeExt(vResourceTypeID type);

    vPackageSystem*         interface_;
    boost::filesystem::path dataRoot_;
    PackageMapType          packages_;
    TypeExtMapType          resourceTypes_;
};

#endif // PACKAGE_SYSTEM_IMPL_H__