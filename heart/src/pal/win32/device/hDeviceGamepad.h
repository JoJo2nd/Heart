/********************************************************************

	filename: 	hDeviceGamepad.h	
	
	Copyright (c) 19:7:2012 James Moran
	
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

#ifndef HDEVICEGAMEPAD_H__
#define HDEVICEGAMEPAD_H__


namespace Heart
{
    class HEART_DLLEXPORT hdGamepad
    {
    public:
#define HEART_DEVICE_AXIS_COUNT     (6)
#define HEART_DEVICE_BUTTON_COUNT   (14)

        hdGamepad()
            : padIdx_(hErrorCode)
            , connected_(hFalse)
        {

        }
        ~hdGamepad()
        {

        }

        hBool           IsConnected() const { return connected_; }
        hdInputButton   GetButton(hUint32 buttonID) const;
        hdInputAxis     GetAxis(hUint32 axisID) const;

    private:

        friend class hdInputDeviceManager;

        void            ConnectPad(hUint32 idx);
        void            Update();
        void            EndOfFrameUpdate() {}

        hUint32             padIdx_;
        hBool               connected_;
        XINPUT_STATE        state_;
        hdInputButton       buttons_[HEART_DEVICE_BUTTON_COUNT];
        hdInputAxis         axes_[HEART_DEVICE_AXIS_COUNT];
    };
}
#endif // HDEVICEGAMEPAD_H__