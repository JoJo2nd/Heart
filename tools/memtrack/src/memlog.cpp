/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "memlog.h"
#include <algorithm>
#include <windows.h>
#include <dbghelp.h>
#include <assert.h>
#include <unordered_map>


MemLog::MemLog(InputStream* in_trace_file) 
    : traceSource(in_trace_file) 
    , header(nullptr)
    , debugSymbols(nullptr) {
    assert(in_trace_file);
    header = (mt_trace_header_t*)in_trace_file->getBase();
    exePath = (const char*)(header+1);
    debugSymbols.reset(new PlatformDebugSymbols());
    firstChunk = (mt_trace_chunk_t*)(exePath+header->modulePathLen);

    mt_uint8* ptr = (mt_uint8*)firstChunk;
    mt_uint8* ptr_end = ptr + in_trace_file->size();
    while (ptr < ptr_end) {
        if ((ptr + sizeof(mt_trace_chunk_t)) >= ptr_end) {
            ptr = ptr_end;
            continue;
        }
        auto* h = (mt_trace_chunk_t*)ptr;
        if ((ptr+sizeof(mt_trace_chunk_t)+h->chunkLen) >= ptr_end) {
            ptr = ptr_end;
            continue;
        }
        switch(h->chunkID) {
        case mt_chunk_id_memory_alloc_event:
        case mt_chunk_id_memory_free_event: {
            auto bt = (mt_trace_backtrace_t*)(h+1);
            MemEvent e;
            e.eventChunk = *h;
            e.memory = (void*)bt;
            events.push_back(e);
        } break;
        case mt_chunk_id_marker_event: {
            MemEvent e;
            e.eventChunk = *h;
            e.memory = (void*)(h + 1);
            events.push_back(e);
        } break;
        case mt_chunk_id_win32_sym_event: {
            auto* sym = (mt_trace_win32_symbols*)(h+1);
            char* sym_path = ((char*)(sym+1))+sym->nameLen+1;
            debugSymbols->loadSymbolInfo(sym->baseAddress, sym->moduleByteLen, sym_path);
        } break;
        }

        ptr += sizeof(mt_trace_chunk_t)+h->chunkLen;
    }

    std::sort(events.begin(), events.end(), [](const MemEvent& lhs, const MemEvent& rhs) {
        return lhs.eventChunk.eventID < rhs.eventChunk.eventID;
    });
}

MemLog::~MemLog() {

}

void MemLog::listAllMarkers(FILE* output) {
    for (const auto& i : events) {
        if (i.eventChunk.chunkID == mt_chunk_id_marker_event) {
            fprintf(output, "Marker: %s@%llu\n", (const char*)i.memory, i.eventChunk.eventID);
        }
    }
}

bool MemLog::getMarkers(FILE* output, const std::vector<std::string>& marker_list, mt_uint64* first, mt_uint64* last) {
    std::vector<mt_uint64> marker_ids;
    char tmp[4096];
    for (const auto& i : events) {
        if (i.eventChunk.chunkID == mt_chunk_id_marker_event) {
            sprintf(tmp, "%s@%llu", (const char*)i.memory, i.eventChunk.eventID);
            for (const auto& m : marker_list) {
                if (strcmp(tmp, m.c_str()) == 0) {
                    marker_ids.push_back(i.eventChunk.eventID);
                    break;
                }
            }
        }
    }

    std::sort(marker_ids.begin(), marker_ids.end());

    if (marker_ids.size() < 2) {
        return false;
    }
    *first = *marker_ids.begin();
    *last = *marker_ids.rbegin();
    return true;
}

void MemLog::writeAllLeaks(FILE* output, mt_uint64 first_marker, mt_uint64 last_marker) {
    typedef std::unordered_map<mt_uint64, MemEvent> map_type;
    map_type allocations;
    allocations.reserve(events.size()*2);
    for (const auto& i : events) {
        if (i.eventChunk.eventID < first_marker) {
            continue;
        }
        if (i.eventChunk.eventID > last_marker) {
            break;
        }
        if (i.eventChunk.chunkID == mt_chunk_id_memory_alloc_event) {
            auto* bt = (mt_trace_backtrace_t*)i.memory;
            allocations.insert(map_type::value_type(bt->address, i));
        } else if (i.eventChunk.chunkID == mt_chunk_id_memory_free_event) {
            auto* bt = (mt_trace_backtrace_t*)i.memory;
            auto it = allocations.find(bt->address);
            if (it != allocations.end()) {
                allocations.erase(it);
            }
        }
    }

    for (const auto& i : allocations) {
        auto* bt = (mt_trace_backtrace_t*)i.second.memory;
        fprintf(output, "Memory leak detected at Address 0x%016llX, Size %llu, EventID %llu, Callstack:\n", bt->address, bt->size, i.second.eventChunk.eventID);
        void** st = (void**)(bt+1);
        for (auto n=bt->stackSize, i=0ul; i<n; ++i) {
            fprintf(output, "\t%s\n", debugSymbols->getSymbolFromAddress((mt_uint64)st[i]));
        }
    }
}

PlatformDebugSymbols::PlatformDebugSymbols() {
    SymSetOptions(/*SYMOPT_DEFERRED_LOADS | */ SYMOPT_ALLOW_ABSOLUTE_SYMBOLS | SYMOPT_UNDNAME | SYMOPT_DEBUG | SYMOPT_LOAD_LINES | SYMOPT_INCLUDE_32BIT_MODULES);
    auto r = SymInitialize(GetCurrentProcess(), NULL, true);
    assert(r != 0);
    
}

PlatformDebugSymbols::~PlatformDebugSymbols() {
    SymCleanup(GetCurrentProcess());
}

const char* PlatformDebugSymbols::getSymbolFromAddress(mt_uint64 in_address) {
    auto it = addressTable.find(in_address);
    if (it != addressTable.end()) {
        return it->second.c_str();
    }

    uint64 address = in_address;
    for (const auto& i : loadedModules) {
        if (address >= i.oldBase && address < (i.oldBase+i.len)) {
            address -= i.oldBase;
            address += i.newBase;
        }
    }

    char            buffer[2048 + sizeof(SYMBOL_INFO)];
    char            outbuffer[4096];
    SYMBOL_INFO*    symbol = (SYMBOL_INFO*)buffer;
    DWORD           dwDisplacement;
    IMAGEHLP_LINE64 line;

    symbol->MaxNameLen = 2047;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    DWORD errorcode = 0;
    HANDLE process = GetCurrentProcess();
    if (!SymFromAddr(process, (DWORD64)address, 0, symbol)) {
        errorcode = GetLastError();
    }
    if (SymGetLineFromAddr64(process, (DWORD64)address, &dwDisplacement, &line)) {
        if (errorcode != 0) {
            sprintf_s(outbuffer, sizeof(outbuffer), "0x%p - %s(%u)", (void*)address, line.FileName, line.LineNumber);
        }
        else {
            sprintf_s(outbuffer, sizeof(outbuffer), "%s - %s(%u)", symbol->Name, line.FileName, line.LineNumber);
        }
    }
    else {
        if (errorcode != 0) {
            sprintf_s(outbuffer, sizeof(outbuffer), "0x%p", (void*)address);
        }
        else {
            sprintf_s(outbuffer, sizeof(outbuffer), "%s", symbol->Name);
        }
    }
    addressTable.insert(AddressTable::value_type(in_address, outbuffer));
    it = addressTable.find(in_address);
    return it->second.c_str();
}

void PlatformDebugSymbols::loadSymbolInfo(mt_uint64 base_address, mt_uint64 len, const char* sympath) {
    LoadedModule ld;
    ld.oldBase = base_address;
    ld.len = len;
    ld.newBase = SymLoadModuleEx(GetCurrentProcess(), 0, sympath, 0, 0, 0, 0, 0);
    loadedModules.push_back(ld);
}
