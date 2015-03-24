/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#define HEART_MEMTRACK_FULL_STACKTRACK
#ifdef HEART_64BIT
#   define size_t_fmt "%llu"
#else
#   define size_t_fmt "%lu"
#endif

#include "base/hHeartConfig.h"
#include "base/hTypes.h"
#include <windows.h>

namespace Heart {
namespace hSysCall {

    HEART_EXPORT hUint64 HEART_API GetProcessorSpeed()
    {
        //char Buffer[_MAX_PATH];
        DWORD BufSize = _MAX_PATH;
        DWORD dwMHz = _MAX_PATH;
        HKEY hKey;

        // open the key where the proc speed is hidden:
        long lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
            0,
            KEY_READ,
            &hKey);

        if(lError != ERROR_SUCCESS)
        {// if the key is not found, tell the user why:
            return 1;
        }

        // query the key:
        RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE) &dwMHz, NULL/*&BufSize*/);

        return dwMHz*1000000;
    }

}

namespace hMemTracking
{
#if 0
    struct hMemTrackingHeader
    {
        hUint64   baseAddress_;
        char      moduleName_[512];
    };

namespace
{
    static const hUint32            g_maxSymbols = 4096;
    static hUint32                  g_symbolsCount = 0;
    static hBool                    g_open   = hFalse;
    static _RTL_CRITICAL_SECTION    g_access;
    static FILE*                    g_file = NULL;
    static hUintptr_t               g_symbols[g_maxSymbols];
}
#endif
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
namespace hSysCall
{
#if 0
namespace
{
    static hBool                    g_doneInit=hFalse;
}
    void hInitSystemDebugLibs() {
        if (!g_doneInit) {
            hcAssert(g_doneInit==hFalse);
#ifdef HEART_PLAT_WINDOWS
            SymInitialize(GetCurrentProcess(), NULL, TRUE);
            SymSetOptions(SYMOPT_LOAD_LINES|SYMOPT_INCLUDE_32BIT_MODULES|SYMOPT_LOAD_ANYTHING);
#endif
            g_doneInit=hFalse;
        }
    }
#endif
}

namespace hMemTracking
{
#if 0

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

    static void memTrackExitHandle()
    {

    }

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

    static hFORCEINLINE void StackTraceToFile(FILE* f) {
        static const hUint32 callstackLimit = 256;
        unsigned int   i;
        void         * stack[ 256 ];
        unsigned short frames;
        
        frames = CaptureStackBackTrace( 2, 256, stack, NULL );
        hcAssert(frames > 0 && frames < 256);
        for( i = 0; i < frames && i < 256; i++ )
        {
            fprintf(g_file, "bt(%u,%p)\n", frames - i - 1, stack[i]);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void HEART_API InitMemTracking()
    {
#ifdef HEART_TRACK_MEMORY_ALLOCS
        InitializeCriticalSection(&g_access);
        hSysCall::hInitSystemDebugLibs();
        if (g_file == NULL)
        {
            if (g_file = fopen("mem_track.txt", "w"))
            {
                g_open = hTrue;
                hMemTrackingHeader header;
                header.baseAddress_=SymGetModuleBase64(GetCurrentProcess(), (DWORD64)&InitMemTracking);
                GetModuleFileName(0, header.moduleName_, (hUint)hArraySize(header.moduleName_));
                fwrite(&header, sizeof(hMemTrackingHeader), 1, g_file);
                fwrite("\n", 1, 1, g_file);
                atexit(memTrackExitHandle);
                //TrackPushMarker("ROOT");
            }
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void HEART_API TrackAlloc( const hChar* tag, hSize_t line, void* heap, void* ptr, hSize_t size, const hChar* heaptag )
    {
#ifdef HEART_TRACK_MEMORY_ALLOCS
        if (g_open)
        {
            EnterCriticalSection(&g_access);
            fprintf(g_file, "!! ALLOC\n"
                "address(%p)\nheap(%s,%p)\nsize("size_t_fmt")\nfile(%s)\nline("size_t_fmt")\n", ptr, heaptag, heap, size, tag, line);
            StackTraceToFile(g_file);
            LeaveCriticalSection(&g_access);
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void HEART_API TrackFree( void* heap, void* ptr, const hChar* heaptag )
    {
#ifdef HEART_TRACK_MEMORY_ALLOCS
        if (g_open)
        {
            EnterCriticalSection(&g_access);
            fprintf(g_file, "!! FREE\n"
                "heap(%s,%p)\naddress(%p)\n", heaptag, heap, ptr);
            StackTraceToFile(g_file);
            LeaveCriticalSection(&g_access);
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void HEART_API TrackPushMarker(const hChar* heaptag)
    {
#ifdef HEART_TRACK_MEMORY_ALLOCS
        if (g_open)
        {
            EnterCriticalSection(&g_access);
            hcAssertMsg(hStrChr(heaptag, ' ') == 0, "Heap tags cannot contain spaces");
            fprintf(g_file, "!! MARKERPUSH %s\n", heaptag);
            LeaveCriticalSection(&g_access);
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void HEART_API TrackPopMarker()
    {
#ifdef HEART_TRACK_MEMORY_ALLOCS
        if (g_open)
        {
            EnterCriticalSection(&g_access);
            fprintf(g_file, "!! MARKERPOP\n");
            LeaveCriticalSection(&g_access);
        }
#endif
    }
#endif
}
}