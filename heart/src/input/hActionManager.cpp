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
                keyNameToIDMappings_.insert(hKeyNameIDMapping::value_type(name, hInputID(hInputID::hType_Mouse, 0, 0, i)));
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
                keyNameToIDMappings_.insert(hKeyNameIDMapping::value_type(name, hInputID(hInputID::hType_Controller, 0, 0, i)));
            }
        }

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

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::endOfFrameUpdate()
    {
        bufferedActions_.clear();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hActionManager::handleSysEvent(hUint syseventid, const hSysEvent* sysevent) {
        if (syseventid == hGetSystemEventID(CONTROLLERDEVICEADDED)) {
            hcPrintf("Controller added");
        } else if (syseventid == hGetSystemEventID(CONTROLLERDEVICEREMOVED)) {
            hcPrintf("Controller removed");
        } else if (syseventid == hGetSystemEventID(KEYDOWN)) {
            hInputID keyid(hInputID::hType_Keyboard, sysevent->key.keysym.scancode, 0, 0);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            if (mapping != invalid) {
                hInputAction action = {
                    mapping->second,
                    0, //device ID
                    1, // one for down
                    (sysevent->key.repeat > 0),
                    0, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        } else if (syseventid == hGetSystemEventID(KEYUP)) {
            hInputID keyid(hInputID::hType_Keyboard, sysevent->key.keysym.scancode, 0, 0);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            if (mapping != invalid) {
                hInputAction action = {
                    mapping->second,
                    0, //device ID
                    0, // zero for up
                    (sysevent->key.repeat > 0),
                    0, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        } else if (syseventid == hGetSystemEventID(TEXTINPUT)) {

        } else if (syseventid == hGetSystemEventID(MOUSEMOTION)) {
            if (sysevent->motion.xrel != 0) {
                hInputID keyid(hInputID::hType_Mouse, 0, 0, 0);
                auto invalid = defaultMappings_.end();
                auto mapping = defaultMappings_.find(keyid);
                if (mapping != invalid) {
                    hInputAction action = {
                        mapping->second,
                        sysevent->motion.which, //device ID
                        sysevent->motion.xrel, // val
                        0,
                        1,
                    };
                    bufferedActions_.emplace_back(action);
                }
            }
            if (sysevent->motion.yrel != 0) {
                hInputID keyid(hInputID::hType_Mouse, 0, 0, 1);
                auto invalid = defaultMappings_.end();
                auto mapping = defaultMappings_.find(keyid);
                if (mapping != invalid) {
                    hInputAction action = {
                        mapping->second,
                        sysevent->motion.which, //device ID
                        sysevent->motion.yrel, // val
                        0,
                        1,
                    };
                    bufferedActions_.emplace_back(action);
                }
            }
        } else if (syseventid == hGetSystemEventID(MOUSEWHEEL)) {
            if (sysevent->wheel.x != 0) {
                hInputID keyid(hInputID::hType_Mouse, 0, 0, 2);
                auto invalid = defaultMappings_.end();
                auto mapping = defaultMappings_.find(keyid);
                if (mapping != invalid) {
                    hInputAction action = {
                        mapping->second,
                        sysevent->wheel.which, //device ID
                        sysevent->wheel.x, // val
                        0,
                        1,
                    };
                    bufferedActions_.emplace_back(action);
                }
            }
            if (sysevent->wheel.y != 0) {
                hInputID keyid(hInputID::hType_Mouse, 0, 0, 3);
                auto invalid = defaultMappings_.end();
                auto mapping = defaultMappings_.find(keyid);
                if (mapping != invalid) {
                    hInputAction action = {
                        mapping->second,
                        sysevent->wheel.which, //device ID
                        sysevent->wheel.y, // val
                        0,
                        1,
                    };
                    bufferedActions_.emplace_back(action);
                }
            }
        } else if (syseventid == hGetSystemEventID(MOUSEBUTTONDOWN)) {
            hInputID keyid(hInputID::hType_Mouse, 0, sysevent->button.button, 0);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            if (mapping != invalid) {
                hInputAction action = {
                    mapping->second,
                    sysevent->button.which, //device ID
                    1, // zero for up
                    0,
                    0, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        } else if (syseventid == hGetSystemEventID(MOUSEBUTTONUP)) {
            hInputID keyid(hInputID::hType_Mouse, 0, sysevent->cbutton.button, 0);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            if (mapping != invalid) {
                hInputAction action = {
                    mapping->second,
                    sysevent->button.which, //device ID
                    0, // zero for up
                    0,
                    0, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        } else if (syseventid == hGetSystemEventID(CONTROLLERAXISMOTION)) {
            hInputID keyid(hInputID::hType_Controller, 0, 0, sysevent->caxis.axis);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            if (mapping != invalid) {
                hInputAction action = {
                    mapping->second,
                    sysevent->caxis.which, //device ID
                    sysevent->caxis.value, // axis val
                    0,
                    1, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        } else if (syseventid == hGetSystemEventID(CONTROLLERBUTTONDOWN)) {
            hInputID keyid(hInputID::hType_Controller, 0, sysevent->cbutton.button, 0);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            if (mapping != invalid) {
                hInputAction action = {
                    mapping->second,
                    sysevent->cbutton.which, //device ID
                    1, // zero for up
                    0,
                    0, // no axis on keyboard
                };
                bufferedActions_.emplace_back(action);
            }
        } else if (syseventid == hGetSystemEventID(CONTROLLERBUTTONUP)) {
            hInputID keyid(hInputID::hType_Controller, 0, sysevent->cbutton.button, 0);
            auto invalid = defaultMappings_.end();
            auto mapping = defaultMappings_.find(keyid);
            if (mapping != invalid) {
                hInputAction action = {
                    mapping->second,
                    sysevent->cbutton.which, //device ID
                    0, // zero for up
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
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    void hActionManager::clearDefaultActionMapping(hStringID keybinding) {
        auto id = keyNameToIDMappings_.find(keybinding);
        if (id != keyNameToIDMappings_.end()) {
            defaultMappings_.erase(id->second);
        }
    }

}
