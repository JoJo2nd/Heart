/********************************************************************

	filename: 	LogWindow.cpp	
	
	Copyright (c) 16:1:2011 James Moran
	
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

#include "stdafx.h"
#include "LogWindow.h"
#include "Resource.h"
#include <richedit.h>

namespace UI
{
namespace Log
{
namespace 
{
	class Logger
	{
	public:
		Logger() :
			hwnd_( NULL )
			,isVisible_( false )
		{

		}

		// Window handle
		HWND hwnd_;

		// current text of the window (contains RTF tags)
		std::string rtText_;
		std::string plainText_;
		std::string tabs_;

		// is the log window currently visible?
		bool isVisible_;

		static const char* RTF_LOG_HEADER;
	};

	enum LogColour
	{
		RED,
		ORANGE,
		GREEN,
		BLUE,
		BLACK,
	};

	const char* Logger::RTF_LOG_HEADER = 
	"{\\rtf1" 
		"\\ansi" 
		"\\deff0"
		"{" 
		"\\fonttbl{\\f0 Courier New;}"
		"}"
	"{\\colortbl;" 
		"\\red255\\green0\\blue0;" 	  // red for errors
		"\\red255\\green120\\blue0;"  // orange for warnings
		"\\red0\\green150\\blue0;" 	  // green for infos
		"\\red0\\green0\\blue180;" 	  // blue for debug messages
		"\\red0\\green0\\blue0;" 	  // black for everything else
	"}";

	Logger gLogger_;
}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	INT_PTR CALLBACK LogDialogProc(HWND ,UINT ,WPARAM ,LPARAM );

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Initilaise( HINSTANCE hInst )
	{
		// needed for the RichEdit control in the log view dialog,
		// otherwise CreateDialog silently fails
		LoadLibrary(_T("riched20.dll") );
		gLogger_.hwnd_ = CreateDialog( hInst, MAKEINTRESOURCE(IDD_LOGVIEWER_DIALOG), NULL, LogDialogProc );

		gLogger_.rtText_ = Logger::RTF_LOG_HEADER;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ShowLogWindow()
	{
		ShowWindow( gLogger_.hwnd_, SW_SHOW );
		UpdateWindow( gLogger_.hwnd_ );
		gLogger_.isVisible_ = true;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void HideLogWindow()
	{
		ShowWindow( gLogger_.hwnd_, SW_HIDE );
		UpdateWindow( gLogger_.hwnd_ );
		gLogger_.isVisible_ = false;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool IsLogVisiable()
	{
		return gLogger_.isVisible_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void WriteLine( const char* message, LogColour colour )
	{
		static const char* colours[] = 
		{
			"{\\pard \\cf1 \\b \\fs18 ",
			"{\\pard \\cf1 \\b \\fs18 ",
			"{\\pard \\cf3 \\b \\fs18 ",
			"{\\pard \\cf4 \\b \\fs18 ",
			"{\\pard \\cf5 \\b \\fs18 "
		};

		std::string _message = message;
		std::string _final = colours[colour];

		for (unsigned int i = 0; i < _message.length();++i)
		{
			if ('\\' == _message[i] ||
				'}'  == _message[i] ||
				'{'  == _message[i])
			{
				_message.insert(i++,"\\");
			}
		}

		_final += gLogger_.tabs_;
		_final += _message;
		_final += "\\par}";

		gLogger_.rtText_.append( _final );

		std::string _msg = gLogger_.rtText_;
		_msg += "}";

		SETTEXTEX sInfo;
		sInfo.flags = ST_DEFAULT;
		sInfo.codepage = CP_ACP;

		SendDlgItemMessage( gLogger_.hwnd_, IDC_LOGWINDOW, EM_SETTEXTEX, (WPARAM)&sInfo, (LPARAM)_msg.c_str() );

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	INT_PTR CALLBACK LogDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		lParam;
		switch (uMsg)
		{
		case WM_INITDIALOG:
			{
				return TRUE;
			}
			break;
		case WM_SIZE:
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);

				SetWindowPos(GetDlgItem(hwndDlg,IDC_LOGWINDOW),NULL,0,0,x-20,y-24,SWP_NOMOVE|SWP_NOZORDER);

				return TRUE;
			}
			break;
		case WM_CLOSE:
			EndDialog(hwndDlg,0);
			return TRUE;
		}

		return FALSE;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void WriteInfo( const char* message, ... )
	{
		va_list marker;
		va_start( marker, message );

		static char buffer[ 2048 ] = "[INFO]";
		buffer[ 2046 ] = 0;
		u32 len = vsprintf_s( buffer+6, 2038, message, marker );
		
		WriteLine( buffer, GREEN );

		va_end( marker );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void WriteWarning( const char* message, ... )
	{
		va_list marker;
		va_start( marker, message );

		static char buffer[ 2048 ] = "[WARN]";
		buffer[ 2046 ] = 0;
		u32 len = vsprintf_s( buffer+6, 2038, message, marker );

		WriteLine( buffer, ORANGE );

		va_end( marker );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void WriteError( const char* message, ... )
	{
		va_list marker;
		va_start( marker, message );

		static char buffer[ 2048 ] = "[ERROR]";
		buffer[ 2046 ] = 0;
		u32 len = vsprintf_s( buffer+7, 2038, message, marker );

		WriteLine( buffer, RED );

		va_end( marker );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void WriteLog( const char* message, ... )
	{
		va_list marker;
		va_start( marker, message );

		static char buffer[ 2048 ] = "[LOG]";
		buffer[ 2046 ] = 0;
		u32 len = vsprintf_s( buffer+5, 2038, message, marker );

		WriteLine( buffer, BLACK );

		va_end( marker );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void PushTabIndent()
	{
		gLogger_.tabs_ += "\t";
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void PopTabIndent()
	{
		if ( gLogger_.tabs_.size() > 0 )
		{
			gLogger_.tabs_.resize( gLogger_.tabs_.size()-1 );
		}
	}

}
}