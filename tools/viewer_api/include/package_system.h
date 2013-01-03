/********************************************************************

filename:   package_system.h  

Copyright (c) 31:12:2012 James Moran

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

#ifndef PACKAGE_SYSTEM_H__
#define PACKAGE_SYSTEM_H__

#include "viewer_api_config.h"
#include "status_codes.h"

class vPackage;
class vPackageLink;
class vResource;
class vResourceParameter;

typedef int vResourceTypeID;

class VAPI_EXPORT vPackageSystem 
{
    VAPI_PIMPL(vPackageSystem);
public:
    vPackageSystem();
    ~vPackageSystem();
    bool            initialiseSystem(const wchar_t*);
    bool            serialise();
    size_t          getPackageCount() const;
    vPackage*       getPackage(size_t) const;
    vPackage*       getPackage(const char*) const;
    vResourceTypeID  registerType(const char* ext);
    const char*     getTypeExt(vResourceTypeID type) const;
};

class VAPI_EXPORT vPackage 
{
    VAPI_PIMPL(vPackage);
public:
    vPackage();
    ~vPackage();
    const char*         getName() const;
    size_t              getHeapSize() const;
    void                setHeapSize(size_t) const;
    size_t              getResourceCount() const;
    vResource*          getResourceInfo(size_t) const;
    vResource*          getResourceInfoByName(const char* asset);
    vResource*          addResourceInfo(const char* name, vResourceTypeID type);
    vErrorCode          removeResourceInfo(vResource*);
    void                addPackageLink(vResource* res, vResource* linkedRes);
};

class VAPI_EXPORT vResource
{
    VAPI_PIMPL(vResource);
public:
    vResource(vPackage::vImpl* pkg, const char* name);
    ~vResource();
    const char*         getName() const;
    void                setName(const char*);
    const char*         getPackageName() const;
    const char*         getFullAssetName() const;
    vResourceTypeID     getType() const;
    void                setType(vResourceTypeID type);
    size_t              getParameterCount() const;
    vResourceParameter* getParameter(size_t) const;
    vResourceParameter* getParameter(const char*) const;
    void                addParameter(const vResourceParameter&);
    void                removeParameter(const vResourceParameter&);
    void                removeParameter(const char*);
    void                setInputFilePath(const char* filepath);
};

class VAPI_EXPORT vResourceParameter
{
    VAPI_PIMPL(vResourceParameter);
public:
    vResourceParameter();
    ~vResourceParameter();
    const char*     getName() const;
    void            setName(const char*);
    const char*     getValue() const;
    void            setValue(const char*);
};

#endif // PACKAGE_SYSTEM_H__