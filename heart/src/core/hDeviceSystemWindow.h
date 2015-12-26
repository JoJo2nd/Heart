/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "base/hFunctor.h"
#include "core/hDeviceConfig.h"
#include "base/hMutex.h"
#include "base/hConditionVariable.h"
#include "SDL.h"
#include "SDL_syswm.h"
#include <unordered_map>

namespace Heart {
    class HeartConfig;
    class EventManager;

    typedef SDL_Event                   hSysEvent; // just in-case it is replaced at a later date
#define hGetSystemEventID(x) (SDL_##x)
    hFUNCTOR_TYPEDEF(void(*)(hUint, const hSysEvent*), hSysEventHandler);

    class ISystemHandle
    {
    public:
        virtual hUintptr_t  getSystemHandle() = 0;
    };


    class hdSystemWindow
    {
    public:
        hdSystemWindow()
            : sdlWindow_(nullptr)
            , systemHandle_(nullptr)
        {
            hAtomic::AtomicSet(exitSignal_, 0);
        }

        hBool                   Create();
        const ISystemHandle*    GetSystemHandle() { return systemHandle_; };
        void                    SetWindowTitle( const hChar* titleStr );
        void                    Update();
        void                    PumpMessages();
        void                    Destroy();
        void                    signalExit() { hAtomic::AtomicSet(exitSignal_, 1); }
        hBool                   exitSignaled() { return hAtomic::AtomicGet(exitSignal_) != 0; }
        static hUint32          getWindowWidth() { return wndWidth_; }
        static hUint32          getWindowHeight() { return wndHeight_; }
        hBool                   getOwnWindow() const { return hTrue; }
        void                    setSysEventHandler(hUint sysEventID, hSysEventHandler handler);
        void                    removeSysEventHandler(hUint sysEventID, hSysEventHandler handler);
        SDL_Window*             getSDLWindow() const { return sdlWindow_; }
        
    private:

        typedef std::unordered_map<hUint, hSysEventHandler> hSysEventHandlerMap;

        void bindSysEventHandlers();
        void sysEventQuit(hUint syseventid, const hSysEvent* sysevent);
        void sysEventFocus(hUint syseventid, const hSysEvent* sysevent);
        void sysEventSize(hUint syseventid, const hSysEvent* sysevent);

        SDL_Window*                 sdlWindow_;
        hSysEventHandlerMap         sysEventHandlers_;
        ISystemHandle*              systemHandle_;
        hMutex                      exitMutex_;
        hAtomicInt                  exitSignal_;
        hBool                       hasFocus_;
        static hUint32              wndWidth_;
        static hUint32              wndHeight_;
    };
}
