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
    HEART_EXPORT hUint32 HEART_API getProcessorCount();
#if 0
    void HEART_API hInitSystemDebugLibs();
#endif

#define HEART_SHAREDLIB_INVALIDADDRESS (nullptr)
    typedef void* hSharedLibAddress;

    HEART_EXPORT
    hSharedLibAddress HEART_API openSharedLib(const hChar* libname);
    HEART_EXPORT
    void HEART_API closeSharedLib(hSharedLibAddress lib);
    HEART_EXPORT
    void* HEART_API getFunctionAddress(hSharedLibAddress lib, const char* symbolName);
    template < typename t_fn >
    hFORCEINLINE t_fn* getFunctionAddress(hSharedLibAddress lib, const char* symbolName, t_fn*& fn_ptr) {
        fn_ptr = static_cast<t_fn*>(getFunctionAddress(lib, symbolName));
        return fn_ptr;
    }
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