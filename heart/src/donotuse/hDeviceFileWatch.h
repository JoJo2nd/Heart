/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hFunctor.h"

namespace Heart {
    enum hdFilewatchEvents {
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