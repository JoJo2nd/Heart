/********************************************************************

    filename:   package_impl.cpp  
    
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

#include "impl/package_impl.h"
#include "impl/resource_impl.h"
#include "impl/xml_helpers.h"
#include "package_system.h"
#include "boost/shared_ptr.hpp"
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vResource* vPackage::vImpl::getResourceInfo(size_t res) const
{
    size_t idx = 0;
    for (ResourceMap::const_iterator i=resources_.begin(),c=resources_.end(); i!=c; ++i, ++idx) {        if (idx == res) {
            return i->second.get();
        }
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vResource* vPackage::vImpl::addResourceInfo(const char* name, vResourceTypeID type)
{
    // Already added?
    if (resources_.find(name) != resources_.end()) return NULL;

    boost::shared_ptr< vResource > restoadd(new vResource(this, name));
    restoadd->setType(type);
    resources_[name] = restoadd;

    return restoadd.get();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vErrorCode vPackage::vImpl::removeResourceInfo(vResource* res)
{
    for (ResourceMap::iterator i=resources_.begin(),c=resources_.end(); i!=c; ++i) {
        if (res == i->second.get()) {
            if (i->second.unique()) {
                resources_.erase(i);
                return vOK;
            }
            else {
                return vERROR_HAS_DEPENDANTS;
            }

        }
    }

    return vERROR_NOT_FOUND;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vErrorCode vPackage::vImpl::addPackageLink(vResource* res, vResource* linkedRes)
{
    assert(res && linkedRes);
    boost::shared_ptr< vResource > ptr = resources_[linkedRes->getName()];
    return res->impl_->addAsDependent(ptr);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vErrorCode vPackage::vImpl::loadFromConfig(boost::filesystem::path xmlpath, vPackageSystem* pkgSys)
{
    packagePath_ = xmlpath.parent_path();
    packagePathStr_ = packagePath_.generic_string();
    xmlPath_ = xmlpath;
    lnkPath_ = packagePath_ / "LNK.XML";
    name_ = packagePath_.filename().generic_string();

    xml_doc pkgxml;
    boost::filesystem::ifstream file;
    
    file.open(xmlpath);
    if (file.is_open()) {
        file.seekg(0, std::ios_base::end);
        size_t fsize = (size_t)file.tellg();
        char* xmldata = new char[fsize+1];
        memset(xmldata, 0, fsize+1);
        file.seekg(0, std::ios_base::beg);
        file.read(xmldata, fsize);
        xmldata[fsize] = 0;
        pkgxml.copy_parse(xmldata, fsize+1);
        delete xmldata; xmldata = NULL;
        file.close();

        xml_getter resnode = xml_getter(pkgxml).first_sibling("resources").first_child("resource");
        for (; resnode.to_node(); ++resnode) {
            vResourceTypeID restypeid = pkgSys->registerType(resnode.get_attribute_string("type"));
            vResource* resource = addResourceInfo(resnode.get_attribute_string("name"), restypeid);
            if (!resource) {
                return vERROR_DUPLICATE_RESORUCE_FOUND;
            }
            resource->setInputFilePath(resnode.get_attribute_string("input"));
            xml_getter paranode = resnode.first_child(NULL);
            for (; paranode.to_node(); paranode = paranode.next_sibling()) {
                vResourceParameter parameter;
                parameter.setName(paranode.name());
                parameter.setValue(paranode.get_value_string());
                resource->addParameter(parameter);
            }
        }       
    }

    return vOK;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vErrorCode vPackage::vImpl::serialise(const vPackageSystem& pkgSys) const
{
    using namespace rapidxml;
    std::vector< std::string > packagelinks;

    for (ResourceMap::const_iterator i=resources_.begin(),c=resources_.end(); i!=c; ++i) {
        boost::shared_ptr<vResource> res = i->second;
        const vResource::vImpl::DependencyTableType& deps = res->impl_->dependencies_;
        for (size_t i=0,c=deps.size(); i<c; ++i) {
            bool added=false;
            for (size_t pli=0,plic=packagelinks.size(); pli<plic; ++pli) {
                if (packagelinks[pli] == deps[i]->getPackageName()) {
                    added = true;
                    break;
                }
                if (!added && name_ != deps[i]->getPackageName()) {
                    packagelinks.push_back(deps[i]->getPackageName());
                }
            }
        }
    }

    xml_doc pkgxml;
    xml_node<>* node = pkgxml.allocate_node(node_element, "packagelinks");
    for (size_t pli=0,plic=packagelinks.size(); pli<plic; ++pli) {
        xml_node<>* link=pkgxml.allocate_node(node_element, "link", packagelinks[pli].c_str());
        node->append_node(link);
    }
    pkgxml.append_node(node);

    node = pkgxml.allocate_node(node_element, "resources");
    for (ResourceMap::const_iterator i=resources_.begin(),c=resources_.end(); i!=c; ++i) {
        i->second->impl_->serialise(&pkgxml, node, pkgSys);
    }
    pkgxml.append_node(node);

    std::fstream file;
    file.open(xmlPath_.generic_string().c_str(), std::ios_base::out|std::ios_base::binary|std::ios_base::trunc);
    if (!file.is_open()) return vERROR_FILE_IO;
    file << pkgxml;
    file.close();

    xml_doc lnkxml;

    for (ResourceMap::const_iterator i=resources_.begin(),c=resources_.end(); i!=c; ++i) {
        boost::shared_ptr<vResource> res = i->second;
        const vResource::vImpl::DependencyTableType& deps = res->impl_->dependencies_;
        for (size_t i=0,c=deps.size(); i<c; ++i) {
            xml_node<>* link=lnkxml.allocate_node(node_element, "link");
            xml_attribute<>* attfrom=lnkxml.allocate_attribute("from", res->getFullAssetName());
            xml_attribute<>* attto=lnkxml.allocate_attribute("to", deps[i]->getFullAssetName());
            link->append_attribute(attfrom);
            link->append_attribute(attto);
            lnkxml.append_node(link);
        }
    }

    file.open(lnkPath_.generic_string().c_str(), std::ios_base::out|std::ios_base::binary|std::ios_base::trunc);
    if (!file.is_open()) return vERROR_FILE_IO;
    file << lnkxml;
    file.close();

    return vOK;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void splitAssetName(const std::string& instr, std::string* pkgstr, std::string* assetstr)
{
    *pkgstr = instr.substr(0, instr.find_first_of('.'));
    *assetstr = instr.substr(instr.find_first_of('.')+1);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vErrorCode vPackage::vImpl::resolveResourceLinks(vPackageSystem* pkgSys)
{
    xml_doc pkgxml;
    boost::filesystem::ifstream file;

    file.open(lnkPath_);
    if (file.is_open()) {
        file.seekg(0, std::ios_base::end);
        size_t fsize = (size_t)file.tellg();
        char* xmldata = new char[fsize+1];
        memset(xmldata, 0, fsize+1);
        file.seekg(0, std::ios_base::beg);
        file.read(xmldata, fsize);
        xmldata[fsize] = 0;
        pkgxml.copy_parse(xmldata, fsize+1);
        delete xmldata; xmldata = NULL;
        file.close();

        xml_getter linknode = xml_getter(pkgxml);
        for (; linknode.to_node(); ++linknode) {
            std::string from = linknode.get_attribute_string("from");
            std::string to = linknode.get_attribute_string("to");
            std::string frompkg, topkg;
            std::string fromasset, toasset;

            splitAssetName(from, &frompkg, &fromasset);
            splitAssetName(to, &topkg, &toasset);

            assert(frompkg == getName());
            vPackage* pkg = pkgSys->getPackage(topkg.c_str());
            if (!pkg) 
                return vERROR_DEP_PKG_NOT_FOUND;//TODO: warn
            vResource* linkres = pkg->getResourceInfoByName((char*)toasset.c_str());
            if (!linkres) 
                return vERROR_DEP_ASSET_NOT_FOUND;//TODO: WARN not error
            vResource* res = getResourceInfoByName(fromasset.c_str());
            if (!res) 
                return vERROR_NOT_FOUND;
            addPackageLink(res, linkres);
        }
    }

    return vOK;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vResource* vPackage::vImpl::getResourceInfoByName(const char* asset)
{
    if (resources_.find(asset) == resources_.end()) return NULL;
    else return resources_[asset].get();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void vPackage::vImpl::getResourcesOfType(vResourceTypeID type, std::vector<vResource*>* outarray)
{
    for (ResourceMap::const_iterator i=resources_.begin(),c=resources_.end(); i!=c; ++i) {
        if (i->second->getType() == type) {
            outarray->push_back(i->second.get());
        }
    }
}
