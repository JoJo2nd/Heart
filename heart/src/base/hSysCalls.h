/********************************************************************

    filename: 	hSysCalls.h	
    
    Copyright (c) 20:8:2012 James Moran
    
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
#ifndef HSYSCALLS_H__
#define HSYSCALLS_H__

namespace Heart
{

namespace hSysCall
{
    HEART_DLLEXPORT
    hUint64 HEART_API GetProcessorSpeed();

    HEART_DLLEXPORT
    void HEART_API GetCurrentWorkingDir(hChar* out, hUint bufsize);

    HEART_DLLEXPORT
    void HEART_API hInitSystemDebugLibs();
}

namespace hMemTracking
{
    HEART_DLLEXPORT
    void HEART_API InitMemTracking();

    HEART_DLLEXPORT
    void HEART_API TrackAlloc(const hChar* tag, hSize_t line, void* heap, void* ptr, hSize_t size, const hChar* heaptag);

    HEART_DLLEXPORT
    void HEART_API TrackRealloc(const hChar* tag, hSize_t line, void* heap, void* ptr, hSize_t size, const hChar* heaptag);

    HEART_DLLEXPORT
    void HEART_API TrackFree(void* heap, void* ptr, const hChar* heaptag);

    HEART_DLLEXPORT
    void HEART_API TrackPushMarker(const hChar* heaptag);

    HEART_DLLEXPORT
    void HEART_API TrackPopMarker();
}

}

#endif // HSYSCALLS_H__