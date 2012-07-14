/********************************************************************
	created:	2010/08/21
	created:	21:8:2010   22:36
	filename: 	DeviceKernel.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

// #include "Common.h"
// #include "DeviceKernel.h"
// #include "Heart.h"
#include "hEventManager.h"

namespace Heart
{
namespace Device
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool Kernel::Create( const hdDeviceConfig& config, EventManager* pEventManager )
	{
		hInstance_ = config.instance_;
		wndWidth_ = config.Width_;
		wndHeight_ = config.Height_;
		pEventManager_ = pEventManager;

		hcAssert( ( strlen( config.classname_ ) + 1 ) < hdDeviceConfig::WNDCLASSNAMELEN );
		strcpy( &wndClassName_[ 0 ], "HeartEngineWindow"/*config.classname_*/ );

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

			systemHandle_.hWnd_ = hWnd_;

			return hTrue;
		}

		return hFalse;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Kernel::Destroy()
	{
		DestroyWindow( hWnd_ );
		UnregisterClass( &wndClassName_[ 0 ], hInstance_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool Kernel::CreateWndClassEx( HINSTANCE hinstance, const hChar* classname )
	{
		// Fill in the window class structure with parameters 
		// that describe the main window. 
		ZeroMemory( &wndClassEx_, sizeof( wndClassEx_ ) );
		wndClassEx_.cbSize = sizeof( wndClassEx_ );				// size of structure 
		wndClassEx_.style = CS_HREDRAW | CS_VREDRAW;            // redraw if size changes 
		wndClassEx_.lpfnWndProc = &Kernel::WindowProc;			// points to window procedure 
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

	LRESULT CALLBACK Kernel::WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		Kernel* wnd = NULL;
		// i wont have to do this if it was defined right!!! :@ [6/21/2008 James]
#pragma warning ( push )
#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4312 )
		if( uMsg == WM_NCCREATE )
		{
			// retrieve Window instance from window creation data and associate
			wnd = reinterpret_cast< Kernel* >( ( ( LPCREATESTRUCT ) lParam )->lpCreateParams );
			SetWindowLongPtr( hwnd, 0, reinterpret_cast< LONG_PTR >( wnd ) );
		}
		else
		{
			// retrieve associated Window instance
			wnd = reinterpret_cast< Kernel* >( GetWindowLongPtr( hwnd, 0 ) );
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

	LRESULT Kernel::WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		switch (uMsg) 
		{ 
		case WM_CLOSE:
			pEventManager_->PostEvent( KERNEL_EVENT_CHANNEL, KernelEvents::QuitRequestedEvent() );
			PostQuitMessage( 0 );
			return 0;
		case WM_ACTIVATE:
			{
				GetCursorPos( &prevMousePos_ );
			}
			return 0;
		case WM_KEYDOWN:
			{
				hBool repeat = (lParam & 0x40000000) == 0x40000000;
				pEventManager_->PostEvent( KERNEL_EVENT_CHANNEL, KernelEvents::KeyboardInputEvent( wParam, hTrue, repeat ) );
			}
			break;
		case WM_KEYUP:
			{
				pEventManager_->PostEvent( KERNEL_EVENT_CHANNEL, KernelEvents::KeyboardInputEvent( wParam, hFalse, hFalse ) ); 
			}
			break;
        case WM_MOUSEWHEEL:
            {
                wheelMove_ = (hInt16)HIWORD( wParam );
            }
            break;
        case WM_LBUTTONDOWN:
            {
                pEventManager_->PostEvent( KERNEL_EVENT_CHANNEL, KernelEvents::KeyboardInputEvent( Device::IID_LEFTMOUSEBUTTON, hTrue, hFalse ) ); 
            }
            break;
        case WM_LBUTTONUP:
            {
                pEventManager_->PostEvent( KERNEL_EVENT_CHANNEL, KernelEvents::KeyboardInputEvent( Device::IID_LEFTMOUSEBUTTON, hFalse, hFalse ) ); 
            }
            break;
        case WM_RBUTTONDOWN:
            {
                pEventManager_->PostEvent( KERNEL_EVENT_CHANNEL, KernelEvents::KeyboardInputEvent( Device::IID_RIGHTMOUSEBUTTON, hTrue, hFalse ) ); 
            }
            break;
        case WM_RBUTTONUP:
            {
                pEventManager_->PostEvent( KERNEL_EVENT_CHANNEL, KernelEvents::KeyboardInputEvent( Device::IID_RIGHTMOUSEBUTTON, hFalse, hFalse ) ); 
            }
            break;
        case WM_MBUTTONDOWN:
            {
                pEventManager_->PostEvent( KERNEL_EVENT_CHANNEL, KernelEvents::KeyboardInputEvent( Device::IID_MIDDLEMOUSEBUTTON, hTrue, hFalse ) ); 
            }
            break;
        case WM_MBUTTONUP:
            {
                pEventManager_->PostEvent( KERNEL_EVENT_CHANNEL, KernelEvents::KeyboardInputEvent( Device::IID_MIDDLEMOUSEBUTTON, hFalse, hFalse ) ); 
            }
            break;
		case WM_CHAR:
			{
				hChar charcode = ( wParam & 0x7F );
				if ( charcode > 0 )
				{
					pEventManager_->PostEvent( KERNEL_EVENT_CHANNEL, KernelEvents::KeyboardCharacterEvent( charcode ) );
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

	void Kernel::Update()
	{
		if ( GetActiveWindow() == hWnd_ )
		{
			RECT r, w;
			POINT mouse;

			GetClientRect( hWnd_, &r );
			GetWindowRect( hWnd_, &w );

			GetCursorPos( &mouse );

			pEventManager_->PostEvent( KERNEL_EVENT_CHANNEL, KernelEvents::MouseMoveEvent(prevMousePos_.x - mouse.x, prevMousePos_.y - mouse.y, wheelMove_) );

			prevMousePos_.x = w.left + (r.right/2);
			prevMousePos_.y = w.top + (r.bottom/2);
            wheelMove_ = 0;

		    SetCursorPos( prevMousePos_.x, prevMousePos_.y );

			ShowCursor( false );
		}
		else
		{
			ShowCursor( true );
		}

		PumpMessages();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Kernel::SetWindowTitle( const hChar* titleStr )
	{
		SetWindowText( hWnd_, titleStr );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Kernel::PumpMessages()
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

    HEART_SLIBEXPORT 
	hBool HEART_API DefaultFullscreenSetting()
	{
		return hFalse;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    HEART_SLIBEXPORT 
	hUint32 HEART_API DefaultScreenWidth()
	{
		return 640;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    HEART_SLIBEXPORT 
	hUint32 HEART_API DefaultScreenHeight()
	{
		return 480;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    HEART_SLIBEXPORT 
	hBool HEART_API DefaultVsyncSetting()
	{
		return hFalse;
	}

}
}