/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "memtracktypes.h"
#include <vector>
#include <string>
#include <map>

struct Callstack {
    typedef std::vector<uint64>             BacktraceType;
    typedef std::map<uint64, std::string>   SymbolMapType;

    Callstack() 
        : line_(0) {
    }

    void            initilaise(const char* source, uint line);
    void            insertBacktraceLevel(uint level, uint64 address);

    static void     clearSymbolMap();
    static void     insertSymbol(uint64 address, const char* symbol);
    static bool     findSymbol(uint64 address);
    static bool     addressSymbolLookup(uint64 address, std::string* outsymbol);

    uint            line_;
    std::string     sourcePath_;
    BacktraceType   backtrace_;

    static SymbolMapType    s_symbolMap;
};
