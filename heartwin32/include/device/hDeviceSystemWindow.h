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

namespace Heart
{
	class HeartConfig;
	class EventManager;

    #define                             HEART_SHAREDLIB_INVALIDADDRESS (NULL)
    typedef HMODULE                     hSharedLibAddress; 

	class SystemHandle
	{
	public:
		HWND		hWnd_;
	};
	HEARTDEV_SLIBEXPORT hBool HEART_API                hd_DefaultFullscreenSetting();
	HEARTDEV_SLIBEXPORT hUint32 HEART_API              hd_DefaultScreenWidth();
	HEARTDEV_SLIBEXPORT hUint32 HEART_API              hd_DefaultScreenHeight();
	HEARTDEV_SLIBEXPORT hBool HEART_API                hd_DefaultVsyncSetting();
    HEARTDEV_SLIBEXPORT hSharedLibAddress HEART_API    hd_OpenSharedLib(const hChar* libname);
    HEARTDEV_SLIBEXPORT void HEART_API                 hd_CloseSharedLib(hSharedLibAddress lib);
    HEARTDEV_SLIBEXPORT void* HEART_API                hd_GetFunctionAddress(hSharedLibAddress lib, const char* symbolName);

	class HEARTDEV_SLIBEXPORT hdSystemWindow
	{
	public:
		hdSystemWindow()
			: hInstance_( NULL )
			, hWnd_( NULL )
		{
			wndTitle_[0] = 0;
		}

		hBool					Create( const hdDeviceConfig& config );
		const SystemHandle*		GetSystemHandle() { return &systemHandle_; };
		void					SetWindowTitle( const hChar* titleStr );
		void					Update();
		void                    PumpMessages();
		void					Destroy();
        hdKeyboard*             GetSystemKeyboard() { return &keyboard_; }
        hdMouse*                GetSystemMouse()    { return &mouse_; }
		
	private:

		hBool                   CreateWndClassEx( HINSTANCE hinstance, const hChar* classname );
		static LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		LRESULT                 WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

		HINSTANCE					hInstance_;
		HWND						hWnd_;
		WNDCLASSEX					wndClassEx_;
		hChar						wndClassName_[ 256 ];
		hChar						wndTitle_[ 256 ];
		hUint32						wndWidth_;
		hUint32						wndHeight_;
		SystemHandle				systemHandle_;
		POINT						prevMousePos_;
        hInt16                      wheelMove_;

        hdKeyboard		            keyboard_;
        hdMouse                     mouse_;

	};
}

#endif // DEVICEKERNEL_H__