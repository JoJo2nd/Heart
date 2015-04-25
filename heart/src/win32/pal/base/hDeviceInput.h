/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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