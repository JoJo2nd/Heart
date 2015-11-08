/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "debug/hDebugMenuManager.h"
#include "base/hStringID.h"
#include "input/hActionManager.h"
#include "imgui.h"
#include <vector>

namespace Heart {
namespace hDebugMenuManager {
#if HEART_DEBUG_INFO
namespace {
    struct hDebugMenu {
        hDebugMenu() 
            : enabled(false) {
        }
        hDebugMenu(const hChar* in_name, const DebugMenuDrawCallback& in_func)
            : name(in_name)
            , func(in_func)
            , enabled(false) {
        }
        std::string name;
        DebugMenuDrawCallback func;
        bool enabled;
    };

    std::vector<hDebugMenu> menus;
    bool debugMenuOpen = false;

    const hStringID OpenDebugMenuAction("Open Debug Menu");
}
    void registerMenu(const hChar* name, const DebugMenuDrawCallback& callback) {
        for (auto i = menus.begin(), n = menus.end(); i != n; ++i) {
            if (i->name == name) {
                menus.erase(i);
            }
        }
        menus.emplace_back(name, callback);
    }

    void unregisterMenu(const hChar* name) {
        for (auto i=menus.begin(), n=menus.end(); i!=n; ++i){
            if (i->name == name) {
                menus.erase(i);
                return;
            }
        }
    }

    void SubmitMenus(hActionManager* action_mgr) {
        hInputAction in_action;
        if (action_mgr->queryAction(0, OpenDebugMenuAction, &in_action)) {
            if (in_action.rising_) {
                debugMenuOpen = !debugMenuOpen;
            }
        }
        if (!debugMenuOpen) {
            return;
        }

        ImGui::Begin("Debug Menu", &debugMenuOpen, ImGuiWindowFlags_ShowBorders);
        for (auto& i : menus) {
            if (ImGui::Button(i.name.c_str())) i.enabled = !i.enabled;
        }

        for (auto& i : menus) {
            if (i.enabled) {
                i.func();
            }
        }
        ImGui::End();
    }
#endif
}
}