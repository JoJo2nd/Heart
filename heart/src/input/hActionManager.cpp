/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "input/hActionManager.h"
#include "base/hTypes.h"
#include "base/hStringID.h"
#include "base/hDeviceInput.h"
#include "core/hDeviceSystemWindow.h"
#include "lua/hLuaStateManager.h"
#include <unordered_map>
#include <vector>

namespace Heart
{
enum hDeviceID 
{
    hDeviceID_Keyboard = 0,
    hDeviceID_Mouse = 1,
    hDeviceID_ControllerFirst = 2
};

struct hInputID {
    enum hType {
        hType_Keyboard = 0,
        hType_Mouse = 1,
        hType_Controller = 2,
        hType_MouseAxis = 3,
        hType_ControllerAxis = 4,
    };
    // bits 0-1: device type
    // bits 2-11: scan code empty on non-keyboard devices
    union {
        hUint32 id_; 
        struct {
            hUint type_ : 4; //
            hUint scancode_ : 9; // only valid on keyboard
            hUint buttonId_ : 8; // only on mouse/controller, axisId is zero
            hUint axisId_ : 8; // only on mouse/controller, buttonId is zero
            hUint spare : 3;
        };
    };

    hInputID() {}
    hInputID(hType type, hUint scancode, hUint buttonid, hUint axisid)
        : type_(type), scancode_(scancode), buttonId_(buttonid), axisId_(axisid), spare(0)
    {}

    hBool operator == (const hInputID& rhs) const { return id_ == rhs.id_; }
    hBool operator != (const hInputID& rhs) const { return id_ != rhs.id_; }
    hBool operator < (const hInputID& rhs) const { return id_ < rhs.id_; }
};

struct hInputIDHash
{
    hSize_t operator()(const hInputID& rhs) const { return rhs.id_; }
};

typedef std::unordered_map<hInputID, hStringID, hInputIDHash> hActionMapping;
typedef std::unordered_map<hStringID, hInputID> hKeyNameIDMapping;
typedef std::unordered_map<hStringID, hInputAction> hActionInputMapping;
typedef std::vector<hInputAction> hActionArray;
typedef std::vector<DeviceInput::hGameController*> hControllerArray;
typedef std::vector<hActionInputMapping> hBoundMappingsArray;
typedef std::vector<hUint> hIntArray;

class hdSystemWindow;

struct hActionManagerImpl {
    const hUint hInvalidBinding = ~0;

    hdSystemWindow*         systemWindow_;
    hKeyNameIDMapping       keyNameToIDMappings_;
    hActionMapping          defaultMappings_;
    hActionArray            bufferedActions_;
    hControllerArray        openControllers_;
    hBoundMappingsArray     boundMappings_;
    hIntArray               deviceIDToBoundID_;

    void handleSysEvent(hUint syseventid, const hSysEvent* sysevent){
        if (syseventid == hGetSystemEventID(CONTROLLERDEVICEADDED)) {
            if (sysevent->cdevice.which >= openControllers_.size()) {
                openControllers_.resize(sysevent->cdevice.which+1);
            }
            openControllers_[sysevent->cdevice.which] = DeviceInput::hOpenController(sysevent->cdevice.which);
            addDeviceRange(hDeviceID_Keyboard, sysevent->cdevice.which);
        } else if (syseventid == hGetSystemEventID(CONTROLLERDEVICEREMOVED)) {
            DeviceInput::hCloseController(openControllers_[sysevent->cdevice.which]);
            openControllers_[sysevent->cdevice.which] = hNullptr;
        } else if (syseventid == hGetSystemEventID(KEYDOWN)) {
            hInputID keyid(hInputID::hType_Keyboard, sysevent->key.keysym.scancode, 0, 0);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            auto boundID = getBoundID(hDeviceID_Keyboard);
            if (mapping != invalid && boundID != hInvalidBinding) {
                hInputAction action = {
                    mapping->second,
                    0, //device ID
                    0, // value
                    0,
                    1,
                    0,
                    (sysevent->key.repeat > 0),
                    0, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        } else if (syseventid == hGetSystemEventID(KEYUP)) {
            hInputID keyid(hInputID::hType_Keyboard, sysevent->key.keysym.scancode, 0, 0);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            auto boundID = getBoundID(hDeviceID_Keyboard);
            if (mapping != invalid && boundID != hInvalidBinding) {
                hInputAction action = {
                    mapping->second,
                    0, //device ID
                    0, // value
                    0,
                    0,
                    1,
                    (sysevent->key.repeat > 0),
                    0, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        } else if (syseventid == hGetSystemEventID(TEXTINPUT)) {

        } else if (syseventid == hGetSystemEventID(MOUSEMOTION)) {
            auto boundID = getBoundID(sysevent->motion.which);
            int x, y;
            SDL_GetMouseState(&x, &y);
            if (sysevent->motion.xrel != 0) {
                hInputID keyid(hInputID::hType_MouseAxis, 0, 0, 0);
                auto invalid = defaultMappings_.end();
                auto mapping = defaultMappings_.find(keyid);
                if (mapping != invalid && boundID != hInvalidBinding) {
                    hInputAction action = {
                        mapping->second,
                        sysevent->motion.which, //device ID
                        sysevent->motion.xrel, // val
                        x,
                        0,
                        0,
                        0,
                        1,
                    };
                    bufferedActions_.emplace_back(action);
                }
            }
            if (sysevent->motion.yrel != 0) {
                hInputID keyid(hInputID::hType_MouseAxis, 0, 0, 1);
                auto invalid = defaultMappings_.end();
                auto mapping = defaultMappings_.find(keyid);
                if (mapping != invalid && boundID != hInvalidBinding) {
                    hInputAction action = {
                        mapping->second,
                        sysevent->motion.which, //device ID
                        sysevent->motion.yrel, // val
                        y,
                        0,
                        0,
                        0,
                        1,
                    };
                    bufferedActions_.emplace_back(action);
                }
            }
        } else if (syseventid == hGetSystemEventID(MOUSEWHEEL)) {
            auto boundID = getBoundID(sysevent->motion.which);
            if (sysevent->wheel.x != 0) {
                hInputID keyid(hInputID::hType_MouseAxis, 0, 0, 2);
                auto invalid = defaultMappings_.end();
                auto mapping = defaultMappings_.find(keyid);
                if (mapping != invalid && boundID != hInvalidBinding) {
                    hInputAction action = {
                        mapping->second,
                        sysevent->wheel.which, //device ID
                        sysevent->wheel.x, // val
                        sysevent->wheel.x,
                        0,
                        0,
                        0,
                        1,
                    };
                    bufferedActions_.emplace_back(action);
                }
            }
            if (sysevent->wheel.y != 0) {
                hInputID keyid(hInputID::hType_MouseAxis, 0, 0, 3);
                auto invalid = defaultMappings_.end();
                auto mapping = defaultMappings_.find(keyid);
                if (mapping != invalid && boundID != hInvalidBinding) {
                    hInputAction action = {
                        mapping->second,
                        sysevent->wheel.which, //device ID
                        sysevent->wheel.y, // val
                        sysevent->wheel.y, // val
                        0,
                        0,
                        0,
                        1,
                    };
                    bufferedActions_.emplace_back(action);
                }
            }
        } else if (syseventid == hGetSystemEventID(MOUSEBUTTONDOWN)) {
            auto boundID = getBoundID(sysevent->motion.which);
            hInputID keyid(hInputID::hType_Mouse, 0, sysevent->button.button, 0);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            if (mapping != invalid && boundID != hInvalidBinding) {
                hInputAction action = {
                    mapping->second,
                    sysevent->button.which, //device ID
                    1, // value
                    0,
                    1,
                    0,
                    0,
                    0, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        } else if (syseventid == hGetSystemEventID(MOUSEBUTTONUP)) {
            auto boundID = getBoundID(sysevent->motion.which);
            hInputID keyid(hInputID::hType_Mouse, 0, sysevent->button.button, 0);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            if (mapping != invalid && boundID != hInvalidBinding) {
                hInputAction action = {
                    mapping->second,
                    sysevent->button.which, //device ID
                    0, // value
                    0,
                    0,
                    1,
                    0,
                    0, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        } else if (syseventid == hGetSystemEventID(CONTROLLERAXISMOTION)) {
            auto boundID = getBoundID(sysevent->caxis.which);
            hInputID keyid(hInputID::hType_ControllerAxis, 0, 0, sysevent->caxis.axis);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            if (mapping != invalid && boundID != hInvalidBinding) {
                hInputAction action = {
                    mapping->second,
                    sysevent->caxis.which, //device ID
                    sysevent->caxis.value, // axis val
                    0,
                    0,
                    0,
                    0,
                    1, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        } else if (syseventid == hGetSystemEventID(CONTROLLERBUTTONDOWN)) {
            auto boundID = getBoundID(sysevent->cbutton.which);
            hInputID keyid(hInputID::hType_Controller, 0, sysevent->cbutton.button, 0);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            if (mapping != invalid && boundID != hInvalidBinding) {
                hInputAction action = {
                    mapping->second,
                    sysevent->cbutton.which, //device ID
                    0, // value
                    0,
                    1,
                    0,
                    0,
                    0, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        } else if (syseventid == hGetSystemEventID(CONTROLLERBUTTONUP)) {
            auto boundID = getBoundID(sysevent->cbutton.which);
            hInputID keyid(hInputID::hType_Controller, 0, sysevent->cbutton.button, 0);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            if (mapping != invalid && boundID != hInvalidBinding) {
                hInputAction action = {
                    mapping->second,
                    sysevent->cbutton.which, //device ID
                    0, // value
                    0,
                    0,
                    1,
                    0,
                    0, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        }

    }
    void addDeviceRange(hUint first, hUint last) {
        boundMappings_.resize(last+1);
        flushBoundMappings();
    }
    void flushBoundMappings() {
        for (auto device=boundMappings_.begin(), devicen=boundMappings_.end(); device!=devicen; ++device) {
            device->clear();
            auto invalid = device->end();
            for (auto i=defaultMappings_.cbegin(), n=defaultMappings_.cend(); i!=n; ++i) {
                hInputAction action = { i->second };
                (*device)[i->second] = action;
            }
        }
    }
    void addToBoundMappings(hStringID keybinding, hStringID actionname) {
        for (auto device=boundMappings_.begin(), devicen=boundMappings_.end(); device!=devicen; ++device) {
            hInputAction action = { actionname };
            (*device)[keybinding] = action;
        }
    }
    void removeFromBoundMappings(hStringID keybinding) {
        for (auto device=boundMappings_.begin(), devicen=boundMappings_.end(); device!=devicen; ++device) {
            device->erase(keybinding);
        }
    }
    void bindDeviceToID(hUint deviceid, hUint id) {
        if (deviceid >= deviceIDToBoundID_.size()) {
            deviceIDToBoundID_.resize(deviceid+1, hInvalidBinding);
        }
        deviceIDToBoundID_[deviceid] = id;
    }
    hFORCEINLINE hUint getBoundID(hUint deviceid) {
        return deviceIDToBoundID_[deviceid];
    }
    static hInt luaSetDefaultActionMapping(lua_State* L) {
        hActionManager* am = (hActionManager*)lua_topointer(L, lua_upvalueindex(1));
        if (!am) {
            return luaL_error(L, "Unable to grab engine pointer" );
        }
        const hChar* keyname = luaL_checkstring(L, 1);
        const hChar* actionname = luaL_checkstring(L, 2);
        am->setDefaultActionMapping(hStringID(keyname), hStringID(actionname));
        return 0;
    }
    static hInt luaClearDefaultActionMapping(lua_State* L) {
        hActionManager* am = (hActionManager*)lua_topointer(L, lua_upvalueindex(1));
        if (!am) {
            return luaL_error(L, "Unable to grab engine pointer" );
        }
        const hChar* keyname = luaL_checkstring(L, 1);
        am->clearDefaultActionMapping(hStringID(keyname));
        return 0;
    }
};

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hActionManager::hActionManager() {
        impl_=new hActionManagerImpl();

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hActionManager::~hActionManager() {
        delete impl_;
        impl_=nullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    hBool hActionManager::initialise(hdSystemWindow* window)
    {
        auto& systemWindow_ = impl_->systemWindow_;
        auto& keyNameToIDMappings_ = impl_->keyNameToIDMappings_;
        auto& openControllers_ = impl_->openControllers_;

        systemWindow_ = window;

        for (hUint sc=DeviceInput::hGetKeyboardFirstScancode(),scn=DeviceInput::hGetKeyboardLastScancode(); sc<=scn; ++sc) {
            hStringID name=DeviceInput::hGetScancodeName(sc);
            if (!name.is_default()) {
                keyNameToIDMappings_.insert(hKeyNameIDMapping::value_type(name, hInputID(hInputID::hType_Keyboard, sc, 0, 0)));
            }
        }

        for (hUint i=DeviceInput::hGetMouseFirstButtonID(),n=DeviceInput::hGetMouseLastButtonID(); i<=n; ++i) {
            hStringID name=DeviceInput::hGetMouseButtonName(i);
            if (!name.is_default()) {
                keyNameToIDMappings_.insert(hKeyNameIDMapping::value_type(name, hInputID(hInputID::hType_Mouse, 0, i, 0)));
            }
        }

        for (hUint i=DeviceInput::hGetMouseFirstAxisID(),n=DeviceInput::hGetMouseLastAxisID(); i<=n; ++i) {
            hStringID name=DeviceInput::hGetMouseAxisName(i);
            if (!name.is_default()) {
                keyNameToIDMappings_.insert(hKeyNameIDMapping::value_type(name, hInputID(hInputID::hType_MouseAxis, 0, 0, i)));
            }
        }

        for (hUint i=DeviceInput::hGetControllerFirstButtonID(),n=DeviceInput::hGetControllerLastButtonID(); i<=n; ++i) {
            hStringID name=DeviceInput::hGetControllerButtonName(i);
            if (!name.is_default()) {
                keyNameToIDMappings_.insert(hKeyNameIDMapping::value_type(name, hInputID(hInputID::hType_Controller, 0, i, 0)));
            }
        }

        for (hUint i=DeviceInput::hGetControllerFirstAxisID(),n=DeviceInput::hGetControllerLastAxisID(); i<=n; ++i) {
            hStringID name=DeviceInput::hGetControllerAxisName(i);
            if (!name.is_default()) {
                keyNameToIDMappings_.insert(hKeyNameIDMapping::value_type(name, hInputID(hInputID::hType_ControllerAxis, 0, 0, i)));
            }
        }

        openControllers_.resize(DeviceInput::hGetControllerCount());
        for (hUint i=0, n=(hUint)DeviceInput::hGetControllerCount(); i<n; ++i) {
            openControllers_.push_back(DeviceInput::hOpenController(i));
        }

        impl_->addDeviceRange(0, 0);
        impl_->bindDeviceToID(hDeviceID_Keyboard, 0);
        impl_->bindDeviceToID(hDeviceID_Mouse, 0);
        impl_->bindDeviceToID(hDeviceID_ControllerFirst, 0);

        hSysEventHandler sysEvent = hFUNCTOR_BINDMEMBER(hSysEventHandler, hActionManagerImpl, handleSysEvent, impl_);
        systemWindow_->setSysEventHandler(hGetSystemEventID(KEYDOWN), sysEvent);
        systemWindow_->setSysEventHandler(hGetSystemEventID(KEYUP), sysEvent);
        systemWindow_->setSysEventHandler(hGetSystemEventID(TEXTINPUT), sysEvent);
        systemWindow_->setSysEventHandler(hGetSystemEventID(MOUSEBUTTONDOWN), sysEvent);
        systemWindow_->setSysEventHandler(hGetSystemEventID(MOUSEBUTTONUP), sysEvent);
        systemWindow_->setSysEventHandler(hGetSystemEventID(MOUSEWHEEL), sysEvent);
        systemWindow_->setSysEventHandler(hGetSystemEventID(MOUSEMOTION), sysEvent);
        systemWindow_->setSysEventHandler(hGetSystemEventID(CONTROLLERDEVICEADDED), sysEvent);
        systemWindow_->setSysEventHandler(hGetSystemEventID(CONTROLLERDEVICEREMOVED), sysEvent);
        systemWindow_->setSysEventHandler(hGetSystemEventID(CONTROLLERAXISMOTION), sysEvent);
        systemWindow_->setSysEventHandler(hGetSystemEventID(CONTROLLERBUTTONDOWN), sysEvent);
        systemWindow_->setSysEventHandler(hGetSystemEventID(CONTROLLERBUTTONUP), sysEvent);

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::destory()
    {
        auto& systemWindow_ = impl_->systemWindow_;
        auto& openControllers_ = impl_->openControllers_;

        for (hSize_t i=0, n=openControllers_.size(); i<n; ++i) {
            DeviceInput::hCloseController(openControllers_[i]);
            openControllers_[i] = hNullptr;
        }
        openControllers_.clear();

        hSysEventHandler sysEvent = hFUNCTOR_BINDMEMBER(hSysEventHandler, hActionManagerImpl, handleSysEvent, impl_);
        systemWindow_->removeSysEventHandler(hGetSystemEventID(KEYDOWN), sysEvent);
        systemWindow_->removeSysEventHandler(hGetSystemEventID(KEYUP), sysEvent);
        systemWindow_->removeSysEventHandler(hGetSystemEventID(TEXTINPUT), sysEvent);
        systemWindow_->removeSysEventHandler(hGetSystemEventID(MOUSEBUTTONDOWN), sysEvent);
        systemWindow_->removeSysEventHandler(hGetSystemEventID(MOUSEBUTTONUP), sysEvent);
        systemWindow_->removeSysEventHandler(hGetSystemEventID(MOUSEWHEEL), sysEvent);
        systemWindow_->removeSysEventHandler(hGetSystemEventID(MOUSEMOTION), sysEvent);
        systemWindow_->removeSysEventHandler(hGetSystemEventID(CONTROLLERDEVICEADDED), sysEvent);
        systemWindow_->removeSysEventHandler(hGetSystemEventID(CONTROLLERDEVICEREMOVED), sysEvent);
        systemWindow_->removeSysEventHandler(hGetSystemEventID(CONTROLLERAXISMOTION), sysEvent);
        systemWindow_->removeSysEventHandler(hGetSystemEventID(CONTROLLERBUTTONDOWN), sysEvent);
        systemWindow_->removeSysEventHandler(hGetSystemEventID(CONTROLLERBUTTONUP), sysEvent);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::update()
    {
        auto& bufferedActions_ = impl_->bufferedActions_;
        auto& boundMappings_ = impl_->boundMappings_;

        // primative update of bound action mappings
        for (hSize_t i=0, n=bufferedActions_.size(); i<n; ++i) {
            auto* action = &bufferedActions_[i];
            boundMappings_[action->deviceID_][action->actionName_] = *action;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::endOfFrameUpdate()
    {
        auto& bufferedActions_ = impl_->bufferedActions_;
        auto& boundMappings_ = impl_->boundMappings_;

        // primative update of bound action mappings
        for (auto device=boundMappings_.begin(), devicen=boundMappings_.end(); device!=devicen; ++device) {
            for (auto action=device->begin(), actionn=device->end(); action!=actionn; ++action) {
                action->second.rising_=0;
                action->second.falling_=0;
            }
        }
        // clear queue
        bufferedActions_.clear();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::setDefaultActionMapping(hStringID keybinding, hStringID actionname) {
        auto& defaultMappings_ = impl_->defaultMappings_;
        auto& keyNameToIDMappings_ = impl_->keyNameToIDMappings_;

        auto id = keyNameToIDMappings_.find(keybinding);
        if (id != keyNameToIDMappings_.end()) {
            defaultMappings_[id->second] = actionname;
        }
        impl_->addToBoundMappings(keybinding, actionname);
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    void hActionManager::clearDefaultActionMapping(hStringID keybinding) {
        auto& defaultMappings_ = impl_->defaultMappings_;
        auto& keyNameToIDMappings_ = impl_->keyNameToIDMappings_;

        auto id = keyNameToIDMappings_.find(keybinding);
        if (id != keyNameToIDMappings_.end()) {
            defaultMappings_.erase(id->second);
        }
        impl_->removeFromBoundMappings(keybinding);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::registerLuaLib(hLuaStateManager* luaStateManager) {
        lua_State* L = luaStateManager->GetMainState();

        lua_newtable(L);
        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, hActionManagerImpl::luaSetDefaultActionMapping, 1);
        lua_setfield(L, -2, "set_default_action_mapping");
        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, hActionManagerImpl::luaClearDefaultActionMapping, 1);
        lua_setfield(L, -2, "clear_default_action_mapping");

        
        lua_pushglobaltable(L);
        lua_pushvalue(L, -2);
        lua_setfield(L, -2, "input");

        lua_pop(L, 2);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hActionManager::queryAction(hUint id, hStringID action, hInputAction* value) {
        auto& boundMappings_ = impl_->boundMappings_;

        hcAssert(value);
        if (id > boundMappings_.size()) {
            return hFalse;
        }

        auto m = boundMappings_[id].find(action);
        if (m != boundMappings_[id].end()) {
            *value = m->second;
            return hTrue;
        }
        return hFalse;
    }

}
