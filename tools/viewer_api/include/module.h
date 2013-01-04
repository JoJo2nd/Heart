/********************************************************************

    filename:   module.h  
    
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

#ifndef MODULE_H__
#define MODULE_H__

#include <exception>

namespace Heart
{
class hHeartEngine;
}

class vActionStack;
class wxWindow;
class wxAuiManager;
class wxMenuBar;

typedef void (VAPI_API* vDebugStringOutputProc)(const char* msg, ...);

struct vModuleInitStruct 
{
    vDebugStringOutputProc  debugOutput;
    wxAuiManager*           aui;
    wxMenuBar*              menu;
    vActionStack*           actionStack;
    wxWindow*               parent;
};

class vModuleException : public std::exception
{
public:
    explicit vModuleException(const char* msg)
        : what_(msg)
    {
    }
    const char* what() const {return what_;}
private:
    const char* what_;
};

class vModuleBase
{
public:
    virtual ~vModuleBase() {}
    virtual const char*  getModuleName() const = 0;
    virtual void initialise(const vModuleInitStruct& initdata) = 0;
    virtual void destroy() = 0;
    virtual void activate() = 0;
    virtual void constantUpdate() = 0;
    virtual void activeUpdate() = 0;
    virtual void engineUpdate(Heart::hHeartEngine*) = 0;
    virtual void engineRender(Heart::hHeartEngine*) = 0;
    virtual void deactvate() = 0;
};

#endif // MODULE_H__