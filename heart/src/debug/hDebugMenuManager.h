/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include <functional>

namespace Heart {
namespace hRenderer {
    struct hCmdList;
}
class hActionManager;
namespace hDebugMenuManager {
    typedef std::function<void ()> DebugMenuDrawCallback;
#if HEART_DEBUG_INFO
    void registerMenu(const hChar* name, const DebugMenuDrawCallback& callback);
    void unregisterMenu(const hChar* name);
    void SubmitMenus(hActionManager* action_mgr);
#endif
}
}