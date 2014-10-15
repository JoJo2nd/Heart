/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "pal/hDeviceInput.h"

namespace Heart
{
namespace DeviceInput
{
#ifdef HEART_USE_SDL2
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

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hInt hGetControllerCount() {
        return SDL_NumJoysticks();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hGameController* hOpenController(hUint index) {
        hGameController* controller = SDL_GameControllerOpen(index);
        if (controller) {
            SDL_GameControllerEventState(SDL_ENABLE);
        }
        return controller;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hCloseController(hGameController* controller) {
        SDL_GameControllerClose(controller);
    }

#endif
}
}