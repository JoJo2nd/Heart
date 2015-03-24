/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef HSYSCALLS_H__
#define HSYSCALLS_H__

namespace Heart
{

namespace hSysCall {
    HEART_EXPORT hUint64 HEART_API GetProcessorSpeed();
#if 0
    void HEART_API hInitSystemDebugLibs();
#endif
}

namespace hMemTracking {
#if 0
    void HEART_API InitMemTracking();
    void HEART_API TrackAlloc(const hChar* tag, hSize_t line, void* heap, void* ptr, hSize_t size, const hChar* heaptag);
    void HEART_API TrackFree(void* heap, void* ptr, const hChar* heaptag);
    void HEART_API TrackPushMarker(const hChar* heaptag);
    void HEART_API TrackPopMarker();
#endif
}

}

#endif // HSYSCALLS_H__