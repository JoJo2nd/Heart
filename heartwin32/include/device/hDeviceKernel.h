/********************************************************************
	created:	2010/08/21
	created:	21:8:2010   22:26
	filename: 	DeviceKernel.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef DEVICEKERNEL_H__
#define DEVICEKERNEL_H__

// #include "hTypes.h"
// #include "DeviceIKernel.h"
// #include <windows.h>

namespace Heart
{
	class HeartConfig;
	class EventManager;

	static const hUint32 KERNEL_EVENT_CHANNEL = 0;

    #define                             HEART_SHAREDLIB_INVALIDADDRESS (NULL)
    typedef HMODULE                     hSharedLibAddress; 

/*
namespace KernelEvents
{
	// Move these to seperate header
	enum KernelEventID
	{
		KM_GAME_UPDATE_PAUSE,
		KM_GAME_UPDATE_RESUME,
		KM_WINDOW_RESIZE,
		KM_DEVICE_LOST,
		KM_DEVICE_RESET,
		KM_KEYBOARD_INPUT,
		KM_KEYBOARD_CHARACTER,
		KM_QUIT_REQUESTED,
		KM_MOUSE_MOVE,

		KM_MAX
	};

	typedef Event< KM_GAME_UPDATE_PAUSE, hNullType >				GameUpdatePauseEvent;
	typedef Event< KM_GAME_UPDATE_RESUME, hNullType >				GameUpdateResumeEvent;

	struct WindowResizePayload
	{
		WindowResizePayload( hUint32 x, hUint32 y, hUint32 width, hUint32 height ) 
			: x_( x )
			,y_( y )
			,width_( width )
			,height_( height )
		{}
		hUint32 x_;
		hUint32 y_;
		hUint32 width_;
		hUint32 height_;
	};

	typedef Event< KM_WINDOW_RESIZE, WindowResizePayload >			WindowResizeEvent;
	typedef Event< KM_DEVICE_LOST, hNullType >						DeviceLostEvent;
	typedef Event< KM_DEVICE_RESET, hNullType >					DeviceResetEvent;

	struct KeyboardInputPayload
	{
		KeyboardInputPayload( hUint32 keyid, hBool down, hBool isRepeat ) 
			: key_( keyid )
			, down_( down )
			, isRepeat_( isRepeat )
		{}
		hUint32			key_;
		hBool			down_;
		hBool			isRepeat_;
	};
	typedef Event< KM_KEYBOARD_INPUT, KeyboardInputPayload >		KeyboardInputEvent;
	typedef Event< KM_QUIT_REQUESTED, hNullType >					QuitRequestedEvent;
	struct KeyboardCharPayload
	{
		KeyboardCharPayload( hChar character ) :
	char_( character )
	{

	}

	hChar char_;
	};
	typedef Event< KM_KEYBOARD_CHARACTER, KeyboardCharPayload >			KeyboardCharacterEvent;
	struct MouseMovePayload
	{
		MouseMovePayload(hInt32 x, hInt32 y, hInt32 wheeldir) 
			: xDelta_( x )
			, yDelta_( y )
            , wheel_(wheeldir)
		{

		}

		hInt32 xDelta_;
		hInt32 yDelta_;
        hInt32 wheel_;
	};
	typedef Event< KM_MOUSE_MOVE, MouseMovePayload >					MouseMoveEvent;
}
*/
namespace Device
{
	class SystemHandle
	{
	public:
		HWND		hWnd_;
	};

	HEARTDEV_SLIBEXPORT hBool HEART_API                DefaultFullscreenSetting();
	HEARTDEV_SLIBEXPORT hUint32 HEART_API              DefaultScreenWidth();
	HEARTDEV_SLIBEXPORT hUint32 HEART_API              DefaultScreenHeight();
	HEARTDEV_SLIBEXPORT hBool HEART_API                DefaultVsyncSetting();
}
    HEARTDEV_SLIBEXPORT hSharedLibAddress HEART_API    OpenSharedLib(const hChar* libname);
    HEARTDEV_SLIBEXPORT void HEART_API                 CloseSharedLib(hSharedLibAddress lib);
    HEARTDEV_SLIBEXPORT void* HEART_API                GetFunctionAddress(hSharedLibAddress lib, const char* symbolName);

namespace Device
{
	class HEARTDEV_SLIBEXPORT Kernel //: public IKernel
	{
	public:
		Kernel()
			: hInstance_( NULL )
			, hWnd_( NULL )
		{
			wndTitle_[0] = 0;
		}

		hBool					Create( const hdDeviceConfig& config, EventManager* pEventsManager );
		const SystemHandle*		GetSystemHandle() { return &systemHandle_; };
		void					SetWindowTitle( const hChar* titleStr );
		void					Update();
		void                    PumpMessages();
		void					Destroy();
		
	private:

		hBool CreateWndClassEx( HINSTANCE hinstance, const hChar* classname );
		static LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		LRESULT WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

		HINSTANCE					hInstance_;
		HWND						hWnd_;
		WNDCLASSEX					wndClassEx_;
		hChar						wndClassName_[ 256 ];
		hChar						wndTitle_[ 256 ];
		hUint32						wndWidth_;
		hUint32						wndHeight_;
		SystemHandle				systemHandle_;
		EventManager*				pEventManager_;
		POINT						prevMousePos_;
        hInt16                      wheelMove_;
	};
}
}

#endif // DEVICEKERNEL_H__