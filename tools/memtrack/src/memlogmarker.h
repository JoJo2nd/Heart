/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

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
    size_t          getChildCount() { return children_.size(); }
    MemLogMarker*   getChild(size_t idx) const { return children_[idx]; }
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
