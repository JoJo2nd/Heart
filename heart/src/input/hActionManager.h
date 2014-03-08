/********************************************************************

	filename: 	hActionManager.h	
	
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
#pragma once

#ifndef HACTIONMANAGER_H__
#define HACTIONMANAGER_H__


namespace Heart
{
    class hLuaStateManager;

    struct hInputAction
    {
        hStringID   actionName_;
        hUint       deviceID_;
        hInt16      value_;
        hUint       rising_ : 1; // aka button down
        hUint       falling_ : 1; // aka button up
        hUint       isRepeat_ : 1;
        hUint       isAxis_ : 1;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    class HEART_DLLEXPORT hActionManager
    {
    public:

        hActionManager() {}
        ~hActionManager() {}

        hBool       initialise(hdSystemWindow* window);
        void        destory();
        void        update();
        void        endOfFrameUpdate();
        void        setDefaultActionMapping(hStringID keyname, hStringID actionname);
        void        clearDefaultActionMapping(hStringID keyname);
        void        registerLuaLib(hLuaStateManager* luaStateManager);
        hBool       queryAction(hUint id, hStringID action, hInputAction* value);

    private:

        enum hDeviceID 
        {
            hDeviceID_Keyboard = 0,
            hDeviceID_Mouse = 1,
            hDeviceID_ControllerFirst = 2
        };

        static const hUint hInvalidBinding = ~0;

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
                };
            };

            hInputID() {}
            hInputID(hType type, hUint scancode, hUint buttonid, hUint axisid)
                : type_(type), scancode_(scancode), buttonId_(buttonid), axisId_(axisid)
            {}

            hBool operator == (const hInputID& rhs) const { return id_ == rhs.id_; }
            hBool operator != (const hInputID& rhs) const { return id_ != rhs.id_; }
            hBool operator < (const hInputID& rhs) const { return id_ < rhs.id_; }
        };

        struct hInputIDHash
        {
            hSize_t operator()(const hInputID& rhs) const { return rhs.id_; }
        };

        void handleSysEvent(hUint syseventid, const hSysEvent* sysevent);
        void addDeviceRange(hUint first, hUint last);
        void flushBoundMappings();
        void addToBoundMappings(hStringID keybinding, hStringID actionname);
        void removeFromBoundMappings(hStringID keybinding);
        void bindDeviceToID(hUint deviceid, hUint id);
        hFORCEINLINE hUint getBoundID(hUint deviceid) {
            return deviceIDToBoundID_[deviceid];
        }
        static hInt luaSetDefaultActionMapping(lua_State* L);
        static hInt luaClearDefaultActionMapping(lua_State* L);

        typedef std::unordered_map<hInputID, hStringID, hInputIDHash> hActionMapping;
        typedef std::unordered_map<hStringID, hInputID> hKeyNameIDMapping;
        typedef std::unordered_map<hStringID, hInputAction> hActionInputMapping;
        typedef std::vector<hInputAction> hActionArray;
        typedef std::vector<DeviceInput::hGameController*> hControllerArray;
        typedef std::vector<hActionInputMapping> hBoundMappingsArray;
        typedef std::vector<hUint> hIntArray;

        hdSystemWindow*         systemWindow_;
        hKeyNameIDMapping       keyNameToIDMappings_;
        hActionMapping          defaultMappings_;
        hActionArray            bufferedActions_;
        hControllerArray        openControllers_;
        hBoundMappingsArray    boundMappings_;
        hIntArray               deviceIDToBoundID_;
    };
}
#endif // HACTIONMANAGER_H__
