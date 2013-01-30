/********************************************************************

	filename: 	hDeviceGamepad.cpp	
	
	Copyright (c) 5:11:2012 James Moran
	
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

    hdInputButton hdGamepad::GetButton(hUint32 buttonID) const
    {
        for (hUint32 i = 0; i < HEART_DEVICE_BUTTON_COUNT; ++i)
        {
            if (buttonID == (1 << i)) return buttons_[i];
        }
        
        return hdInputButton();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdInputAxis hdGamepad::GetAxis(hUint32 axisID) const
    {
        if (axisID < HEART_PAD_LEFT_TRIGGER && axisID > HEART_PAD_RIGHT_STICKY) return hdInputAxis();

        axisID -= HEART_PAD_LEFT_TRIGGER;
        return axes_[axisID & 0x0FFFFFFF];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdGamepad::ConnectPad(hUint32 idx)
    {
        if (idx < HEART_MAX_GAMEPADS) padIdx_ = idx;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdGamepad::Update()
    {
        HRESULT hr;

        if (padIdx_ == hErrorCode) return;

        hZeroMem(&state_, sizeof(state_));
        hr = XInputGetState(padIdx_, &state_);

        if (hr == ERROR_SUCCESS)
        {
            connected_ = hTrue;
            for (hUint32 i = 0; i < HEART_DEVICE_BUTTON_COUNT; ++i)
            {
                hBool bstate = ((state_.Gamepad.wButtons & (1 << i)) > 0);
                buttons_[i].raisingEdge_ = (bstate && buttons_[i].buttonVal_ != bstate) ? hTrue : hFalse;
                buttons_[i].fallingEdge_ = (!bstate && buttons_[i].buttonVal_ != bstate) ? hTrue : hFalse;
                buttons_[i].buttonVal_ = bstate;
            }

// HEART_PAD_LEFT_TRIGGER   
            hFloat axisVal;
            axisVal = hMax((hFloat)state_.Gamepad.bLeftTrigger - XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 0.f);
            axisVal /= 255.0f - XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
            axisVal = hMin(axisVal, 1.0f);
            axes_[0].anologueVal_ = axisVal;
// HEART_PAD_RIGHT_TRIGGER  
            axisVal = hMax((hFloat)state_.Gamepad.bRightTrigger- XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 0.f);
            axisVal /= 255.0f - XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
            axisVal = hMin(axisVal, 1.0f);
            axes_[1].anologueVal_ = axisVal;
// HEART_PAD_LEFT_STICKX   
            axisVal = hMax(fabs((hFloat)state_.Gamepad.sThumbLX) - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, 0.f);
            axisVal /= 32767.0f - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
            axisVal = hMin(axisVal, 1.0f);
            axisVal *= state_.Gamepad.sThumbLX < 0 ? -1.f : 1.f;
            axes_[2].anologueVal_ = axisVal;
// HEART_PAD_LEFT_STICKY    
            axisVal = hMax(fabs((hFloat)state_.Gamepad.sThumbLY) - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, 0.f);
            axisVal /= 32767.0f - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
            axisVal = hMin(axisVal, 1.0f);
            axisVal *= state_.Gamepad.sThumbLY < 0 ? -1.f : 1.f;
            axes_[3].anologueVal_ = axisVal;
// HEART_PAD_RIGHT_STICKX   
            axisVal = hMax(fabs((hFloat)state_.Gamepad.sThumbRX) - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, 0.f);
            axisVal /= 32767.0f - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
            axisVal = hMin(axisVal, 1.0f);
            axisVal *= state_.Gamepad.sThumbRX < 0 ? -1.f : 1.f;
            axes_[4].anologueVal_ = axisVal;
// HEART_PAD_RIGHT_STICKY   
            axisVal = hMax(fabs((hFloat)state_.Gamepad.sThumbRY) - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, 0.f);
            axisVal /= 32767.0f - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
            axisVal = hMin(axisVal, 1.0f);
            axisVal *= state_.Gamepad.sThumbRY < 0 ? -1.f : 1.f;
            axes_[5].anologueVal_ = axisVal;
        }
        else
        {
            connected_ = hFalse;
        }
    }

}