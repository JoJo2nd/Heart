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

#define HEART_MOUSE_LBUTTON (0x10000001)
#define HEART_MOUSE_MBUTTON (0x10000002)
#define HEART_MOUSE_RBUTTON (0x10000003)
#define HEART_MOUSE_BUTTON1 (0x10000004)
#define HEART_MOUSE_BUTTON2 (0x10000005)
#define HEART_MOUSE_BUTTON3 (0x10000006)
#define HEART_MOUSE_BUTTON4 (0x10000007)
#define HEART_MOUSE_BUTTON5 (0x10000008)

#define XINPUT_GAMEPAD_LEFT_TRIGGER  (0x01000000)
#define XINPUT_GAMEPAD_RIGHT_TRIGGER (0x02000000)
#define XINPUT_GAMEPAD_LEFT_STICKX   (0x03000000)
#define XINPUT_GAMEPAD_LEFT_STICKY   (0x04000000)
#define XINPUT_GAMEPAD_RIGHT_STICKX  (0x05000000)
#define XINPUT_GAMEPAD_RIGHT_STICKY  (0x06000000)

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