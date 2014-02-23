/********************************************************************

    filename:   hDeviceInputDeviceManager.h  
    
    Copyright (c) 22:2:2014 James Moran
    
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
#ifndef DEVICEINPUTDEVICEMANAGER_H__
#define DEVICEINPUTDEVICEMANAGER_H__

namespace Heart
{
    class hdSystemWindow;
namespace DeviceInput
{
    hUint hGetKeyboardFirstScancode();
    hUint hGetKeyboardLastScancode();
    hStringID hGetScancodeName(hUint scancode);
    hUint hGetMouseFirstButtonID();
    hUint hGetMouseLastButtonID();
    hStringID hGetMouseButtonName(hUint buttonid);
    hUint hGetMouseFirstAxisID();
    hUint hGetMouseLastAxisID();
    hStringID hGetMouseAxisName(hUint axisid);
    hUint hGetControllerFirstButtonID();
    hUint hGetControllerLastButtonID();
    hStringID hGetControllerButtonName(hUint buttonid);
    hUint hGetControllerFirstAxisID();
    hUint hGetControllerLastAxisID();
    hStringID hGetControllerAxisName(hUint axisid);
}

    struct hInputAction
    {
        hStringID   actionName_;
        hUint       deviceID_;
        hInt16      value_;
        hUint       isRepeat_ : 1;
        hUint       isAxis_ : 1;
    };

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	class HEART_DLLEXPORT hdInputDeviceManager
	{
	public:

#define HEART_MAX_GAMEPADS (4)

		hdInputDeviceManager();
		virtual ~hdInputDeviceManager();

		hBool				            Initialise(hdSystemWindow* window);
		void				            Destory();
		void				            Update();
        void                            endOfFrameUpdate();
        hdKeyboard*                     GetSystemKeyboard() const { return keyboard_; }
        hdMouse*                        GetSystemMouse() const { return mouse_; }
        hdGamepad*                      GetGamepad(hUint32 padIdx) { return padIdx < HEART_MAX_GAMEPADS ? pads_+padIdx : NULL; }

        static hUint32                  GetInputIDPairCount();
        static const hdInputIDPair*     GetInputIDPairArray();

#ifdef HEART_USE_SDL2
        void setDefaultActionMapping(hStringID keyname, hStringID actionname);
        void clearDefaultActionMapping(hStringID keyname);
#endif

	private:

        static const hdInputIDPair      s_deviceInputPairs[];
#ifdef HEART_USE_SDL2
        struct hInputID {
            enum hType {
                hType_Keyboard = 0,
                hType_Mouse = 1,
                hType_Controller = 2,
            };
            // bits 0-1: device type
            // bits 2-11: scan code empty on non-keyboard devices
            union {
                hUint32 id_; 
                struct {
                    hUint type_ : 2; // 0 = keyboard, 1 = mouse, 2 = controller
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

        typedef std::unordered_map<hInputID, hStringID, hInputIDHash> hActionMapping;
        typedef std::unordered_map<hStringID, hInputID, hStringIDHash> hKeyNameIDMapping;
        typedef std::vector<hInputAction> hActionArray;

        hKeyNameIDMapping   keyNameToIDMappings_;
        hActionMapping      defaultMappings_;
        hActionArray        bufferedActions_;
#endif

        hdSystemWindow* systemWindow_;
		hdKeyboard*		keyboard_;
        hdMouse*        mouse_;
        hdGamepad       pads_[HEART_MAX_GAMEPADS];
		//TODO add game pad
	};

}

#endif // DEVICEINPUTDEVICEMANAGER_H__