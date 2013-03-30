/********************************************************************

    filename: 	DeviceWin32.h	
    
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
#ifndef DEVICEWIN32_H__
#define DEVICEWIN32_H__

//////////////////////////////////////////////////////////////////////////
// Common Includes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "base/hHeartConfig.h"

//////////////////////////////////////////////////////////////////////////
// Platform Includes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// shut the compiler up
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP(SP1) or later.
#define WINVER 0x0502		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP(SP1) or later.                   
#define _WIN32_WINNT 0x0502	// Change this to the appropriate value to target other versions of Windows.
#endif	

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <al.h>         //openAL
#include <alc.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
//#ifdef HEART_DEBUG
    #include <dbghelp.h>
//#endif
#include <new>
#include <string>
#include <deque>

#include <xinput.h>

//////////////////////////////////////////////////////////////////////////
// base includes /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "HeartBase.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Math ///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined (HEART_USE_XNAMATH)
#   include <xnamath.h>
#else
#   include <directxmath.h>
#endif

#include "math/hVector.h"
#include "math/hVec2.h"
#include "math/hVec3.h"
#include "math/hVec4.h"
#include "math/hQuaternion.h"
#include "math/hMatrix.h"

//////////////////////////////////////////////////////////////////////////
// Thread Includes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "device/hMutex.h"
#include "device/hSemaphore.h"
#include "device/hEvent.h"
#include "device/hDeviceThread.h"

//////////////////////////////////////////////////////////////////////////
// Memory Includes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "device/hMalloc.h"

//////////////////////////////////////////////////////////////////////////
// Input Includes ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "device/hDeviceInput.h"
#include "device/hDeviceKeyboard.h"
#include "device/hDeviceMouse.h"
#include "device/hDeviceGamepad.h"
#include "device/hDeviceInputDeviceManager.h"

//////////////////////////////////////////////////////////////////////////
// System Includes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "device/hDeviceConfig.h"
#include "device/hDeviceSystemWindow.h"

//////////////////////////////////////////////////////////////////////////
// Sound /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "device/hDeviceSoundUtil.h"
#include "device/hDeviceSoundVoice.h"
#include "device/hDeviceSoundCore.h"

//////////////////////////////////////////////////////////////////////////
// File System Includes //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "device/hDeviceFileSystem.h"

//////////////////////////////////////////////////////////////////////////
// renderer //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "dx11/hWin32DX11.h"

namespace Heart
{
    typedef hdW32Mutex                  hMutex;
    typedef hdW32ThreadEvent            hdThreadEvent;
    typedef hdThreadEvent               hThreadEvent;
    typedef hdW32Thread                 hThread;
    typedef hdW32SoundCoreDevice        hdSoundCore;
    typedef hdW32SoundVoiceDevice       hdSoundVoice;
    typedef hdW32SoundVoiceInfo         hdSoundVoiceInfo;
}

#endif // DEVICEWIN32_H__