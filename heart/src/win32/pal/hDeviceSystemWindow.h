/********************************************************************

    filename: 	hDeviceKernel.h	
    
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

#ifndef DEVICEKERNEL_H__
#define DEVICEKERNEL_H__

#include "base/hTypes.h"
#include "base/hFunctor.h"
#include "pal/hDeviceConfig.h"
#include "pal/hEvent.h"
#include <winsock2.h>
#include <windows.h>
#ifdef HEART_USE_SDL2
#   include "SDL.h"
#   include "SDL_syswm.h"
#endif

namespace Heart
{
    class HeartConfig;
    class EventManager;

    #define                             HEART_SHAREDLIB_INVALIDADDRESS (NULL)
    typedef HMODULE                     hSharedLibAddress; 

#ifdef HEART_USE_SDL2
    typedef SDL_Event                   hSysEvent; // just in-case it is replaced at a later date
#   define hGetSystemEventID(x) (SDL_##x)
    hFUNCTOR_TYPEDEF(void(*)(hUint, const hSysEvent*), hSysEventHandler);
#endif

    class SystemHandle
    {
    public:
        HWND		hWnd_;
    };
    hBool HEART_API                hd_DefaultFullscreenSetting();
    hUint32 HEART_API              hd_DefaultScreenWidth();
    hUint32 HEART_API              hd_DefaultScreenHeight();
    hBool HEART_API                hd_DefaultVsyncSetting();
    void HEART_API                 hd_AddSharedLibSearchDir(const hChar* abspath);
    hSharedLibAddress HEART_API    hd_OpenSharedLib(const hChar* libname);
    void HEART_API                 hd_CloseSharedLib(hSharedLibAddress lib);
    hTime HEART_API                hd_GetSharedLibTimestamp(hSharedLibAddress lib);
    void* HEART_API                hd_GetFunctionAddress(hSharedLibAddress lib, const char* symbolName);

    class hdSystemWindow
    {
    public:
        hdSystemWindow()
            : impl_(nullptr)
            , hInstance_(nullptr)
            , hWnd_(nullptr)
            , ownWindow_(hFalse)
            , procChain_(nullptr)
            , exitSignal_(hFalse)
        {
            wndTitle_[0] = 0;
        }

        hBool                   Create( const hdDeviceConfig& config );
        const SystemHandle*     GetSystemHandle() { return &systemHandle_; };
        void                    SetWindowTitle( const hChar* titleStr );
        void                    Update();
        void                    PumpMessages();
        void                    Destroy();
#ifndef HEART_USE_SDL2
        hdKeyboard*             GetSystemKeyboard() { return &keyboard_; }
        hdMouse*                GetSystemMouse()    { return &mouse_; }
#endif
        void                    signalExit() { exitSignal_.Signal(); }
        hBool                   exitSignaled() { return exitSignal_.TryWait();}
        hUint32                 getWindowWidth() const { return wndWidth_; }
        hUint32                 getWindowHeight() const { return wndHeight_; }
        hBool                   getOwnWindow() const { return ownWindow_; }
#ifdef HEART_USE_SDL2
        void                    setSysEventHandler(hUint sysEventID, hSysEventHandler handler);
        void                    removeSysEventHandler(hUint sysEventID, hSysEventHandler handler);
#endif
        
    private:

#ifdef HEART_USE_SDL2
        void bindSysEventHandlers();
        void sysEventQuit(hUint syseventid, const hSysEvent* sysevent);
        void sysEventFocus(hUint syseventid, const hSysEvent* sysevent);
        void sysEventSize(hUint syseventid, const hSysEvent* sysevent);
#else
        hBool                   CreateWndClassEx( HINSTANCE hinstance, const hChar* classname );
        static LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
        LRESULT                 WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
#endif

        static hdSystemWindow*      s_instance;

#ifdef HEART_USE_SDL2
        struct hdSystemWindowImpl*   impl_;
#endif
        HINSTANCE					hInstance_;
        HWND						hWnd_;
        WNDCLASSEX					wndClassEx_;
        hChar						wndClassName_[ 256 ];
        hChar						wndTitle_[ 256 ];
        hUint32						wndWidth_;
        hUint32						wndHeight_;
        SystemHandle				systemHandle_;
        hThreadEvent                exitSignal_;
        POINT						prevMousePos_;
        hInt16                      wheelMove_;
        hUint32                     cursorOffsetX_;
        hUint32                     cursorOffsetY_;
        WNDPROC                     procChain_;
        hBool                       hasFocus_;
        hBool                       ownWindow_;
#ifndef HEART_USE_SDL2
        hdKeyboard		            keyboard_;
        hdMouse                     mouse_;
#endif
    };
}

#endif // DEVICEKERNEL_H__