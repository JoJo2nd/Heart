/********************************************************************

    filename:   resource_impl.h  
    
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

#ifndef RESOURCE_IMPL_H__
#define RESOURCE_IMPL_H__

#include "viewer_api_config.h"
#include "package_system.h"
#include "xml_helpers.h"
#include "boost/smart_ptr.hpp"
#include "boost/filesystem.hpp"
#include <vector>
#include <map>

VAPI_PRIVATE_HEADER();

class vResourceParameter::vImpl
{
public:
    vImpl() {}
    ~vImpl() {}

    const char*     getName() const { return name_.c_str(); }
    void            setName(const char* name) { name_ = name; }
    const char*     getValue() const { return value_.c_str(); }
    void            setValue(const char* val) { value_ = val; }
    void            serialise(xml_doc* doc, rapidxml::xml_node<>* node) const;
private:

    std::string     name_;
    std::string     value_;
};

class vResource::vImpl 
{
public:
    typedef std::vector< boost::shared_ptr< vResource > > DependencyTableType;
    typedef std::map< std::string, boost::shared_ptr< vResourceParameter > >   ParameterMapType; 
    typedef std::pair< std::string, boost::shared_ptr< vResourceParameter > > ParamPair;

    vImpl(vPackage::vImpl* pkg, const char* resName);
    ~vImpl() 
    {}
    const char*         getName() const { return resName_.c_str(); }
    void                setName(const char* name);
    const char*         getPackageName() const;
    const char*         getFullAssetName() const;
    vResourceTypeID     getType() const { return resType_; }
    void                setType(vResourceTypeID type) { resType_ = type; }
    size_t              getParameterCount() const { return parameters_.size(); }
    vResourceParameter* getParameter(size_t idx) const;
    vResourceParameter* getParameter(const char* paramname) const;
    void                addParameter(const vResourceParameter& param);
    void                removeParameter(const vResourceParameter& param);
    void                removeParameter(const char* paramname);
    void                setInputFilePath(const boost::filesystem::path& p);

    //Non-public interface
    vErrorCode          addAsDependent(boost::shared_ptr< vResource > dep);
    void                serialise(xml_doc* doc, rapidxml::xml_node<>* node, const vPackageSystem& pkgsys) const;

    vPackage::vImpl*             pkg_;
    std::string                  resName_;
    vResourceTypeID              resType_;
    std::string                  fullAssetPath_;
    std::string                  relativeInputPathStr_;
    boost::filesystem::path      absoluteInputPath_;
    boost::filesystem::path      relativeInputPath_;
    DependencyTableType          dependencies_;
    ParameterMapType             parameters_;
};

#endif // RESOURCE_IMPL_H__