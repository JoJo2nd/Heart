/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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
#if defined (PLATFORM_WINDOWS)
#   include <tchar.h>
#endif
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

#include "base/hMemory.h"
#include "base/hMemoryUtil.h"

#include "base/hSysCalls.h"

#include "base/hCRC32.h"

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
#include "base/hClock.h"
#include "base/hProfiler.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Math ///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "vectormath/SSE/cpp/vectormath_aos.h"

#include "math/hVector.h"
#include "math/hVec2.h"
#include "math/hVec3.h"
#include "math/hVec4.h"
#include "math/hQuaternion.h"
#include "math/hMatrix.h"

#endif // HEARTBASE_H__