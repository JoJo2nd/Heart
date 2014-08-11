/********************************************************************

    filename: 	hdevicekernel.cpp	
    
    Copyright (c) 7:7:2012 James Moran
    
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

#include "core/hSystem.h"
#include "base/hStringUtil.h"
#include <unordered_map>

namespace Heart {

typedef std::unordered_map<hUint, hSysEventHandler> hSysEventHandlerMap;

struct hdSystemWindowImpl
{
    hdSystemWindowImpl()
        : sdlWindow_(nullptr)
    {
    }

    SDL_Window*                 sdlWindow_;
    hSysEventHandlerMap         sysEventHandlers_;
};

    hdSystemWindow* hdSystemWindow::s_instance = NULL;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdSystemWindow::Create( const hdDeviceConfig* deviceconfig ) {
        impl_ = new hdSystemWindowImpl;
        auto& sdlWindow_ = impl_->sdlWindow_;
        sdlWindow_ = SDL_CreateWindow(
            "HeartEngine", 
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            deviceconfig->getWidth(),
            deviceconfig->getHeight(),
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
        if (!sdlWindow_) {
            hcPrintf("SDL_CreateWindow() Error: %s", SDL_GetError());
            return hFalse;
        }

        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        if (!SDL_GetWindowWMInfo(sdlWindow_, &info)) {
            hcPrintf("SDL_GetWindowWMInfo() Error: %s", SDL_GetError());
            return hFalse;
        }

        SDL_GetWindowSize(sdlWindow_, (int*)&wndWidth_, (int*)&wndHeight_);

        bindSysEventHandlers();

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::Destroy() {
        SDL_DestroyWindow(impl_->sdlWindow_);
        delete impl_;
        impl_ = nullptr;
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::Update() {
        PumpMessages();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::SetWindowTitle( const hChar* titleStr ) {
        SDL_SetWindowTitle(impl_->sdlWindow_, titleStr);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::PumpMessages() {
        auto& sysEventHandlers_ = impl_->sysEventHandlers_;
        SDL_Event sdl_event;
        auto invalidhandler = sysEventHandlers_.end();
        while(SDL_PollEvent(&sdl_event)) {
            auto handler = sysEventHandlers_.find(sdl_event.type);
            if (handler != invalidhandler) {
                handler->second(sdl_event.type, &sdl_event);
            } else if (sdl_event.type == hGetSystemEventID(WINDOWEVENT)) {
                handler = sysEventHandlers_.find(sdl_event.window.type);
                if (handler != invalidhandler) {
                    handler->second(sdl_event.window.type, &sdl_event);
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::setSysEventHandler(hUint sysEventID, hSysEventHandler handler) {
        auto& sysEventHandlers_ = impl_->sysEventHandlers_;
        hcAssert(sysEventHandlers_.find(sysEventID) == sysEventHandlers_.end());
        sysEventHandlers_.insert(hSysEventHandlerMap::value_type(sysEventID, handler));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::removeSysEventHandler(hUint sysEventID, hSysEventHandler handler) {
        auto& sysEventHandlers_ = impl_->sysEventHandlers_;
        hcAssert((sysEventHandlers_.find(sysEventID) != sysEventHandlers_.end())
                && (sysEventHandlers_.find(sysEventID)->second == handler));
        sysEventHandlers_.erase(sysEventID);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::bindSysEventHandlers() {
        setSysEventHandler(hGetSystemEventID(QUIT), hFUNCTOR_BINDMEMBER(hSysEventHandler, hdSystemWindow, sysEventQuit, this));
        setSysEventHandler(hGetSystemEventID(WINDOWEVENT_FOCUS_GAINED), hFUNCTOR_BINDMEMBER(hSysEventHandler, hdSystemWindow, sysEventFocus, this));
        setSysEventHandler(hGetSystemEventID(WINDOWEVENT_RESIZED), hFUNCTOR_BINDMEMBER(hSysEventHandler, hdSystemWindow, sysEventSize, this));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::sysEventQuit(hUint syseventid, const hSysEvent* sysevent) {
        signalExit();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::sysEventFocus(hUint syseventid, const hSysEvent* sysevent) {
        hBool hasFocus_ = (syseventid == hGetSystemEventID(WINDOWEVENT_FOCUS_GAINED));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::sysEventSize(hUint syseventid, const hSysEvent* sysevent) {
        wndWidth_ = sysevent->window.data1;
        wndHeight_ = sysevent->window.data2;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

     hBool HEART_API hd_DefaultFullscreenSetting()
    {
        return hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

     hUint32 HEART_API hd_DefaultScreenWidth()
    {
        return 640;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

     hUint32 HEART_API hd_DefaultScreenHeight()
    {
        return 480;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

     hBool HEART_API hd_DefaultVsyncSetting()
    {
        return hFalse;
    }
}
