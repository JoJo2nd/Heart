/********************************************************************

    filename:   hDeviceInput.cpp  
    
    Copyright (c) 23:2:2014 James Moran
    
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