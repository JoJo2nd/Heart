/********************************************************************

    filename:   viewer_config.h  
    
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

#ifndef VIEWER_CONFIG_H__
#define VIEWER_CONFIG_H__

#ifdef VAPI_COMPILE_DLL
#   define VAPI_EXPORT __declspec(dllexport)
#else
#   define VAPI_EXPORT __declspec(dllimport)
#endif

#define VAPI_API   __cdecl

#define VAPI_PIMPL(klass) \
    public: \
        class vImpl; \
        vImpl*   impl_; \
    private: \
        klass(const klass& rhs); \
        klass& operator = (const klass& rhs)

#ifdef VAPI_COMPILE_DLL
#   define VAPI_PRIVATE_HEADER()
#else
#   define VAPI_PRIVATE_HEADER() char* err_CannotIncludeOutsideOfLibrary[0]
#endif

#endif // VIEWER_CONFIG_H__