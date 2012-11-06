/********************************************************************

	filename: 	DeviceInput.h	
	
	Copyright (c) 23:7:2011 James Moran
	
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
#ifndef DEVICEIINPUT_H__
#define DEVICEIINPUT_H__

#define HEART_INPUT_ANY_PORT (hErrorCode)

namespace Heart
{

    typedef hUint32 hdInputID;

#define HEART_INPUTID_UNDEFINED (hErrorCode)

#define HEART_MOUSE_LBUTTON (0x10000001)
#define HEART_MOUSE_MBUTTON (0x10000002)
#define HEART_MOUSE_RBUTTON (0x10000003)
#define HEART_MOUSE_BUTTON1 (0x10000004)
#define HEART_MOUSE_BUTTON2 (0x10000005)
#define HEART_MOUSE_BUTTON3 (0x10000006)
#define HEART_MOUSE_BUTTON4 (0x10000007)
#define HEART_MOUSE_BUTTON5 (0x10000008)

#ifdef HEART_PLAT_WINDOWS
#   define HEART_PAD_LEFT_TRIGGER   (0x10000009)
#   define HEART_PAD_RIGHT_TRIGGER  (0x1000000A)
#   define HEART_PAD_LEFT_STICKX    (0x1000000B)
#   define HEART_PAD_LEFT_STICKY    (0x1000000C)
#   define HEART_PAD_RIGHT_STICKX   (0x1000000D)
#   define HEART_PAD_RIGHT_STICKY   (0x1000000E)
#   define HEART_PAD_DPAD_UP        XINPUT_GAMEPAD_DPAD_UP	        //0x0001  
#   define HEART_PAD_DPAD_DOWN      XINPUT_GAMEPAD_DPAD_DOWN	    //0x0002 
#   define HEART_PAD_DPAD_LEFT      XINPUT_GAMEPAD_DPAD_LEFT	    //0x0004
#   define HEART_PAD_DPAD_RIGHT     XINPUT_GAMEPAD_DPAD_RIGHT	    //0x0008
#   define HEART_PAD_START          XINPUT_GAMEPAD_START	        //0x0010
#   define HEART_PAD_BACK           XINPUT_GAMEPAD_BACK	            //0x0020
#   define HEART_PAD_LEFT_THUMB     XINPUT_GAMEPAD_LEFT_THUMB	    //0x0040
#   define HEART_PAD_RIGHT_THUMB    XINPUT_GAMEPAD_RIGHT_THUMB	    //0x0080
#   define HEART_PAD_LEFT_SHOULDER  XINPUT_GAMEPAD_LEFT_SHOULDER    //0x0100
#   define HEART_PAD_RIGHT_SHOULDER XINPUT_GAMEPAD_RIGHT_SHOULDER   //0x0200
#   define HEART_PAD_A              XINPUT_GAMEPAD_A	            //0x1000
#   define HEART_PAD_B              XINPUT_GAMEPAD_B	            //0x2000
#   define HEART_PAD_X              XINPUT_GAMEPAD_X	            //0x4000
#   define HEART_PAD_Y              XINPUT_GAMEPAD_Y	            //0x8000
#else
#   error ("Platform not defined") 
#endif

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	enum hdInputButtonState
	{
		hButtonState_IS_UP,
		hButtonState_IS_DOWN = 1,
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    struct HEARTDEV_SLIBEXPORT hdInputButton
	{
		hdInputButton() 
            : buttonState_(0)
		{}
        hdInputButton(const hdInputButton& rhs)
        {
            buttonState_ = rhs.buttonState_;
        }
        union
        {
            struct  
            {
                hBool			buttonVal_		: 1;
                hBool			raisingEdge_	: 1;
                hBool			fallingEdge_	: 1;
                hBool			isRepeat_		: 1;
            };
            hUint32 buttonState_;
        };
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	struct HEARTDEV_SLIBEXPORT hdInputAxis
	{
		hdInputAxis() 
            : anologueVal_( 0.0f ) 
		{}
		hFloat			anologueVal_;
	};

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    struct HEARTDEV_SLIBEXPORT hdInputIDPair
    {
        const hChar*    stringName_;
        hdInputID       inputID_;
    };
}

#endif // DEVICEIINPUT_H__