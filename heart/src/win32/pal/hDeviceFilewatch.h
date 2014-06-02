/********************************************************************

    filename:   hDeviceFileWatch.h  
    
    Copyright (c) 16:10:2013 James Moran
    
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

#ifndef HDEVICEFILEWATCH_H__
#define HDEVICEFILEWATCH_H__

#include "base/hTypes.h"
#include "base/hFunctor.h"

namespace Heart
{
    enum hdFilewatchEvents
    {
        hdFilewatchEvents_AddRemove      = 1 << 1,
        hdFilewatchEvents_FileModified   = 1 << 2,
        //hdFilewatchEvents_RecursiveWatch = 1 << 3,

        hdFilewatchEvents_Added          = 1 << 4,
        hdFilewatchEvents_Removed        = 1 << 5, 
        hdFilewatchEvents_Modified       = 1 << 6,
        hdFilewatchEvents_Rename         = 1 << 7,

        hdFilewatchEvents_Unknown,
    };

    hFUNCTOR_TYPEDEF(void(*)(const hChar*, const hChar*, hdFilewatchEvents), hdFilewatchEventCallback);
    typedef hUintptr_t hdFilewatchHandle;

    hdFilewatchHandle hdBeginFilewatch(const hChar* path, hUint validevents, hdFilewatchEventCallback callback);
    void              hdEndFilewatch(hdFilewatchHandle);
}

#endif // HDEVICEFILEWATCH_H__