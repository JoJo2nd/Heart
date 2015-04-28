/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "imgui.h"

namespace Heart {
    class hSystem;
    class hActionManager;
namespace hRenderer {
    struct hCmdList;
}

    hBool ImGuiInit();
    void ImGuiShutdown();
    hRenderer::hCmdList* ImGuiNewFrame(hSystem* system, hActionManager* action_mgr);
}
