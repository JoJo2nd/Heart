/********************************************************************

	filename: 	memlogmarker.h	
	
	Copyright (c) 13:11:2012 James Moran
	
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

#ifndef MEMLOGMARKER_H__
#define MEMLOGMARKER_H__

#include "records.h"
#include <vector>
#include <set>

class MemLogMarker
{
public:

#define MemLogMarker_MAXNAMELEN (32)

    typedef std::vector<AllocRecord>    AllocVectorType;
    typedef std::vector<FreeRecord>     FreeVectorType;
    typedef std::set<AllocRecord>       AllocMapType;
    typedef std::vector<MemLogMarker*>  ChildVectorType;

    MemLogMarker() 
        : firstAllocID_(~0)
        , parent_(nullptr)
    {}
    ~MemLogMarker() {}

    void            setName(const char* name) { strcpy_s(markerName, MemLogMarker_MAXNAMELEN, name); }
    const char*     getName() const { return markerName; }
    void            insertMemAlloc(const AllocRecord& allocr);
    bool            insertMemFree(const FreeRecord& freer);
    void            addChild(MemLogMarker* child);
    void            setParent(MemLogMarker* parent) { parent_=parent; }
    MemLogMarker*   getParent() const;
    void            clearChildren() { children_.clear(); }
    uint            getChildCount() { return children_.size(); }
    MemLogMarker*   getChild(uint idx) const { return children_[idx]; }
    void            getAliveAllocsInclusive(AllocVectorType* aliveOut) const;
    void            getAliveAllocsExclusive(AllocVectorType* aliveOut) const;
    bool            ownAllocOrParentsDo(const FreeRecord& freer);

private:

    void            eraseAllocInclusive(const AllocRecord& allocr);

    char                markerName[MemLogMarker_MAXNAMELEN];
    MemLogMarker*       parent_;
    AllocVectorType     allocs_;
    FreeVectorType      frees_;
    AllocMapType        heapView_;
    ChildVectorType     children_;
    uint64              firstAllocID_;
};

#endif // MEMLOGMARKER_H__