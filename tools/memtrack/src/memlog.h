/********************************************************************

	filename: 	memlog.h	
	
	Copyright (c) 14:11:2012 James Moran
	
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
    void setBaseAddresses(uint64 oldbase, uint64 newbase) {
        oldBaseAddress_=oldbase;
        newBaseAddress_=newbase;
    }
    void pushMemoryMarker(const char* name);
    void popMemoryMarker();
    void logMemoryAlloc(uint64 address, uint64 size, const char* heap, const Callstack& backtrace);
    void logMemoryFree(uint64 address, const char* heap, const Callstack& backtrace);
    void getAllMemoryLeaks(AllocVectorType* leaksArray);
    MarkerStackType::const_iterator getFirstMarker() const { return rootList_.begin(); }
    MarkerStackType::const_iterator getLastMarker() const { return rootList_.end(); }

private:

    typedef std::list<MemLogMarker>     MarkerListType;

    uint64          oldBaseAddress_;
    uint64          newBaseAddress_;
    MarkerStackType rootList_;
    MarkerListType  markerList_;
    MarkerStackType markerStack_;
    uint64          nextID_;
};

int parseMemLog(const char* filepath, MemLog* log, IODevice* ioaccess);

#endif // MEMLOG_H__