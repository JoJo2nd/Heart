/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
    
#include "core/hSystem.h"
#include "base/hStringUtil.h"
#include <unordered_map>

namespace Heart {

    hUint32 hdSystemWindow::wndWidth_ = 0;
    hUint32 hdSystemWindow::wndHeight_ = 0;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdSystemWindow::Create() {
#if HEART_DEBUG
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
        sdlWindow_ = SDL_CreateWindow(
            "HeartEngine", 
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            hConfigurationVariables::getCVarInt("window.sizex", 640),
            hConfigurationVariables::getCVarInt("window.sizey", 320),
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
        SDL_DestroyWindow(sdlWindow_);
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
        SDL_SetWindowTitle(sdlWindow_, titleStr);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::PumpMessages() {
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
        hcAssert(sysEventHandlers_.find(sysEventID) == sysEventHandlers_.end());
        sysEventHandlers_.insert(hSysEventHandlerMap::value_type(sysEventID, handler));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::removeSysEventHandler(hUint sysEventID, hSysEventHandler handler) {
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
		//exit(0);
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
}
