/********************************************************************

	filename: 	hDeviceMouse.h	
	
	Copyright (c) 11:8:2012 James Moran
	
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

#ifndef DEVICEMOUSE_H__
#define DEVICEMOUSE_H__

//#include "DeviceIInput.h"

namespace Heart
{
	class HEARTDEV_SLIBEXPORT hdMouse
	{
	public:

        hFloat              GetAbsoluteX() const { return absX_; }
        hFloat              GetAbsoluteY() const { return absY_; }
        hFloat              GetDeltaX() const { return dx_; }
        hFloat              GetDeltaY() const { return dy_; }
        void                SetMousePosition(hFloat x, hFloat y);
        hdInputButton       GetButton( hdInputID buttonId ) const;
        void                SetButton(hdInputID buttonID, hdInputButtonState state);
        void                EndOfFrameUpdate();

	private:

        static const hUint32 buttonCount = HEART_MOUSE_BUTTON5-HEART_MOUSE_BUTTON1;

        hFloat  absX_;
        hFloat  absY_;
        hFloat  dx_;
        hFloat  dy_;

        hdInputButton   mb_[buttonCount];
	};
}

#endif // DEVICEMOUSE_H__