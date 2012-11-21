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

//#define HEART_PRINT_SYMBOLS
#define HEART_MEMTRACK_FULL_STACKTRACK

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
    static const hUint32            g_maxSymbols = 4096;
    static hUint32                  g_symbolsCount = 0;
    static hBool                    g_open   = hFalse;
    static _RTL_CRITICAL_SECTION    g_access;
    static FILE*                    g_file = NULL;
    static hUint32                  g_symbols[g_maxSymbols];
}

void FlushSymbolsToLogFile();

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

    int SymCmp(const void* a, const void* b)
    {
        if ( *(hUint32*)a >  *(hUint32*)b ) return 1;
        //if ( *(hUint32*)a == *(hUint32*)b ) return 0;
        if ( *(hUint32*)a <  *(hUint32*)b ) return -1;
        return 0;
    }

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

    static void AddSymbolToLogFile(void* symadd)
    {
        if (bsearch(&symadd, g_symbols, g_symbolsCount, sizeof(hUint32), SymCmp) != NULL) return;

        g_symbols[g_symbolsCount++] = (hUint32)symadd;

        qsort(g_symbols, g_symbolsCount, sizeof(hUint32), SymCmp);

        if (g_symbolsCount >= g_maxSymbols)
        {
            FlushSymbolsToLogFile();
            g_symbolsCount = 0;
        }
    }

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

    static void FlushSymbolsToLogFile()
    {
        static HANDLE   process = 0;
        SYMBOL_INFO*    symbol;
        DWORD           dwDisplacement;
        IMAGEHLP_LINE64 line;

        if (!g_symbolsCount) return;

        if (!process)
        {
            process = GetCurrentProcess();

            SymInitialize( process, NULL, TRUE );
            SymSetOptions(SYMOPT_LOAD_LINES);
        }

        symbol               = ( SYMBOL_INFO * )hAlloca(sizeof( SYMBOL_INFO ) + 256 * sizeof( char ));
        symbol->MaxNameLen   = 255;
        symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        fprintf(g_file, "!! SYMBOLTABLE\n");
        for (hUint32 i = 0; i < g_symbolsCount; ++i)
        {
            SymFromAddr(process, (DWORD64)(g_symbols[i]), 0, symbol);
            SymGetLineFromAddr64(process, (DWORD64)(g_symbols[i]), &dwDisplacement, &line);
            fprintf(g_file, "st(%LLX,%s[%u])\n", g_symbols[i], symbol->Name, line.LineNumber);
        }
    }

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

    static void memTrackExitHandle()
    {
        EnterCriticalSection(&g_access);
        FlushSymbolsToLogFile();
        //TrackPopMarker(/*ROOT*/);
        LeaveCriticalSection(&g_access);
    }

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

    static void StackTraceToFile(FILE* f)
    {
        unsigned int   i;
        void         * stack[ 100 ];
        unsigned short frames;
        
        frames = CaptureStackBackTrace( 0, 100, stack, NULL );
        for( i = 0; i < frames; i++ )
        {
            AddSymbolToLogFile(stack[i]);
            fprintf(g_file, "bt(%u,%LLX)\n", frames - i - 1, stack[i]);
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
                atexit(memTrackExitHandle);
                //TrackPushMarker("ROOT");
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
            fprintf(g_file, "!! ALLOC\n"
                "address(%LLX)\nheap(%s,%LLX)\nsize(%u)\nfile(%s)\nline(%u)\n", ptr, heaptag, heap, size, tag, line);
            StackTraceToFile(g_file);
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
            fprintf(g_file, "!! ALLOC\n"
                "address(%LLX)\nheap(%s,%LLX)\nsize(%u)\nfile(%s)\nline(%u)\n", ptr, heaptag, heap, size, tag, line);
            StackTraceToFile(g_file);
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
            fprintf(g_file, "!! FREE\n"
                "heap(%s,%LLX)\naddress(%LLX)\n", heaptag, heap, ptr);
            StackTraceToFile(g_file);
            LeaveCriticalSection(&g_access);
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    void HEART_API TrackPushMarker(const hChar* heaptag)
    {
#ifdef HEART_TRACK_MEMORY_ALLOCS
        if (g_open)
        {
            EnterCriticalSection(&g_access);
            fprintf(g_file, "!! MARKERPUSH %s\n", heaptag);
            LeaveCriticalSection(&g_access);
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
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