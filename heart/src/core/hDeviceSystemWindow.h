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

#pragma once

#include "base/hTypes.h"
#include "base/hFunctor.h"
#include "core/hDeviceConfig.h"
#include "pal/hMutex.h"
#include "pal/hConditionVariable.h"
#include "SDL.h"
#include "SDL_syswm.h"

namespace Heart {
    class HeartConfig;
    class EventManager;

    #define                             HEART_SHAREDLIB_INVALIDADDRESS (nullptr)
    typedef hUintptr_t                  hSharedLibAddress; 

    typedef SDL_Event                   hSysEvent; // just in-case it is replaced at a later date
#define hGetSystemEventID(x) (SDL_##x)
    hFUNCTOR_TYPEDEF(void(*)(hUint, const hSysEvent*), hSysEventHandler);

    class ISystemHandle
    {
    public:
        virtual hUintptr_t  getSystemHandle() = 0;
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
        {
        }

        hBool                   Create( const hdDeviceConfig* config );
        const ISystemHandle*    GetSystemHandle() { return systemHandle_; };
        void                    SetWindowTitle( const hChar* titleStr );
        void                    Update();
        void                    PumpMessages();
        void                    Destroy();
        void                    signalExit() { exitSignal_.broadcast(); }
        hBool                   exitSignaled() { return exitSignal_.tryWait(&exitMutex_); }
        hUint32                 getWindowWidth() const { return wndWidth_; }
        hUint32                 getWindowHeight() const { return wndHeight_; }
        hBool                   getOwnWindow() const { return hTrue; }
        void                    setSysEventHandler(hUint sysEventID, hSysEventHandler handler);
        void                    removeSysEventHandler(hUint sysEventID, hSysEventHandler handler);
        
    private:

        void bindSysEventHandlers();
        void sysEventQuit(hUint syseventid, const hSysEvent* sysevent);
        void sysEventFocus(hUint syseventid, const hSysEvent* sysevent);
        void sysEventSize(hUint syseventid, const hSysEvent* sysevent);

        static hdSystemWindow*      s_instance;

        struct hdSystemWindowImpl*  impl_;
        ISystemHandle*              systemHandle_;
        hUint32                     wndWidth_;
        hUint32                     wndHeight_;
        hMutex                      exitMutex_;
        hConditionVariable          exitSignal_;
    };
}