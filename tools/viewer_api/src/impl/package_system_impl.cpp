/********************************************************************

    filename:   package_system_impl.cpp  
    
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

#include "impl/package_system_impl.h"
#include "impl/package_impl.h"
#include "package_system.h"
#include "boost/shared_ptr.hpp"
#include "boost/filesystem.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool vPackageSystem::vImpl::initialiseSystem(const wchar_t* datapath)
{
    using namespace boost;

    dataRoot_ = filesystem::canonical(datapath);
    filesystem::recursive_directory_iterator dirItr(dataRoot_);

    for (dirItr; dirItr != filesystem::recursive_directory_iterator(); ++dirItr) {
        filesystem::path p = *dirItr;
        if (p.filename()=="PKG.XML") {
            int lvl = dirItr.level();
            if (lvl == 2) { // we don't allow nested packages
                boost::shared_ptr< vPackage > pkg(new vPackage());
                pkg->impl_->loadFromConfig(p, interface_);
                packages_[pkg->getName()] = pkg;
            }
        }
    }

    for (PackageMapType::iterator i=packages_.begin(); i!=packages_.end(); ++i) {
        i->second->impl_->resolveResourceLinks(interface_);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool vPackageSystem::vImpl::serialise()
{
    for (PackageMapType::iterator i=packages_.begin(); i!=packages_.end(); ++i) {
        i->second->impl_->serialise(*interface_);
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vPackage* vPackageSystem::vImpl::getPackage(const char* pkg)
{
    boost::shared_ptr< vPackage > r = packages_[pkg];
    return r.get();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vPackage* vPackageSystem::vImpl::getPackage(size_t pkg)
{
    size_t i=0;
    for(PackageMapType::const_iterator itr=packages_.begin(),c=packages_.end(); itr!=c; ++itr, ++i) {
        if (i == pkg) return itr->second.get();
    }

    return NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vResourceTypeID vPackageSystem::vImpl::registerType(const char* ext)
{
    assert(strlen(ext) == 3);
    if (strlen(ext) != 3) return 0;
    boost::crc_32_type crc;
    crc.process_block(ext, ext+3);
    resourceTypes_[crc.checksum()] = ext;
    return crc.checksum();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const char* vPackageSystem::vImpl::getTypeExt(vResourceTypeID type)
{
    return resourceTypes_[type].c_str();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vResourceTypeID vPackageSystem::vImpl::getTypeID(const char* ext)
{
    assert(strlen(ext) == 3);
    if (strlen(ext) != 3) return 0;
    boost::crc_32_type crc;
    crc.process_block(ext, ext+3);
    if (resourceTypes_.find(crc.checksum()) == resourceTypes_.end()) return 0;
    return crc.checksum();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void vPackageSystem::vImpl::getResourcesOfType(vResourceTypeID type, std::vector<vResource*>* outarray)
{
    for(PackageMapType::const_iterator itr=packages_.begin(),c=packages_.end(); itr!=c; ++itr) {
        itr->second->getResourcesOfType(type, outarray);
    }
}
