/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "memtracker.h"
#include "memlogmarker.h"
#include "ioaccess.h"
#include <vector>
#include <memory>
#include <unordered_map>

class PlatformDebugSymbols {
public:
    PlatformDebugSymbols();
    virtual ~PlatformDebugSymbols();

    void        loadSymbolInfo(mt_uint64 base_address, mt_uint64 len, const char* sympath);
    const char* getSymbolFromAddress(mt_uint64 addr);
private:
    typedef std::unordered_map<mt_uint64, std::string> AddressTable;

    struct LoadedModule {
        mt_uint64 oldBase, newBase;
        mt_uint64 len;
    };
    std::vector<LoadedModule> loadedModules;
    AddressTable addressTable;
};

class MemLog
{
public:
    MemLog(InputStream* in_trace_file);
    ~MemLog();

    void listAllMarkers(FILE* output);
    bool getMarkers(FILE* output, const std::vector<std::string>& marker_list, mt_uint64* first, mt_uint64* last);
    void writeAllLeaks(FILE* output, mt_uint64 first_marker, mt_uint64 last_marker);

private:

    struct MemEvent {
        mt_trace_chunk_t  eventChunk;
        void*             memory;
    };

    typedef std::vector<MemEvent>       MemEventArray;

    InputStream* traceSource;
    mt_trace_header_t* header;
    const char* exePath;
    mt_trace_chunk_t* firstChunk;
    std::unique_ptr<PlatformDebugSymbols>   debugSymbols;
    MemEventArray events;
};

int parseMemLog(const char* filepath, MemLog** log, IODevice* ioaccess);
