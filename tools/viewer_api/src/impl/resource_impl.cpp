/********************************************************************

    filename:   resource_impl.cpp  
    
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


#include "impl/resource_impl.h"
#include "impl/package_impl.h"
#include "package_system.h"
#include "boost/shared_ptr.hpp"
#include "boost/filesystem.hpp"
#include <shlwapi.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vResource::vImpl::vImpl(vPackage::vImpl* pkg, const char* resName) : pkg_(pkg)
    , resName_(resName)
{
    fullAssetPath_ = getPackageName();
    fullAssetPath_ += ".";
    fullAssetPath_ += resName_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void vResource::vImpl::setInputFilePath(const boost::filesystem::path& p)
{
    absoluteInputPath_ = boost::filesystem::absolute(p, pkg_->getPackagePath());
    if (boost::filesystem::exists(absoluteInputPath_)) {
        absoluteInputPath_ = boost::filesystem::canonical(absoluteInputPath_);//dies when path is not there
    }
    
    wchar_t outputRelPath[MAX_PATH];
    PathRelativePathTo(outputRelPath, pkg_->getPackagePath().c_str(), FILE_ATTRIBUTE_DIRECTORY, absoluteInputPath_.c_str(), FILE_ATTRIBUTE_NORMAL);
    if (wcslen(outputRelPath) == 0) {
        relativeInputPath_ = absoluteInputPath_;
    }
    else {
        relativeInputPath_ = outputRelPath;
    }

    relativeInputPathStr_ = relativeInputPath_.generic_string();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vErrorCode vResource::vImpl::addAsDependent(boost::shared_ptr< vResource > dep)
{
    assert(dep && dep->impl_);

    vImpl* dimpl = dep->impl_;
    //check for cyclic reference
    for(DependencyTableType::iterator i=dimpl->dependencies_.begin(), c=dimpl->dependencies_.end(); i!=c; ++i) {
        if (i->get()->impl_ == this && dimpl->pkg_ != pkg_) {
            return vERROR_CREATES_CYCLIC_REF;
        }
        if (i->get()->impl_ == dep->impl_) {
            return vOK_ALREADY_ADDED;
        }
    }

    dependencies_.push_back(dep);
    return vOK;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vResourceParameter* vResource::vImpl::getParameter(size_t idx) const
{
    size_t i=0;
    for(ParameterMapType::const_iterator itr=parameters_.begin(),c=parameters_.end(); itr!=c; ++itr, ++i) {
        if (i == idx) return itr->second.get();
    }

    return NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vResourceParameter* vResource::vImpl::getParameter(const char* paramname) const
{
    ParameterMapType::const_iterator itr = parameters_.find(paramname);
    if (itr == parameters_.end()) return NULL;
    return itr->second.get();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void vResource::vImpl::addParameter(const vResourceParameter& param)
{
    if (parameters_.find(param.getName()) == parameters_.end()) {
        parameters_.insert(ParamPair(param.getName(),boost::shared_ptr< vResourceParameter >(new vResourceParameter)));
    }
    boost::shared_ptr< vResourceParameter > p = parameters_[param.getName()];
    p->setName(param.getName());
    p->setValue(param.getValue());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void vResource::vImpl::removeParameter(const vResourceParameter& param)
{
    parameters_.erase(param.getName());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void vResource::vImpl::removeParameter(const char* paramname)
{
    parameters_.erase(paramname);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void vResource::vImpl::serialise(xml_doc* doc, rapidxml::xml_node<>* node, const vPackageSystem& pkgsys) const
{
    using namespace rapidxml;
    xml_node<>* resnode=doc->allocate_node(node_element, "resource");
    xml_attribute<>* attr=doc->allocate_attribute("name", resName_.c_str());
    resnode->append_attribute(attr);
    attr=doc->allocate_attribute("input", relativeInputPathStr_.c_str());
    resnode->append_attribute(attr);
    attr=doc->allocate_attribute("type", pkgsys.getTypeExt(resType_));
    resnode->append_attribute(attr);
    for(ParameterMapType::const_iterator i=parameters_.begin(); i!=parameters_.end(); ++i) {
        i->second->impl_->serialise(doc, resnode);
    }
    node->append_node(resnode);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const char* vResource::vImpl::getPackageName() const
{
    return pkg_->getName();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void vResource::vImpl::setName(const char* name)
{
    resName_ = name;
    fullAssetPath_ = getPackageName();
    fullAssetPath_ += ".";
    fullAssetPath_ += resName_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const char* vResource::vImpl::getFullAssetName() const
{
    return fullAssetPath_.c_str();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const char* vResource::vImpl::getInputFilePath()
{
    absoluteInputPathStr_ = absoluteInputPath_.generic_string();
    return absoluteInputPathStr_.c_str();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void vResourceParameter::vImpl::serialise(xml_doc* doc, rapidxml::xml_node<>* node) const
{
    using namespace rapidxml;
    xml_node<>* pnode=doc->allocate_node(node_element, name_.c_str(), value_.c_str());
    node->append_node(pnode);
}
