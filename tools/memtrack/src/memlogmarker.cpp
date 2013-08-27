/********************************************************************

	filename: 	memlogmarker.cpp	
	
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

#include "precompiled.h"
#include "memlogmarker.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MemLogMarker::insertMemAlloc(const AllocRecord& allocr)
{
    firstAllocID_ = (allocr.uid_ < firstAllocID_) ? allocr.uid_ : firstAllocID_;

    allocs_.push_back(allocr);

    AllocMapType::iterator foundalloc = heapView_.find(allocr);
    if (foundalloc != heapView_.end()){
        wxString msg;
        msg.Printf("Warning: allocation 0x%llX has been allocated twice, this suggests that the Free() was done but not tracked\n"
            "Original allocation was made from %s(%u)\n"
            "Second allocation was made from %s(%u)\n", 
            allocr.address_, foundalloc->backtrace_.sourcePath_.c_str(), foundalloc->backtrace_.line_,
            allocr.backtrace_.sourcePath_.c_str(), allocr.backtrace_.line_);
        wxMessageDialog mb(NULL, msg);
        mb.ShowModal();
    }
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
