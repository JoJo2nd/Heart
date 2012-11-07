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

#include <dbghelp.h>

namespace Heart
{
namespace hSysCall
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hUint64 HEART_API GetProcessorSpeed()
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
namespace
{
    static hBool                    g_open   = hFalse;
    static _RTL_CRITICAL_SECTION    g_access;
    static FILE*                    g_file = NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

    static void StackTraceToFile(FILE* f)
    {
        unsigned int   i;
        void         * stack[ 100 ];
        unsigned short frames;
        SYMBOL_INFO  * symbol;
        static HANDLE  process = 0;

        if (!process)
        {
            process = GetCurrentProcess();

            SymInitialize( process, NULL, TRUE );
        }

        frames               = CaptureStackBackTrace( 0, 100, stack, NULL );
        symbol               = ( SYMBOL_INFO * )hAlloca(sizeof( SYMBOL_INFO ) + 256 * sizeof( char ));
        symbol->MaxNameLen   = 255;
        symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

        for( i = 2; i < frames; i++ )
        {
#ifdef HEART_PRINT_SYMBOLS
            SymFromAddr( process, ( DWORD64 )( stack[ i ] ), 0, symbol );
            fprintf(g_file, "%i: %s - 0x%08X\n", frames - i - 1, symbol->Name, symbol->Address);
#else
            fprintf(g_file, "%i: 0x%08X\n", frames - i - 1, stack[i]);
#endif
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    void HEART_API InitMemTracking()
    {
#ifdef HEART_TRACK_MEMORY_ALLOCS
        InitializeCriticalSection(&g_access);
        if (g_file == NULL)
        {
            if (g_file = fopen("mem_track.txt", "w"))
            {
                g_open = hTrue;
            }
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    void HEART_API TrackAlloc( const hChar* tag, hUint32 line, void* heap, void* ptr, hUint32 size, const hChar* heaptag )
    {
#ifdef HEART_TRACK_MEMORY_ALLOCS
        if (g_open)
        {
            EnterCriticalSection(&g_access);
            fprintf(g_file, "\\!! ALLOC - 0x%08X\n"
                "heap: %s (0x%08X) ptr: 0x%08X size: %u file: %s(%u)\n", ptr, heaptag, heap, ptr, size, tag, line);
#ifdef HEART_MEMTRACK_FULL_STACKTRACK
            StackTraceToFile(g_file);
#endif
            LeaveCriticalSection(&g_access);
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    void HEART_API TrackRealloc( const hChar* tag, hUint32 line, void* heap, void* ptr, hUint32 size, const hChar* heaptag )
    {
#ifdef HEART_TRACK_MEMORY_ALLOCS
        if (g_open)
        {
            EnterCriticalSection(&g_access);
            fprintf(g_file, "\\!! ALLOC - 0x%08X\n"
                "heap: %s (0x%08X) ptr: 0x%08X size: %u file: %s(%u)\n", ptr, heaptag, heap, ptr, size, tag, line);
#ifdef HEART_MEMTRACK_FULL_STACKTRACK
            StackTraceToFile(g_file);
#endif
            LeaveCriticalSection(&g_access);
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    void HEART_API TrackFree( void* heap, void* ptr, const hChar* heaptag )
    {
#ifdef HEART_TRACK_MEMORY_ALLOCS
        if (g_open)
        {
            EnterCriticalSection(&g_access);
            fprintf(g_file, "\\!! FREE - 0x%08X\n"
                "heap: %s (0x%08X) ptr: 0x%08X\n", ptr, heaptag, heap, ptr);
#ifdef HEART_MEMTRACK_FULL_STACKTRACK
            StackTraceToFile(g_file);
#endif
            LeaveCriticalSection(&g_access);
        }
#endif
    }

}
}