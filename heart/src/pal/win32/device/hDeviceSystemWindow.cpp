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


namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool hdSystemWindow::Create( const hdDeviceConfig& deviceconfig )
	{
		hInstance_ = deviceconfig.instance_;
		wndWidth_ = deviceconfig.width_;
		wndHeight_ = deviceconfig.height_;

		//hcAssert( ( strlen( deviceconfig.classname_ ) + 1 ) < hdDeviceConfig::WNDCLASSNAMELEN );
		strcpy( &wndClassName_[ 0 ], "hWindow"/*deviceconfig.classname_*/ );

		if ( CreateWndClassEx( hInstance_, wndClassName_ ) )
		{
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
				(LPVOID) this );     // no window-creation data 

			DWORD err = GetLastError();

			if ( !hWnd_ ) 
				return hFalse; 

			// Show the window and send a WM_PAINT message to the window 
			// procedure. 

			ShowWindow( hWnd_, SW_SHOW ); 
			UpdateWindow( hWnd_ ); 

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

			return hTrue;
		}

		return hFalse;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdSystemWindow::Destroy()
	{
		DestroyWindow( hWnd_ );
		UnregisterClass( &wndClassName_[ 0 ], hInstance_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool hdSystemWindow::CreateWndClassEx( HINSTANCE hinstance, const hChar* classname )
	{
		// Fill in the window class structure with parameters 
		// that describe the main window. 
		ZeroMemory( &wndClassEx_, sizeof( wndClassEx_ ) );
		wndClassEx_.cbSize = sizeof( wndClassEx_ );				// size of structure 
		wndClassEx_.style = CS_HREDRAW | CS_VREDRAW;            // redraw if size changes 
		wndClassEx_.lpfnWndProc = &hdSystemWindow::WindowProc;			// points to window procedure 
		wndClassEx_.cbClsExtra = 0;								// no extra class memory 
		wndClassEx_.cbWndExtra = sizeof( void* );				// no extra window memory 
		wndClassEx_.hInstance = hinstance;						// handle to instance 
		wndClassEx_.hIcon = LoadIcon( NULL, IDI_APPLICATION );  // predefined app. icon 
		wndClassEx_.hCursor = LoadCursor( NULL, IDC_ARROW );    // predefined arrow 
		wndClassEx_.hbrBackground = NULL;						// white background brush 
		wndClassEx_.lpszMenuName =  NULL;						// name of menu resource 
		wndClassEx_.lpszClassName = classname;					// name of window class 
		wndClassEx_.hIconSm = NULL;								// small class icon 

		// Register the window class. 
		return RegisterClassEx( &wndClassEx_ ) != 0 ? hTrue : hFalse;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	LRESULT CALLBACK hdSystemWindow::WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		hdSystemWindow* wnd = NULL;
		// i wont have to do this if it was defined right!!! :@ [6/21/2008 James]
#pragma warning ( push )
#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4312 )
		if( uMsg == WM_NCCREATE )
		{
			// retrieve Window instance from window creation data and associate
			wnd = reinterpret_cast< hdSystemWindow* >( ( ( LPCREATESTRUCT ) lParam )->lpCreateParams );
			SetWindowLongPtr( hwnd, 0, reinterpret_cast< LONG_PTR >( wnd ) );
		}
		else
		{
			// retrieve associated Window instance
			wnd = reinterpret_cast< hdSystemWindow* >( GetWindowLongPtr( hwnd, 0 ) );
		}
#pragma warning ( pop )
		// call the windows message handler
		if ( wnd )
		{
			return wnd->WndProc( hwnd, uMsg, wParam, lParam );
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	LRESULT hdSystemWindow::WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		switch (uMsg) 
		{ 
		case WM_CLOSE:
			//pEventManager_->PostEvent( KERNEL_EVENT_CHANNEL, KernelEvents::QuitRequestedEvent() );
			PostQuitMessage( 0 );
            exit(0);
			return 0;
		case WM_ACTIVATE:
			{

			}
			return 0;
        case WM_MOUSEMOVE:
            {
                int x = (signed short)LOWORD( lParam );
                int y = (signed short)HIWORD( lParam );

                mouse_.SetMousePosition((hFloat)x, (hFloat)y);
            }
		case WM_KEYDOWN:
			{
				keyboard_.SetButton(wParam, hButtonState_IS_DOWN);
			}
			break;
		case WM_KEYUP:
			{
				keyboard_.SetButton(wParam, hButtonState_IS_UP); 
			}
			break;
        case WM_MOUSEWHEEL:
            {
                wheelMove_ = (hInt16)HIWORD( wParam );
            }
            break;
        case WM_LBUTTONDOWN:
            {
                mouse_.SetButton(HEART_MOUSE_BUTTON1, hButtonState_IS_DOWN);
            }
            break;
        case WM_LBUTTONUP:
            {
                mouse_.SetButton(HEART_MOUSE_BUTTON1, hButtonState_IS_UP);
            }
            break;
        case WM_RBUTTONDOWN:
            {
                mouse_.SetButton(HEART_MOUSE_BUTTON2, hButtonState_IS_DOWN);
            }
            break;
        case WM_RBUTTONUP:
            {
                mouse_.SetButton(HEART_MOUSE_BUTTON2, hButtonState_IS_UP);
            }
            break;
        case WM_MBUTTONDOWN:
            {
                mouse_.SetButton(HEART_MOUSE_BUTTON3, hButtonState_IS_DOWN);
            }
            break;
        case WM_MBUTTONUP:
            {
                mouse_.SetButton(HEART_MOUSE_BUTTON3, hButtonState_IS_UP);
            }
            break;
		case WM_CHAR:
			{
                hChar charcode = ( wParam & 0x7F );
                if ( charcode > 0 )
                {
                    keyboard_.PushCharacterEvent(charcode);
                }
			}
			break;
		default:
			break;
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam); 
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
		SetWindowText( hWnd_, titleStr );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdSystemWindow::PumpMessages()
	{
		MSG msg;
		while ( PeekMessage( &msg, hWnd_, 0, 0, PM_REMOVE ) != 0 )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	HEART_DLLEXPORT hBool HEART_API hd_DefaultFullscreenSetting()
	{
		return hFalse;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	HEART_DLLEXPORT hUint32 HEART_API hd_DefaultScreenWidth()
	{
		return 640;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	HEART_DLLEXPORT hUint32 HEART_API hd_DefaultScreenHeight()
	{
		return 480;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	HEART_DLLEXPORT hBool HEART_API hd_DefaultVsyncSetting()
	{
		return hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT hSharedLibAddress HEART_API hd_OpenSharedLib( const hChar* libname )
    {
        return LoadLibrary(libname);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT void* HEART_API hd_GetFunctionAddress( hSharedLibAddress lib, const char* symbolName )
    {
        return (void*)GetProcAddress(lib, symbolName);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT void HEART_API hd_CloseSharedLib( hSharedLibAddress lib )
    {
        FreeLibrary(lib);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT hTime HEART_API hd_GetSharedLibTimestamp( hSharedLibAddress lib )
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