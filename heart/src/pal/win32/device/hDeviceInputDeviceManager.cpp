/********************************************************************

	filename: 	DeviceInputDeviceManager.cpp	
	
	Copyright (c) 31:3:2012 James Moran
	
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

#define hdVKEY(x) {#x, x}

    const hdInputIDPair hdInputDeviceManager::s_deviceInputPairs[] =
    {
        /*
         * Virtual Keys, Standard Set
         */
        hdVKEY(VK_LBUTTON        ),  //0x01
        hdVKEY(VK_RBUTTON        ),  //0x02
        hdVKEY(VK_CANCEL         ),  //0x03
        hdVKEY(VK_MBUTTON        ),  //0x04    /* NOT contiguous with L & RBUTTON */

#if(_WIN32_WINNT >= 0x0500)
        hdVKEY(VK_XBUTTON1       ),  //0x05    /* NOT contiguous with L & RBUTTON */
        hdVKEY(VK_XBUTTON2       ),  //0x06    /* NOT contiguous with L & RBUTTON */
#endif /* _WIN32_WINNT >= 0x0500 */

        /*
         * 0x07 : unassigned
         */
        hdVKEY(VK_BACK           ),  //0x08
        hdVKEY(VK_TAB            ),  //0x09

        /*
         * 0x0A - 0x0B : reserved
         */
        hdVKEY(VK_CLEAR          ),  //0x0C
        hdVKEY(VK_RETURN         ),  //0x0D

        hdVKEY(VK_SHIFT          ),  //0x10
        hdVKEY(VK_CONTROL        ),  //0x11
        hdVKEY(VK_MENU           ),  //0x12
        hdVKEY(VK_PAUSE          ),  //0x13
        hdVKEY(VK_CAPITAL        ),  //0x14

        hdVKEY(VK_KANA           ),  //0x15
        hdVKEY(VK_HANGEUL        ),  //0x15  /* old name - should be here for compatibility */
        hdVKEY(VK_HANGUL         ),  //0x15
        hdVKEY(VK_JUNJA          ),  //0x17
        hdVKEY(VK_FINAL          ),  //0x18
        hdVKEY(VK_HANJA          ),  //0x19
        hdVKEY(VK_KANJI          ),  //0x19

        hdVKEY(VK_ESCAPE         ),  //0x1B

        hdVKEY(VK_CONVERT        ),  //0x1C
        hdVKEY(VK_NONCONVERT     ),  //0x1D
        hdVKEY(VK_ACCEPT         ),  //0x1E
        hdVKEY(VK_MODECHANGE     ),  //0x1F

        hdVKEY(VK_SPACE          ),  //0x20
        hdVKEY(VK_PRIOR          ),  //0x21
        hdVKEY(VK_NEXT           ),  //0x22
        hdVKEY(VK_END            ),  //0x23
        hdVKEY(VK_HOME           ),  //0x24
        hdVKEY(VK_LEFT           ),  //0x25
        hdVKEY(VK_UP             ),  //0x26
        hdVKEY(VK_RIGHT          ),  //0x27
        hdVKEY(VK_DOWN           ),  //0x28
        hdVKEY(VK_SELECT         ),  //0x29
        hdVKEY(VK_PRINT          ),  //0x2A
        hdVKEY(VK_EXECUTE        ),  //0x2B
        hdVKEY(VK_SNAPSHOT       ),  //0x2C
        hdVKEY(VK_INSERT         ),  //0x2D
        hdVKEY(VK_DELETE         ),  //0x2E
        hdVKEY(VK_HELP           ),  //0x2F

        /*
         * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
         * 0x40 : unassigned
         * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
         */
        hdVKEY(VK_LWIN           ),  //0x5B
        hdVKEY(VK_RWIN           ),  //0x5C
        hdVKEY(VK_APPS           ),  //0x5D

        /*
         * 0x5E : reserved
         */
        hdVKEY(VK_SLEEP          ),  //0x5F

        hdVKEY(VK_NUMPAD0        ),  //0x60
        hdVKEY(VK_NUMPAD1        ),  //0x61
        hdVKEY(VK_NUMPAD2        ),  //0x62
        hdVKEY(VK_NUMPAD3        ),  //0x63
        hdVKEY(VK_NUMPAD4        ),  //0x64
        hdVKEY(VK_NUMPAD5        ),  //0x65
        hdVKEY(VK_NUMPAD6        ),  //0x66
        hdVKEY(VK_NUMPAD7        ),  //0x67
        hdVKEY(VK_NUMPAD8        ),  //0x68
        hdVKEY(VK_NUMPAD9        ),  //0x69
        hdVKEY(VK_MULTIPLY       ),  //0x6A
        hdVKEY(VK_ADD            ),  //0x6B
        hdVKEY(VK_SEPARATOR      ),  //0x6C
        hdVKEY(VK_SUBTRACT       ),  //0x6D
        hdVKEY(VK_DECIMAL        ),  //0x6E
        hdVKEY(VK_DIVIDE         ),  //0x6F
        hdVKEY(VK_F1             ),  //0x70
        hdVKEY(VK_F2             ),  //0x71
        hdVKEY(VK_F3             ),  //0x72
        hdVKEY(VK_F4             ),  //0x73
        hdVKEY(VK_F5             ),  //0x74
        hdVKEY(VK_F6             ),  //0x75
        hdVKEY(VK_F7             ),  //0x76
        hdVKEY(VK_F8             ),  //0x77
        hdVKEY(VK_F9             ),  //0x78
        hdVKEY(VK_F10            ),  //0x79
        hdVKEY(VK_F11            ),  //0x7A
        hdVKEY(VK_F12            ),  //0x7B
        hdVKEY(VK_F13            ),  //0x7C
        hdVKEY(VK_F14            ),  //0x7D
        hdVKEY(VK_F15            ),  //0x7E
        hdVKEY(VK_F16            ),  //0x7F
        hdVKEY(VK_F17            ),  //0x80
        hdVKEY(VK_F18            ),  //0x81
        hdVKEY(VK_F19            ),  //0x82
        hdVKEY(VK_F20            ),  //0x83
        hdVKEY(VK_F21            ),  //0x84
        hdVKEY(VK_F22            ),  //0x85
        hdVKEY(VK_F23            ),  //0x86
        hdVKEY(VK_F24            ),  //0x87

        /*
         * ),  //0x88 - ),  //0x8F : unassigned
         */
        hdVKEY(VK_NUMLOCK        ),  //0x90
        hdVKEY(VK_SCROLL         ),  //0x91

        /*
         * NEC PC-9800 kbd definitions
         */
        hdVKEY(VK_OEM_NEC_EQUAL  ),  //0x92   // '=' key on numpad

        /*
         * Fujitsu/OASYS kbd definitions
         */
        hdVKEY(VK_OEM_FJ_JISHO   ),  //0x92   // 'Dictionary' key
        hdVKEY(VK_OEM_FJ_MASSHOU ),  //0x93   // 'Unregister word' key
        hdVKEY(VK_OEM_FJ_TOUROKU ),  //0x94   // 'Register word' key
        hdVKEY(VK_OEM_FJ_LOYA    ),  //0x95   // 'Left OYAYUBI' key
        hdVKEY(VK_OEM_FJ_ROYA    ),  //0x96   // 'Right OYAYUBI' key

        /*
         * ),  //0x97 - ),  //0x9F : unassigned
         */

        /*
         * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
         * Used only as parameters to GetAsyncKeyState() and GetKeyState().
         * No other API or message will distinguish left and right keys in this way.
         */
        hdVKEY(VK_LSHIFT         ),  //0xA0
        hdVKEY(VK_RSHIFT         ),  //0xA1
        hdVKEY(VK_LCONTROL       ),  //0xA2
        hdVKEY(VK_RCONTROL       ),  //0xA3
        hdVKEY(VK_LMENU          ),  //0xA4
        hdVKEY(VK_RMENU          ),  //0xA5

#if(_WIN32_WINNT >= 0x0500)
        hdVKEY(VK_BROWSER_BACK        ),  //0xA6
        hdVKEY(VK_BROWSER_FORWARD     ),  //0xA7
        hdVKEY(VK_BROWSER_REFRESH     ),  //0xA8
        hdVKEY(VK_BROWSER_STOP        ),  //0xA9
        hdVKEY(VK_BROWSER_SEARCH      ),  //0xAA
        hdVKEY(VK_BROWSER_FAVORITES   ),  //0xAB
        hdVKEY(VK_BROWSER_HOME        ),  //0xAC

        hdVKEY(VK_VOLUME_MUTE         ),  //0xAD
        hdVKEY(VK_VOLUME_DOWN         ),  //0xAE
        hdVKEY(VK_VOLUME_UP           ),  //0xAF
        hdVKEY(VK_MEDIA_NEXT_TRACK    ),  //0xB0
        hdVKEY(VK_MEDIA_PREV_TRACK    ),  //0xB1
        hdVKEY(VK_MEDIA_STOP          ),  //0xB2
        hdVKEY(VK_MEDIA_PLAY_PAUSE    ),  //0xB3
        hdVKEY(VK_LAUNCH_MAIL         ),  //0xB4
        hdVKEY(VK_LAUNCH_MEDIA_SELECT ),  //0xB5
        hdVKEY(VK_LAUNCH_APP1         ),  //0xB6
        hdVKEY(VK_LAUNCH_APP2         ),  //0xB7

#endif /* _WIN32_WINNT >= 0x0500 */

        /*
         * 0xB8 - 0xB9 : reserved
         */
        hdVKEY(VK_OEM_1          ),  //0xBA   // ';:' for US
        hdVKEY(VK_OEM_PLUS       ),  //0xBB   // '+' any country
        hdVKEY(VK_OEM_COMMA      ),  //0xBC   // ',' any country
        hdVKEY(VK_OEM_MINUS      ),  //0xBD   // '-' any country
        hdVKEY(VK_OEM_PERIOD     ),  //0xBE   // '.' any country
        hdVKEY(VK_OEM_2          ),  //0xBF   // '/?' for US
        hdVKEY(VK_OEM_3          ),  //0xC0   // '`~' for US

        /*
         * 0xC1 - 0xD7 : reserved
         */

        /*
         * 0xD8 - 0xDA : unassigned
         */
        hdVKEY(VK_OEM_4          ),  //0xDB  //  '[{' for US
        hdVKEY(VK_OEM_5          ),  //0xDC  //  '\|' for US
        hdVKEY(VK_OEM_6          ),  //0xDD  //  ']}' for US
        hdVKEY(VK_OEM_7          ),  //0xDE  //  ''"' for US
        hdVKEY(VK_OEM_8          ),  //0xDF

        /*
         * ),  //0xE0 : reserved
         */

        /*
         * Various extended or enhanced keyboards
         */
        hdVKEY(VK_OEM_AX         ),  //0xE1  //  'AX' key on Japanese AX kbd
        hdVKEY(VK_OEM_102        ),  //0xE2  //  "<>" or "\|" on RT 102-key kbd.
        hdVKEY(VK_ICO_HELP       ),  //0xE3  //  Help key on ICO
        hdVKEY(VK_ICO_00         ),  //0xE4  //  00 key on ICO

#if(WINVER >= 0x0400)
        hdVKEY(VK_PROCESSKEY     ),  //0xE5
#endif /* WINVER >= ),  //0x0400 */

        hdVKEY(VK_ICO_CLEAR      ),  //0xE6


#if(_WIN32_WINNT >= 0x0500)
        hdVKEY(VK_PACKET         ),  //0xE7
#endif /* _WIN32_WINNT >= 0x0500 */

        /*
         * 0xE8 : unassigned
         */

        /*
         * Nokia/Ericsson definitions
         */
        hdVKEY(VK_OEM_RESET      ),  //0xE9
        hdVKEY(VK_OEM_JUMP       ),  //0xEA
        hdVKEY(VK_OEM_PA1        ),  //0xEB
        hdVKEY(VK_OEM_PA2        ),  //0xEC
        hdVKEY(VK_OEM_PA3        ),  //0xED
        hdVKEY(VK_OEM_WSCTRL     ),  //0xEE
        hdVKEY(VK_OEM_CUSEL      ),  //0xEF
        hdVKEY(VK_OEM_ATTN       ),  //0xF0
        hdVKEY(VK_OEM_FINISH     ),  //0xF1
        hdVKEY(VK_OEM_COPY       ),  //0xF2
        hdVKEY(VK_OEM_AUTO       ),  //0xF3
        hdVKEY(VK_OEM_ENLW       ),  //0xF4
        hdVKEY(VK_OEM_BACKTAB    ),  //0xF5

        hdVKEY(VK_ATTN           ),  //0xF6
        hdVKEY(VK_CRSEL          ),  //0xF7
        hdVKEY(VK_EXSEL          ),  //0xF8
        hdVKEY(VK_EREOF          ),  //0xF9
        hdVKEY(VK_PLAY           ),  //0xFA
        hdVKEY(VK_ZOOM           ),  //0xFB
        hdVKEY(VK_NONAME         ),  //0xFC
        hdVKEY(VK_PA1            ),  //0xFD
        hdVKEY(VK_OEM_CLEAR      ),  //0xFE

/*
 * 0xFF : reserved
 */
        hdVKEY(HEART_MOUSE_LBUTTON),
        hdVKEY(HEART_MOUSE_MBUTTON),
        hdVKEY(HEART_MOUSE_RBUTTON),
        hdVKEY(HEART_MOUSE_BUTTON1),
        hdVKEY(HEART_MOUSE_BUTTON2),
        hdVKEY(HEART_MOUSE_BUTTON3),
        hdVKEY(HEART_MOUSE_BUTTON4),
        hdVKEY(HEART_MOUSE_BUTTON5),

        hdVKEY(HEART_PAD_LEFT_TRIGGER),  
        hdVKEY(HEART_PAD_RIGHT_TRIGGER), 
        hdVKEY(HEART_PAD_LEFT_STICKX),   
        hdVKEY(HEART_PAD_LEFT_STICKY),   
        hdVKEY(HEART_PAD_RIGHT_STICKX),  
        hdVKEY(HEART_PAD_RIGHT_STICKY),  
        hdVKEY(HEART_PAD_DPAD_UP),       
        hdVKEY(HEART_PAD_DPAD_DOWN),     
        hdVKEY(HEART_PAD_DPAD_LEFT),     
        hdVKEY(HEART_PAD_DPAD_RIGHT),    
        hdVKEY(HEART_PAD_START),         
        hdVKEY(HEART_PAD_BACK),          
        hdVKEY(HEART_PAD_LEFT_THUMB),    
        hdVKEY(HEART_PAD_RIGHT_THUMB),   
        hdVKEY(HEART_PAD_LEFT_SHOULDER), 
        hdVKEY(HEART_PAD_RIGHT_SHOULDER),
        hdVKEY(HEART_PAD_A),             
        hdVKEY(HEART_PAD_B),             
        hdVKEY(HEART_PAD_X),             
        hdVKEY(HEART_PAD_Y),    

    };

#undef hdVKEY
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hdInputDeviceManager::hdInputDeviceManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hdInputDeviceManager::~hdInputDeviceManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool hdInputDeviceManager::Initialise(hdSystemWindow* window)
	{
        systemWindow_ = window;
#ifndef HEART_USE_SDL2
        keyboard_ = systemWindow_->GetSystemKeyboard();
        mouse_    = systemWindow_->GetSystemMouse();
#endif

        for (hUint32 i = 0; i < HEART_MAX_GAMEPADS; ++i)
        {
            pads_[i].ConnectPad(i);
        }

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

        hSysEventHandler sysEvent = hFUNCTOR_BINDMEMBER(hSysEventHandler, hdInputDeviceManager, handleSysEvent, this);
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

	void hdInputDeviceManager::Destory()
	{
        hSysEventHandler sysEvent = hFUNCTOR_BINDMEMBER(hSysEventHandler, hdInputDeviceManager, handleSysEvent, this);
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

	void hdInputDeviceManager::Update()
	{
#ifndef HEART_USE_SDL2
		keyboard_->Update();
        for (hUint32 i = 0; i < HEART_MAX_GAMEPADS; ++i)
        {
            pads_[i].Update();
        }
#endif
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hdInputDeviceManager::GetInputIDPairCount()
    {
#ifdef HEART_USE_SDL2
        return 0;
#else
        return sizeof(s_deviceInputPairs)/sizeof(hdInputIDPair);
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hdInputIDPair* hdInputDeviceManager::GetInputIDPairArray()
    {
#ifdef HEART_USE_SDL2
        return 0;
#else
        return s_deviceInputPairs;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdInputDeviceManager::endOfFrameUpdate()
    {
#ifndef HEART_USE_SDL2
        keyboard_->EndOfFrameUpdate();
        mouse_->EndOfFrameUpdate();
        for (hUint32 i = 0; i < HEART_MAX_GAMEPADS; ++i)
        {
            pads_[i].EndOfFrameUpdate();
        }
#else
        bufferedActions_.clear();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdInputDeviceManager::handleSysEvent(hUint syseventid, const hSysEvent* sysevent) {
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
#ifdef HEART_USE_SDL2
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdInputDeviceManager::setDefaultActionMapping(hStringID keybinding, hStringID actionname) {
        auto id = keyNameToIDMappings_.find(keybinding);
        if (id != keyNameToIDMappings_.end()) {
            defaultMappings_[id->second] = actionname;
        }
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    void hdInputDeviceManager::clearDefaultActionMapping(hStringID keybinding) {
        auto id = keyNameToIDMappings_.find(keybinding);
        if (id != keyNameToIDMappings_.end()) {
            defaultMappings_.erase(id->second);
        }
    }

namespace DeviceInput
{
    hUint hGetKeyboardFirstScancode() {
        return SDL_SCANCODE_UNKNOWN;
    }
    hUint hGetKeyboardLastScancode() {
        return SDL_NUM_SCANCODES;
    }
    hStringID hGetScancodeName(hUint scancode) {
        return hStringID(SDL_GetScancodeName((SDL_Scancode)scancode));
    }
    hUint hGetMouseFirstButtonID() {
        return SDL_BUTTON_LEFT;
    }
    hUint hGetMouseLastButtonID() {
        return SDL_BUTTON_X2;
    }
    hStringID hGetMouseButtonName(hUint buttonid) {
        static hStringID mouseleft("Mouse Left");
        static hStringID mousemiddle("Mouse Middle");
        static hStringID mouseright("Mouse Right");
        static hStringID mousex1("Mouse X1");
        static hStringID mousex2("Mouse X2");
        switch(buttonid) {
        case SDL_BUTTON_LEFT: return mouseleft;
        case SDL_BUTTON_MIDDLE: return mousemiddle;
        case SDL_BUTTON_RIGHT: return mouseright;
        case SDL_BUTTON_X1: return mousex1;
        case SDL_BUTTON_X2: return mousex2;
        }
        return hStringID();
    }
    hUint hGetMouseFirstAxisID() {
        return 0;
    }
    hUint hGetMouseLastAxisID() {
        return 3;
    }
    hStringID hGetMouseAxisName(hUint axisid) {
        static hStringID xaxis("Mouse X");
        static hStringID yaxis("Mouse Y");
        static hStringID wheelxaxis("Mouse Wheel X");
        static hStringID wheelyaxis("Mouse Wheel Y");
        switch(axisid) {
        case 0: return xaxis;
        case 1: return yaxis;
        case 2: return wheelxaxis;
        case 3: return wheelyaxis;
        }
        return hStringID();
    }
    hUint hGetControllerFirstButtonID() {
        return SDL_CONTROLLER_BUTTON_A;
    }
    hUint hGetControllerLastButtonID() {
        return SDL_CONTROLLER_BUTTON_MAX;
    }
    hStringID hGetControllerButtonName(hUint buttonid) {
        const hChar* name = SDL_GameControllerGetStringForButton((SDL_GameControllerButton)buttonid);
        return name ? hStringID(name) : hStringID();
    }
    hUint hGetControllerFirstAxisID() {
        return SDL_CONTROLLER_AXIS_LEFTX;
    }
    hUint hGetControllerLastAxisID() {
        return SDL_CONTROLLER_AXIS_MAX;
    }
    hStringID hGetControllerAxisName(hUint axisid) {
        const hChar* name = SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)axisid);
        return name ? hStringID(name) : hStringID();
    }
}
#endif
}
