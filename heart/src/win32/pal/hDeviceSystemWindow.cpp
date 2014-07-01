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

#include "pal/hDeviceSystemWindow.h"
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

    hBool hdSystemWindow::Create( const hdDeviceConfig* deviceconfig )
    {
#ifdef HEART_USE_SDL2
        impl_ = new hdSystemWindowImpl;
        auto& sdlWindow_ = impl_->sdlWindow_;
        sdlWindow_ = SDL_CreateWindow(
            "HeartEngine", 
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            deviceconfig.width_,
            deviceconfig.height_,
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

        hInstance_ = deviceconfig.instance_;
        hWnd_ = info.info.win.window;

        SDL_GetWindowSize(sdlWindow_, (int*)&wndWidth_, (int*)&wndHeight_);

        systemHandle_.hWnd_ = hWnd_;

        bindSysEventHandlers();

        return hTrue;
#else
        deviceconfig->getDataMember("hInstance", &instance_);
        deviceconfig->getDataMember("hWnd", &hWnd_);
        deviceconfig->getDataMember("WindowWidth", &wndWidth_);
        deviceconfig->getDataMember("WindowHeight", &wndHeight_);
        ownWindow_ = hFalse;

        strcpy( &wndClassName_[ 0 ], "hWindow" );

        s_instance = this;

        if (!hWnd_ && CreateWndClassEx( hInstance_, wndClassName_ ) ) {
            hWnd_ = CreateWindow( 
                wndClassName_,		 // name of window class 
                wndTitle_,			 // title-bar string 
                WS_OVERLAPPEDWINDOW, // top-level window 
                CW_USEDEFAULT,       // default horizontal position 
                CW_USEDEFAULT,       // default vertical position 
                wndWidth_,			 // default width 
                wndHeight_,	         // default height 
                (HWND) NULL,         // no owner window 
                (HMENU) NULL,        // use class menu 
                hInstance_,          // handle to application instance 
                NULL );              // no window-creation data 

            DWORD err = GetLastError();

            if ( !hWnd_ ) 
                return hFalse; 

            // Show the window and send a WM_PAINT message to the window 
            // procedure. 

            ownWindow_ = hTrue;

            ShowWindow( hWnd_, SW_SHOW ); 
            UpdateWindow( hWnd_ ); 
        }
        else {
            RECT C;
            GetClientRect(hWnd_, &C);
            wndWidth_ = C.right;
            wndHeight_ = C.bottom;

            //procChain_ = (WNDPROC)SetWindowLong(hWnd_, GWL_WNDPROC, (LONG)&hdSystemWindow::WindowProc);
            procChain_= (WNDPROC)SetWindowLongPtr(hWnd_, GWLP_WNDPROC, (LONG_PTR)&hdSystemWindow::WindowProc);
        }

        RECT w,C;
        GetClientRect(hWnd_, &C);
        GetWindowRect( hWnd_, &w );
        cursorOffsetX_ = wndWidth_ - C.right;
        cursorOffsetY_ = wndHeight_ - C.bottom;
        if (cursorOffsetX_ != 0 || cursorOffsetY_ != 0)
        {
            SetWindowPos(hWnd_,NULL, 0, 0, wndWidth_+cursorOffsetX_, wndHeight_+cursorOffsetY_, SWP_NOMOVE|SWP_NOZORDER);

            GetClientRect(hWnd_, &C);
            GetWindowRect( hWnd_, &w );
            cursorOffsetX_ = wndWidth_ - C.right;
            cursorOffsetY_ = wndHeight_ - C.bottom;
        }

        systemHandle_.hWnd_ = hWnd_;
        return hFalse;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::Destroy()
    {
#ifdef HEART_USE_SDL2
        SDL_DestroyWindow(impl_->sdlWindow_);
        delete impl_;
        impl_ = nullptr;
#else
        if (ownWindow_) {
            DestroyWindow( hWnd_ );
            UnregisterClass( &wndClassName_[ 0 ], hInstance_ );
        }
#endif
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::Update()
    {
        PumpMessages();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::SetWindowTitle( const hChar* titleStr )
    {
#ifdef HEART_USE_SDL2
        SDL_SetWindowTitle(impl_->sdlWindow_, titleStr);
#else
        SetWindowText( hWnd_, titleStr );
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::PumpMessages() {
#ifdef HEART_USE_SDL2
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
#else
        MSG msg;
        while ( PeekMessage( &msg, hWnd_, 0, 0, PM_REMOVE ) != 0 )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
#endif
    }

#ifdef HEART_USE_SDL2
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
        hasFocus_ = (syseventid == hGetSystemEventID(WINDOWEVENT_FOCUS_GAINED));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdSystemWindow::sysEventSize(hUint syseventid, const hSysEvent* sysevent) {
        wndWidth_ = sysevent->window.data1;
        wndHeight_ = sysevent->window.data2;
    }

#endif

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

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

     void HEART_API hd_AddSharedLibSearchDir(const hChar* abspath)
    {
        //push directory on to $path
        hUint len = hStrLen(hgetenv("path"))+hStrLen(abspath)+20;
        hChar* buf = (hChar*)hAlloca(len);
        hStrPrintf(buf, len, "path=%s;%s", hgetenv("path"), abspath);
        hputenv(buf);
        hcPrintf("PATH = %s", hgetenv("path"));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

     hSharedLibAddress HEART_API hd_OpenSharedLib( const hChar* libname )
    {
        return LoadLibrary(libname);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

     void* HEART_API hd_GetFunctionAddress( hSharedLibAddress lib, const char* symbolName )
    {
        return (void*)GetProcAddress(lib, symbolName);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

     void HEART_API hd_CloseSharedLib( hSharedLibAddress lib )
    {
        FreeLibrary(lib);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

     hTime HEART_API hd_GetSharedLibTimestamp( hSharedLibAddress lib )
    {
        hChar filename[MAX_PATH+1];
        hUint32 count;
        hTime modTime = 0;
        HANDLE filehandle;
        BY_HANDLE_FILE_INFORMATION fileInfo;
        
        count = GetModuleFileName(lib, filename, MAX_PATH );

        if (count < 0)
            return 0;

        filename[count] = 0;
        filehandle = CreateFile( filename, GENERIC_READ, TRUE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        if (filehandle == INVALID_HANDLE_VALUE)
            return 0;

        GetFileInformationByHandle(filehandle, &fileInfo);

        modTime = (((hUint64)fileInfo.ftLastWriteTime.dwHighDateTime) << 32)  | fileInfo.ftLastWriteTime.dwLowDateTime;

        CloseHandle(filehandle);

        return modTime;
    }

}