/********************************************************************

	filename: 	hActionManager.cpp	
	
	Copyright (c) 1:4:2012 James Moran
	
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

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    hBool hActionManager::initialise(hdSystemWindow* window)
    {
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

        addDeviceRange(0, 0);
        bindDeviceToID(hDeviceID_Keyboard, 0);
        bindDeviceToID(hDeviceID_Mouse, 0);
        bindDeviceToID(hDeviceID_ControllerFirst, 0);

        hSysEventHandler sysEvent = hFUNCTOR_BINDMEMBER(hSysEventHandler, hActionManager, handleSysEvent, this);
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
        for (hSize_t i=0, n=openControllers_.size(); i<n; ++i) {
            DeviceInput::hCloseController(openControllers_[i]);
            openControllers_[i] = hNullptr;
        }
        openControllers_.clear();

        hSysEventHandler sysEvent = hFUNCTOR_BINDMEMBER(hSysEventHandler, hActionManager, handleSysEvent, this);
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

    void hActionManager::handleSysEvent(hUint syseventid, const hSysEvent* sysevent) {
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
            if (mapping != invalid && boundID) {
                hInputAction action = {
                    mapping->second,
                    0, //device ID
                    0, // value
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
                    1,
                    (sysevent->key.repeat > 0),
                    0, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        } else if (syseventid == hGetSystemEventID(TEXTINPUT)) {

        } else if (syseventid == hGetSystemEventID(MOUSEMOTION)) {
            auto boundID = getBoundID(sysevent->motion.which);
            if (sysevent->motion.xrel != 0) {
                hInputID keyid(hInputID::hType_MouseAxis, 0, 0, 0);
                auto invalid = defaultMappings_.end();
                auto mapping = defaultMappings_.find(keyid);
                if (mapping != invalid && boundID != hInvalidBinding) {
                    hInputAction action = {
                        mapping->second,
                        sysevent->motion.which, //device ID
                        sysevent->motion.xrel, // val
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
                    0, // value
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
                    1,
                    0,
                    0, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        }
        
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::setDefaultActionMapping(hStringID keybinding, hStringID actionname) {
        auto id = keyNameToIDMappings_.find(keybinding);
        if (id != keyNameToIDMappings_.end()) {
            defaultMappings_[id->second] = actionname;
        }
        addToBoundMappings(keybinding, actionname);
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    void hActionManager::clearDefaultActionMapping(hStringID keybinding) {
        auto id = keyNameToIDMappings_.find(keybinding);
        if (id != keyNameToIDMappings_.end()) {
            defaultMappings_.erase(id->second);
        }
        removeFromBoundMappings(keybinding);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::registerLuaLib(hLuaStateManager* luaStateManager) {
        lua_State* L = luaStateManager->GetMainState();

        lua_newtable(L);
        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, luaSetDefaultActionMapping, 1);
        lua_setfield(L, -2, "set_default_action_mapping");
        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, luaClearDefaultActionMapping, 1);
        lua_setfield(L, -2, "clear_default_action_mapping");

        
        lua_pushglobaltable(L);
        lua_pushvalue(L, -2);
        lua_setfield(L, -2, "input");

        lua_pop(L, 2);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hInt hActionManager::luaSetDefaultActionMapping(lua_State* L) {
        hActionManager* am = (hActionManager*)lua_topointer(L, lua_upvalueindex(1));
        if (!am) {
            return luaL_error(L, "Unable to grab engine pointer" );
        }
        const hChar* keyname = luaL_checkstring(L, 1);
        const hChar* actionname = luaL_checkstring(L, 2);
        am->setDefaultActionMapping(hStringID(keyname), hStringID(actionname));
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hInt hActionManager::luaClearDefaultActionMapping(lua_State* L) {
        hActionManager* am = (hActionManager*)lua_topointer(L, lua_upvalueindex(1));
        if (!am) {
            return luaL_error(L, "Unable to grab engine pointer" );
        }
        const hChar* keyname = luaL_checkstring(L, 1);
        am->clearDefaultActionMapping(hStringID(keyname));
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::addDeviceRange(hUint first, hUint last) {
        boundMappings_.resize(last+1);
        flushBoundMappings();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::flushBoundMappings() {
        for (auto device=boundMappings_.begin(), devicen=boundMappings_.end(); device!=devicen; ++device) {
            device->clear();
            auto invalid = device->end();
            for (auto i=defaultMappings_.cbegin(), n=defaultMappings_.cend(); i!=n; ++i) {
                hInputAction action = { i->second };
                (*device)[i->second] = action;
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::addToBoundMappings(hStringID keybinding, hStringID actionname) {
        for (auto device=boundMappings_.begin(), devicen=boundMappings_.end(); device!=devicen; ++device) {
            hInputAction action = { actionname };
            (*device)[keybinding] = action;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::removeFromBoundMappings(hStringID keybinding) {
        for (auto device=boundMappings_.begin(), devicen=boundMappings_.end(); device!=devicen; ++device) {
            device->erase(keybinding);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::bindDeviceToID(hUint deviceid, hUint id) {
        if (deviceid >= deviceIDToBoundID_.size()) {
            deviceIDToBoundID_.resize(deviceid+1, hInvalidBinding);
        }
        deviceIDToBoundID_[deviceid] = id;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hActionManager::queryAction(hUint id, hStringID action, hInputAction* value) {
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
