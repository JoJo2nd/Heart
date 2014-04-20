/********************************************************************

	filename: 	HeartBase.h	
	
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
#ifndef HEARTBASE_H__
#define HEARTBASE_H__

//////////////////////////////////////////////////////////////////////////
// config //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "base/hHeartConfig.h"

//////////////////////////////////////////////////////////////////////////
// stdlib ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <new>
#include <string>
#include <vector>
#include <set>
#include <deque>
#include <unordered_map>
#include <math.h>
#include <float.h>

#ifdef HEART_USE_SDL2
//////////////////////////////////////////////////////////////////////////
/// SDL 2 ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#   include "SDL.h"
#   include "SDL_syswm.h"

#endif

#include "gal/gal.h"

//////////////////////////////////////////////////////////////////////////
// crypto ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "cryptoCRC32.h"
#include "cryptoMD5.h"
#include "cryptoMurmurHash.h"

//////////////////////////////////////////////////////////////////////////
// includes //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "base/hTypes.h"

#include "base/hStringUtil.h"
#include "base/hStringID.h"

#include "base/hMemoryHeapBase.h"
#include "base/hMemoryDefines.h"
#include "base/hMemoryUtil.h"

#include "base/hSysCalls.h"

#include "base/hCRC32.h"

#include "base/hReflection.h"
#include "base/hReflectionUtils.h"
#include "base/hSerialiser.h"

#include "base/hArray.h"
#include "base/hFunctor.h"
#include "base/hReferenceCounted.h"
#include "base/hLinkedList.h"
#include "base/hPtrImpl.h"
#include "base/hQueue.h"
#include "base/hMap.h"
#include "base/hStack.h"
#include "base/hUTF8.h"
#include "base/hRendererConstants.h"
#include "base/hIBuiltDataCache.h"
#include "base/hClock.h"
#include "base/hProfiler.h"

#endif // HEARTBASE_H__