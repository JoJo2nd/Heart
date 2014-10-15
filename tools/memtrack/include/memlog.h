/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#ifndef MEMLOG_H__
#define MEMLOG_H__

#include "memlogmarker.h"

struct Callstack;
class IODevice;

class MemLog
{
public:

    typedef std::vector<AllocRecord>    AllocVectorType;
    typedef std::vector<MemLogMarker*>   MarkerStackType;

    MemLog() {}
    ~MemLog() {}

    void clear();
    void pushMemoryMarker(const char* name);
    void popMemoryMarker();
    void logMemoryAlloc(uint64 address, uint64 size, const char* heap, const Callstack& backtrace);
    void logMemoryFree(uint64 address, const char* heap, const Callstack& backtrace);
    void getAllMemoryLeaks(AllocVectorType* leaksArray);
    MarkerStackType::const_iterator getFirstMarker() const { return rootList_.begin(); }
    MarkerStackType::const_iterator getLastMarker() const { return rootList_.end(); }

private:

    typedef std::list<MemLogMarker>     MarkerListType;

    MarkerStackType rootList_;
    MarkerListType  markerList_;
    MarkerStackType markerStack_;
    uint64          nextID_;
};

int parseMemLog(const char* filepath, MemLog* log, IODevice* ioaccess);

#endif // MEMLOG_H__