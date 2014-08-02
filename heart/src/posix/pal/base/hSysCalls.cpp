/********************************************************************

    filename: 	hSysCalls.cpp	
    
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

#include "pal/hPlatform.h"
#include <sys/types.h>
#include <unistd.h>
#include <execinfo.h>

namespace Heart {
namespace hSysCall {

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint64 HEART_API GetProcessorSpeed() {
        FILE* fd = fopen("/proc/cpuinfo", "rb");

        if (!fd) {
            return 0;
        }

        // /proc/cpuinfo contains the current MHz that each core is running at. 
        // e.g. 
        // processor    : 0
        // cpu MHz      : 800.000
        //
        // processor    : 1
        // cpu MHz      : 800.000
        // However, I'm not sure this information is actually useful to us.

        fclose(fd);

        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void HEART_API GetCurrentWorkingDir(hChar* out, hUint bufsize) {
        hUint64 pid = (hUint64)getpid();
        snprintf(out, bufsize, "/proc/%llu/cwd", pid);
    }

}

namespace hMemTracking {
    struct hMemTrackingHeader {
        hUint64   baseAddress_;
        char      moduleName_[512];
    };

namespace {
    // static const hUint32            g_maxSymbols = 4096;
    // static hUint32                  g_symbolsCount = 0;
    // static hBool                    g_open   = hFalse;
    // static _RTL_CRITICAL_SECTION    g_access;
    // static FILE*                    g_file = NULL;
    // static hUintptr_t               g_symbols[g_maxSymbols];
}

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
namespace hSysCall
{
namespace {
    static hBool                    g_doneInit=hFalse;
}
    void hInitSystemDebugLibs() {
        // if (!g_doneInit) {
        //     hcAssert(g_doneInit==hFalse);
        //     g_doneInit=hFalse;
        // }
    }
}

namespace hMemTracking
{


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

    static void memTrackExitHandle()
    {

    }

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

    static hFORCEINLINE void StackTraceToFile(FILE* f)
    {
        static const hInt sym_limit = 4096;
        void* bt[sym_limit];
        backtrace_symbols_fd(bt, sym_limit, fileno(f));

        // static const hUint32 callstackLimit = 256;
        // unsigned int   i;
        // void         * stack[ 256 ];
        // unsigned short frames;
        
        // frames = CaptureStackBackTrace( 2, 256, stack, NULL );
        // hcAssert(frames > 0 && frames < 256);
        // for( i = 0; i < frames && i < 256; i++ )
        // {
        //     fprintf(g_file, "bt(%u,%p)\n", frames - i - 1, stack[i]);
        // }
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

}
}