/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "memlogmarker.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLogMarker::insertMemAlloc(const AllocRecord& allocr)
{
    firstAllocID_ = (allocr.uid_ < firstAllocID_) ? allocr.uid_ : firstAllocID_;

    allocs_.push_back(allocr);

    AllocMapType::iterator foundalloc = heapView_.find(allocr);
    assert(foundalloc != heapView_.end());
    heapView_.insert(allocr);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool MemLogMarker::insertMemFree(const FreeRecord& freer)
{
    AllocRecord allr;

    allr.address_ = freer.address_;
   
    eraseAllocInclusive(allr);

    frees_.push_back(freer);
    //heapView_.erase(allr);
    
    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool MemLogMarker::ownAllocOrParentsDo(const FreeRecord& freer) {
    AllocRecord allr;
    allr.address_ = freer.address_;
    if (heapView_.find(allr) == heapView_.end()) {
        if (parent_) {
            return parent_->ownAllocOrParentsDo(freer);
        }
        return false;
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLogMarker::addChild(MemLogMarker* child)
{
    child->setParent(this);
    children_.push_back(child);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLogMarker::getAliveAllocsInclusive(AllocVectorType* aliveOut) const {
    getAliveAllocsExclusive(aliveOut);
    for (auto i=children_.begin(), n=children_.end(); i!=n; ++i) {
        (*i)->getAliveAllocsInclusive(aliveOut);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLogMarker::getAliveAllocsExclusive(AllocVectorType* aliveOut) const {
    for (auto i = heapView_.begin(), iend = heapView_.end(); i != iend; ++i)
    {
        aliveOut->push_back(*i);
    }
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLogMarker::eraseAllocInclusive(const AllocRecord& allocr) {
    if (heapView_.find(allocr) != heapView_.end()) {
        heapView_.erase(allocr);
    }
    for (auto i=children_.begin(), n=children_.end(); i!=n; ++i) {
        (*i)->eraseAllocInclusive(allocr);
    }
}
