/********************************************************************

	filename: 	DeviceWin32Unity.cpp	
	
	Copyright (c) 22:7:2011 James Moran
	
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

#include "device/hWin32.h"

//////////////////////////////////////////////////////////////////////////
// Memory ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
extern "C"
{
    #include "device/hMalloc.c"
}

//////////////////////////////////////////////////////////////////////////
// Kernel ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "device/hDeviceSystemWindow.cpp"

//////////////////////////////////////////////////////////////////////////
// Sound /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "device/hDeviceSoundCore.cpp"
#include "device/hDeviceSoundVoice.cpp"
#include "device/hDeviceSoundUtil.cpp"

//////////////////////////////////////////////////////////////////////////
// Threading /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// File System Includes //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "device/hDeviceFileSystem.cpp"

//////////////////////////////////////////////////////////////////////////
// Input Includes ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "device/hDeviceKeyboard.cpp"
#include "device/hDeviceMouse.cpp"
#include "device/hDeviceGamepad.cpp"
#include "device/hDeviceInputDeviceManager.cpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "device/dx11/hDX11RenderDevice.cpp"
#include "device/dx11/hDX11RenderSubmissionCtx.cpp"
#include "device/dx11/hDX11ShaderProgram.cpp"