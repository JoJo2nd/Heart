/********************************************************************

	filename: 	memlog.cpp	
	
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

#include "precompiled.h"
#include "memlog.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLog::clear()
{
    markerList_.clear();
    markerStack_.clear();
    nextID_ = 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLog::pushMemoryMarker(const char* name)
{
    MemLogMarker newmarker;

    newmarker.setName(name);
    markerList_.push_back(newmarker);
    if (markerStack_.size() > 0)
    {
        (*markerStack_.rbegin())->addChild(&(*markerList_.rbegin()));
    }
    markerStack_.push_back(&(*markerList_.rbegin()));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLog::popMemoryMarker()
{
    markerStack_.pop_back();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLog::logMemoryAlloc(uint64 address, uint64 size, const char* heap, const Callstack& backtrace)
{
    AllocRecord allr;

    allr.address_ = address;
    allr.size_ = size;
    allr.backtrace_ = backtrace;
    allr.heapID_ = 0;//TODO:
    allr.freeID_ = 0;
    allr.uid_ = nextID_;

    (*markerStack_.rbegin())->insertMemAlloc(allr);
    ++nextID_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLog::logMemoryFree(uint64 address, const char* heap, const Callstack& backtrace)
{
    FreeRecord freer;

    freer.address_ = address;
    freer.heapID_ = 0;//TODO: 
    freer.backtrace_ = backtrace;
    freer.uid_ = nextID_;
    freer.allocID_ = 0;

    for (MarkerStackType::reverse_iterator i = markerStack_.rbegin(), iend = markerStack_.rend(); 
        i != iend; ++i)
    {
        if ((*i)->insertMemFree(freer)) break;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLog::getAllMemoryLeaks(AllocVectorType* leaksArray)
{
    for (MarkerListType::reverse_iterator i = markerList_.rbegin(), iend = markerList_.rend();
        i != iend; ++i)
    {
        i->getAliveAllocs(leaksArray);
    }
}
