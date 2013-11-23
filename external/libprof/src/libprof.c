/********************************************************************

    filename:   libprof.c  
    
    Copyright (c) 7:11:2013 James Moran
    
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


#include <windows.h>
#include <dbghelp.h> //temp
#include <stdlib.h>
#include <stdio.h>
#include <intrin.h>
#include "libprof.h"
#pragma intrinsic(__rdtsc)

#define lpf_maxSamples          (1024*1024)
#define lpf_maxCallstackDepth   (4096)
#define lpf_maxPath             (4096)
#define lpf_64bit       (1)
#define lpf_threadlocal __declspec(thread)
#define lpf_false       (0)
#define lpf_true        (1)


#ifdef lpf_64bit
#   define lpf_naked 
#elif 0
#   define lpf_naked __declspec(naked)
#endif

typedef struct entry
{
    __int64        eip_func;        // 4/8: address of function
    __int64        eip_caller;      // 8/16: return address of caller
    __int64        time;            // 16/24: 
    __int64        child_time;      // 24/32:
    int            stack_depth;     // 28/36:
    int            stack_ptr;       // 32/40:
} entry_t;

typedef struct ProfileHeader
{
    double      m_cpu_speed;
    uintptr_t   m_base_address;
    int         m_samplesCount;
    char        m_module_name[512];
} ProfileHeader_t;

lpf_threadlocal int g_count = 0;
lpf_threadlocal entry_t g_trace[lpf_maxSamples];
lpf_threadlocal int g_trace_active = lpf_false;
lpf_threadlocal unsigned int g_call_stack[lpf_maxCallstackDepth];
lpf_threadlocal int g_stack_idx = 0;
lpf_threadlocal char g_output_filename[lpf_maxPath];
lpf_threadlocal FILE* g_trace_file = NULL;
lpf_threadlocal ProfileHeader_t g_header;
lpf_threadlocal uintptr_t g_base_address=0;
HANDLE g_symbol_handle=0;

static __int64 lpf_get_processor_speed()
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

////////////////////////////////////////////////////////////////////////////////////////////////
static void lpf_open_profile_samples() {
    if (g_symbol_handle==0) {
        g_symbol_handle=GetCurrentProcess();
    }
    g_base_address=SymGetModuleBase64(g_symbol_handle, (DWORD64)&lpf_open_profile_samples);
    g_trace_file = fopen(g_output_filename, "wb");
    GetModuleFileName(0,g_header.m_module_name,512);
    g_header.m_samplesCount = 0;
    g_header.m_base_address=g_base_address;
    g_header.m_cpu_speed = (double)lpf_get_processor_speed();//IGG::g_ProfileCon.m_cpu_speed;
    // write the module filename so we can find the symbols later
    fwrite(&g_header,sizeof(g_header),1,g_trace_file);
}

static void lpf_close_profile_samples() {
    fseek(g_trace_file, 0, SEEK_SET);
    // write the module filename so we can find the symbols later
    fwrite(&g_header,sizeof(g_header),1,g_trace_file);
    fclose(g_trace_file);
}

static void lpf_flush_profile_samples() {
    g_header.m_samplesCount += g_count;
    // write the samples
    fwrite(g_trace,sizeof(entry_t),g_count,g_trace_file);
    g_count=0;
}

void lpf_api lpf_begin_thread_profile(const char* samplesname) {
    if (samplesname && !g_trace_active) {
        g_trace_active=lpf_true;
        strcpy_s(g_output_filename, lpf_maxPath, samplesname);
        g_stack_idx=0;
        g_count=0;
        lpf_open_profile_samples();
    }
}

void lpf_api lpf_end_thread_profile(int discardresults) {
    if (!discardresults && g_trace_active) {
        lpf_flush_profile_samples();
        lpf_close_profile_samples();
    }
    g_trace_active=lpf_false;
}

int lpf_api lpf_is_thread_profiling(void) {
    return g_trace_active;
}

extern void lpf_naked _cdecl lpf_penter(const void* func) {
#ifdef lpf_64bit
    entry_t* trace;
    if (!g_trace_active) {
        return;
    }
    if (g_count==lpf_maxSamples) {
        lpf_flush_profile_samples();
    }
    g_call_stack[g_stack_idx]=g_count;
    trace=&g_trace[g_count++];
    trace->eip_func=(__int64)func;
    trace->time=__rdtsc();
    trace->child_time=0;
    trace->stack_depth=g_stack_idx++;
#else
#endif // HEART_32BIT
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
//  _pleave()
//
//  written by: Rob Wyatt
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////
extern void lpf_naked _cdecl lpf_pexit(const void* func) 
{
#ifdef lpf_64bit
    entry_t* trace;
    if (!g_trace_active) {
        return;
    }
    g_stack_idx--;
    trace=&g_trace[g_call_stack[g_stack_idx]];
    trace->eip_caller=(__int64)func;
    trace->time=__rdtsc()-trace->time;
    //trace->stack_depth=g_stack_idx;

    if (g_stack_idx>1)
    {
        g_trace[g_call_stack[g_stack_idx-1]].child_time+=trace->time;
    }
#else
#endif // HEART_64BIT
}
