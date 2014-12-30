/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "memtracker.h"
#include <stdio.h>
#include <windows.h>
#include <dbghelp.h>
#include <assert.h>
#include <vector>

static DWORD TlsKey;
static bool s_headerWritten;
static FILE* s_MemTrace;
static _RTL_CRITICAL_SECTION s_critSection;
volatile mt_int64 s_globalEventCounter;
static mt_uint64 s_threadIDCounter;

static const mt_uint32 s_localBufferSize = 4096;
static const mt_uint32 s_callstackBase = 0; //3;

struct MemTrackerCtx {
    _RTL_CRITICAL_SECTION* critSection;
    FILE*       memTrace;
    mt_uint64   threadID;
    mt_uint32   unflushedCount;
    mt_uint8    localBuffer[s_localBufferSize];
};

void mt_flush_thread_buffer(MemTrackerCtx* ctx) {
    EnterCriticalSection(ctx->critSection);
    if (!s_headerWritten) {
        char moduleFilename[512] = {0};
        mt_trace_header_t header;
        header.addressSizeBytes = sizeof(void*);
        header.baseAddress = 0;
        header.modulePathLen = 0;
        fwrite(&header, 1, sizeof(header), ctx->memTrace);
        fwrite(moduleFilename, 1, header.modulePathLen, ctx->memTrace);
        s_headerWritten = true;
    }
    fwrite(ctx->localBuffer, 1, ctx->unflushedCount, ctx->memTrace);
    ctx->unflushedCount = 0;
    LeaveCriticalSection(ctx->critSection);
}

BOOL WINAPI DllMain(void* _HDllHandle, unsigned int _Reason, void * _Reserved) {
    switch(_Reason) {
    case DLL_PROCESS_ATTACH: {
        TlsKey = ::TlsAlloc();
        s_globalEventCounter = 0;
        s_threadIDCounter = 0;
        s_headerWritten = false;
        s_MemTrace = fopen("memory.trace", "wb");
        InitializeCriticalSection(&s_critSection);
    }
    case DLL_THREAD_ATTACH: {
        auto* ctx = new MemTrackerCtx();
        ctx->critSection = &s_critSection;
        ctx->memTrace = s_MemTrace;
        ctx->unflushedCount = 0;
        ctx->threadID = s_threadIDCounter++;
        ::TlsSetValue(TlsKey, ctx);
    } break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH: {
        auto* ctx = (MemTrackerCtx*)::TlsGetValue(TlsKey);
        ::TlsSetValue(TlsKey, nullptr);
        mt_flush_thread_buffer(ctx);
        delete ctx;
    
        if (_Reason == DLL_PROCESS_DETACH) {
            fclose(s_MemTrace);
        } 
    }break;
    }

    return TRUE;
}

mt_dll_export void mt_api mem_track_alloc(void* ptr, size_t size, const char* tag) {
    static const mt_uint32 callstackLimit = 256;
    void* stack[256];
    mt_uint32 frames;
    mt_trace_chunk_t cnk_header;
    mt_trace_backtrace_t cnk_bt;
    MemTrackerCtx* ctx = (MemTrackerCtx*)::TlsGetValue(TlsKey);
    // null is possible when setting up thread structures.
    if (!ctx) {
        return;
    }

    cnk_header.chunkID = mt_chunk_id_memory_alloc_event;
    cnk_header.eventID = InterlockedIncrementAcquire64(&s_globalEventCounter);
    cnk_header.threadID = ctx->threadID;
    frames = CaptureStackBackTrace(s_callstackBase, 256, stack, NULL);

    auto tag_len = tag ? strlen(tag) : 0;
    if (tag_len > 32) {
        tag_len = 32;
    }
    cnk_bt.address = (mt_uint64)ptr;
    cnk_bt.size = size;
    cnk_bt.stackSize = frames;
    cnk_bt.tagLen = (mt_uint32)tag_len+1;

    cnk_header.chunkLen = sizeof(cnk_bt) + cnk_bt.tagLen + (cnk_bt.stackSize*sizeof(void*));

    mt_uint64 total_len = sizeof(cnk_header) + cnk_header.chunkLen;
    assert(total_len < s_localBufferSize);
    if (total_len >= (s_localBufferSize-ctx->unflushedCount)) {
        mt_flush_thread_buffer(ctx);
    }
    mt_uint8* b = ctx->localBuffer+ctx->unflushedCount;
    memcpy(b, &cnk_header, sizeof(cnk_header));
    b += sizeof(cnk_header);
    memcpy(b, &cnk_bt, sizeof(cnk_bt));
    b += sizeof(cnk_bt);
    void** st = (void**)b;
    b += (cnk_bt.stackSize*sizeof(void*));
    for (auto i=0ul, n=frames; i<n; ++i) {
        *st = stack[i];
        ++st;
    }
    memcpy(b, tag, tag_len);
    b[tag_len] = 0;
    ctx->unflushedCount += (mt_uint32)total_len;
}

mt_dll_export void mt_api mem_track_free(void* ptr) {
    static const mt_uint32 callstackLimit = 256;
    void* stack[256];
    mt_uint32 frames;
    mt_trace_chunk_t cnk_header;
    mt_trace_backtrace_t cnk_bt;
    MemTrackerCtx* ctx = (MemTrackerCtx*)::TlsGetValue(TlsKey);
    // null is possible when setting up thread structures.
    if (!ctx) {
        return;
    }

    cnk_header.chunkID = mt_chunk_id_memory_free_event;
    cnk_header.eventID = InterlockedIncrementAcquire64(&s_globalEventCounter);
    cnk_header.threadID = ctx->threadID;
    frames = CaptureStackBackTrace(s_callstackBase, 256, stack, NULL);

    cnk_bt.address = (mt_uint64)ptr;
    cnk_bt.size = 0;
    cnk_bt.stackSize = frames;
    cnk_bt.tagLen = 0;

    cnk_header.chunkLen = sizeof(cnk_bt) + (cnk_bt.stackSize*sizeof(void*));

    mt_uint64 total_len = sizeof(cnk_header) + cnk_header.chunkLen;
    assert(total_len < s_localBufferSize);
    if (total_len >= (s_localBufferSize - ctx->unflushedCount)) {
        mt_flush_thread_buffer(ctx);
    }
    mt_uint8* b = ctx->localBuffer + ctx->unflushedCount;
    memcpy(b, &cnk_header, sizeof(cnk_header));
    b += sizeof(cnk_header);
    memcpy(b, &cnk_bt, sizeof(cnk_bt));
    b += sizeof(cnk_bt);
    void** st = (void**)b;
    b += (cnk_bt.stackSize*sizeof(void*));
    for (auto i = 0ul, n = frames; i < n; ++i) {
        *st = stack[i];
        ++st;
    }
    ctx->unflushedCount += (mt_uint32)total_len;
}

mt_dll_export void mt_api mem_track_marker(const char* tag) {
    mt_trace_chunk_t cnk_header;
    MemTrackerCtx* ctx = (MemTrackerCtx*)::TlsGetValue(TlsKey);
    // null is possible when setting up thread structures.
    if (!ctx) {
        return;
    }

    auto tag_len = tag ? strlen(tag) : 0;
    if (tag_len > 32) {
        tag_len = 32;
    }
    cnk_header.chunkID = mt_chunk_id_marker_event;
    cnk_header.eventID = InterlockedIncrementAcquire64(&s_globalEventCounter);
    cnk_header.threadID = ctx->threadID;
    cnk_header.chunkLen = tag_len + 1;

    mt_uint64 total_len = sizeof(cnk_header) + cnk_header.chunkLen;
    assert(total_len < s_localBufferSize);
    if (total_len >= (s_localBufferSize - ctx->unflushedCount)) {
        mt_flush_thread_buffer(ctx);
    }
    mt_uint8* b = ctx->localBuffer + ctx->unflushedCount;
    memcpy(b, &cnk_header, sizeof(cnk_header));
    b += sizeof(cnk_header);
    memcpy(b, tag, tag_len);
    b[tag_len] = 0;
    ctx->unflushedCount += (mt_uint32)total_len;
}

BOOL mt_module_callback(_In_ PCSTR ModuleName, _In_ DWORD64 BaseOfDll, _In_opt_ PVOID UserContext) {
    std::vector<DWORD64>* bases = (std::vector<DWORD64>*)UserContext;
    bases->push_back(BaseOfDll);
    return TRUE;
}

mt_dll_export void mt_api mem_track_load_symbols() {
    MemTrackerCtx* ctx = (MemTrackerCtx*)::TlsGetValue(TlsKey);
    //SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_ALLOW_ABSOLUTE_SYMBOLS | SYMOPT_UNDNAME | SYMOPT_DEBUG | SYMOPT_LOAD_LINES);
    SymInitialize(GetCurrentProcess(), nullptr, TRUE);
    std::vector<DWORD64> bases;
    SymEnumerateModules64(GetCurrentProcess(), reinterpret_cast<PSYM_ENUMMODULES_CALLBACK64>(&mt_module_callback), &bases);

    for (const auto& BaseOfDll : bases) {
        mt_trace_chunk_t cnk_header;
        mt_trace_win32_symbols cnk_syms;
        //MemTrackerCtx* ctx = (MemTrackerCtx*)UserContext;
        IMAGEHLP_MODULE64 minfo;
        memset(&minfo, 0, sizeof(minfo));
        minfo.SizeOfStruct = sizeof(minfo);
        
        SymGetModuleInfo64(GetCurrentProcess(), BaseOfDll, &minfo);
        if (minfo.SymType != SymPdb) {
            continue;
        }

        cnk_syms.baseAddress = BaseOfDll;
        cnk_syms.moduleByteLen = minfo.ImageSize;
        cnk_syms.nameLen = (mt_uint32)strlen(minfo.ModuleName);
        cnk_syms.pathLen = (mt_uint32)strlen(minfo.LoadedImageName);
        
        cnk_header.chunkID = mt_chunk_id_win32_sym_event;
        cnk_header.eventID = InterlockedIncrementAcquire64(&s_globalEventCounter);
        cnk_header.threadID = ctx->threadID;
        cnk_header.chunkLen = sizeof(mt_trace_win32_symbols) + cnk_syms.nameLen + cnk_syms.pathLen + 2; //+2 for null terms
        
        mt_uint64 total_len = sizeof(cnk_header) + cnk_header.chunkLen;
        assert(total_len < s_localBufferSize);
        if (total_len >= (s_localBufferSize - ctx->unflushedCount)) {
            mt_flush_thread_buffer(ctx);
        }
        mt_uint8* b = ctx->localBuffer + ctx->unflushedCount;
        memcpy(b, &cnk_header, sizeof(cnk_header)); b += sizeof(cnk_header);
        memcpy(b, &cnk_syms, sizeof(cnk_syms)); b += sizeof(cnk_syms);
        memcpy(b, minfo.ModuleName, cnk_syms.nameLen); b += cnk_syms.nameLen; *b = 0; ++b;
        memcpy(b, minfo.LoadedImageName, cnk_syms.pathLen); b += cnk_syms.pathLen; *b = 0; ++b;
        ctx->unflushedCount += (mt_uint32)total_len;
    }
}
