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

#include "base/hTypes.h"
#include "base/hStringID.h"
#include "SDL.h"

#define HEART_INPUT_ANY_PORT (hErrorCode)

namespace Heart
{
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

    typedef SDL_GameController hGameController;
    hInt hGetControllerCount();
    hGameController* hOpenController(hUint index);
    void hCloseController(hGameController* controller);
}
}

#endif // DEVICEIINPUT_H__