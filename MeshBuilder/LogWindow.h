/********************************************************************

	filename: 	LogWindow.h	
	
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

#ifndef LOGWINDOW_H__
#define LOGWINDOW_H__

#include "stdafx.h"

namespace UI
{
namespace Log
{
	void Initilaise( HINSTANCE hInst );
	void ShowLogWindow();
	void HideLogWindow();
	bool IsLogVisiable();
	void PushTabIndent();
	void PopTabIndent();
	void WriteLog( const char* message, ... );
	void WriteInfo( const char* message, ... );
	void WriteWarning( const char* message, ... );
	void WriteError( const char* message, ... );
}
}

#endif // LOGWINDOW_H__