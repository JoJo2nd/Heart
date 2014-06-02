/********************************************************************

    filename: 	hResourceManager.cpp
    
    Copyright (c) 2010/06/21 James Moran
    
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

#include "core/hResourceManager.h"
#include "base/hMap.h"
#include "base/hStringID.h"
#include "base/hProfiler.h"
#include "base/hCRC32.h"
#include "core/hResource.h"
#include "core/hResourcePackage.h"
#include <unordered_map>
#include <map>
#include <stack>

namespace Heart
{
namespace hResourceManager
{
namespace
{
    typedef hMap< hUint32, hResourcePackage > hResourcePackageMap;
    typedef std::unordered_map< hStringID, hResourceGraphNode >  hResourceTable;
    typedef std::map< hStringID, hUint > hRootResourceSet;

namespace hGCState
{
    enum Type {
        GatherRoots,
        Mark,
        Sweep,
        Idle,
    };
}

    struct hResourceGarbageCollector
    {
        typedef std::stack< hResourceGraphNode* > hGCStack;

        hResourceGarbageCollector() 
            : state_(hGCState::Idle)
        {}

        hGCState::Type              state_;
        hGCStack                    markStack_;
        hResourceTable::iterator    sweepItr_;
        hRootResourceSet::iterator  rootItr_;

        void step(hFloat limit) {

        }
    };

    hIFileSystem*               filesystem_ = nullptr;
    hJobManager*                jobManager_ = nullptr;
    hJobQueue                   fileReadJobQueue_;
    hJobQueue                   workerQueue_;
    hResourcePackageMap         activePackages_;

    //
    hMutex                     resourceDBMtx_;
    hResourceNotifyTable        resourceNotify_;
    hResourceTable              resourceDB_;
    hRootResourceSet            rootResources_;

    hResourceGarbageCollector   garbageCollector_;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool initialise(hIFileSystem* pFileSystem, hJobManager* jobmanager) {
    filesystem_ = pFileSystem;
    jobManager_ = jobmanager;
    return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void shutdown() {
    while(activePackages_.GetSize() > 0) {
        for (hResourcePackage* i=activePackages_.GetHead(); i!=hNullptr; i=i->GetNext()) {
            if (i->isInReadyState()) {
                i->beginUnload();
            }
        }
        update();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void update() {
    HEART_PROFILE_FUNC();
    for (hResourcePackage* i=activePackages_.GetHead(); i!=hNullptr; ) {
        i->update();
        if (i->unloaded()) {
            if (i->GetRefCount()==0) {
                hResourcePackage* next;
                activePackages_.Erase(i, &next);
                delete i; i = nullptr;
                i=next;
            } else {
                //reload the package
                i->beginLoad();
            }
        } else {
            i=i->GetNext();
        }
    }

    // Kick off any waiting jobs
    jobManager_->kickQueueJobs(&fileReadJobQueue_);
    jobManager_->kickQueueJobs(&workerQueue_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void loadPackage( const hChar* name ) {
    hUint32 pkcrc = hCRC32::StringCRC(name);
    hResourcePackage* pkg=activePackages_.Find(pkcrc);
    if (!pkg) {
        pkg = new hResourcePackage;
        pkg->initialise(filesystem_, &fileReadJobQueue_, &workerQueue_, name);
        activePackages_.Insert(pkcrc, pkg);
        pkg->beginLoad();
    } else {
        pkg->AddRef();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool getIsPackageLoaded( const hChar* name ) {
    hUint32 pkcrc = hCRC32::StringCRC(name);
    hResourcePackage* pkg=activePackages_.Find(pkcrc);
    return (pkg && pkg->isInReadyState()) ? hTrue : hFalse;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void unloadPackage(const hChar* name)
{
    hUint32 pkcrc = hCRC32::StringCRC(name);
    hResourcePackage* pkg=activePackages_.Find(pkcrc);
    hcAssertMsg(pkg, "Couldn't find package \"%s\" to unload", name);
    if (pkg) {
        pkg->DecRef();
        if (pkg->GetRefCount() == 0) {
            pkg->beginUnload();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void printResourceInfo()
{
    hcPrintf("=== Loaded Package Info Start ===");
    for (hResourcePackage* pack = activePackages_.GetHead(); pack; pack = pack->GetNext()) {
        hcPrintf("Package %s -- State: %s -- RC %u", 
            pack->getPackageName(), pack->getPackageStateString(), pack->GetRefCount());
        pack->printResourceInfo();
    }
    hcPrintf("=== Loaded Package Info End ===");
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void addResourceNode(hStringID res_id) {
    if (resourceDB_.find(res_id) == resourceDB_.end()) {
        hResourceGraphNode new_node;
        new_node.colour_ = hResourceColour::Black; // protect if from the GC until the next cycle.
        new_node.resourceData_ = nullptr;
        resourceDB_.insert(hResourceTable::value_type(res_id, new_node));
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void resourceAddRef(hStringID res_id) {
    auto found_item = rootResources_.find(res_id);
    if (found_item == rootResources_.end()) {
        rootResources_.insert(hRootResourceSet::value_type(res_id, 1));
    } else if (found_item != rootResources_.end()) {
        ++found_item->second;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void resourceDecRef(hStringID res_id) {
    auto found_item = rootResources_.find(res_id);
    hcAssert(found_item == rootResources_.end());
        if (found_item != rootResources_.end()) {
            --found_item->second;
            if (found_item->second == 0) {
                rootResources_.erase(found_item);
            }
        }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void insertResourceContainer(hStringID res_id, void* res_data, hStringID type_id) {
    hcAssert(resourceDB_.find(res_id) != resourceDB_.end());
    auto found_item = resourceDB_.find(res_id);
    found_item->second.typeID_ = type_id;
    found_item->second.resourceData_ = res_data;
    //notify listeners it's here!
    auto found_range = resourceNotify_.equal_range(res_id);
    for (auto i=found_range.first; i!=found_range.second; ++i) {
        i->second(res_id, hResourceEvent_DBInsert, type_id, res_data);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void removeResourceContainer(hStringID res_id) {
    hcAssert(resourceDB_.find(res_id) != resourceDB_.end());
    auto found_item = resourceDB_.find(res_id);
    //notify listeners it's about to go!
    auto found_range = resourceNotify_.equal_range(res_id);
    for (auto i=found_range.first; i!=found_range.second; ++i) {
        i->second(res_id, hResourceEvent_DBRemove, found_item->second.typeID_, nullptr);
    }
    found_item->second.resourceData_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void addResourceLink(hStringID res_id, hStringID* links, hUint num_links, hNewResourceEventProc proc) {
    hcAssert(resourceDB_.find(res_id) == resourceDB_.end());
    auto found_item = resourceDB_.find(res_id);
#ifdef HEART_DEBUG
    for (hUint i=0; i<num_links; ++i) {
        for (const auto& link : found_item->second.links_) {
            hcAssert(link != links[i]);
        }
    }
#endif
    for (hUint i=0; i<num_links; ++i) {
        found_item->second.links_.push_back(links[i]);
        registerForResourceEvents(links[i], proc);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void breakResourceLink(hStringID res_id, hStringID* links, hUint num_links, hNewResourceEventProc proc) {
    hcAssert(resourceDB_.find(res_id) == resourceDB_.end());
    auto found_item = resourceDB_.find(res_id);
    for (hUint i=0; i<num_links; ++i) {
        for (auto link = found_item->second.links_.begin(), nlink=found_item->second.links_.end(); link!=nlink; ++link) {
            if (*link == links[i]) {
                unregisterForResourceEvents(*link, proc); 
                found_item->second.links_.erase(link);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void registerForResourceEvents(hStringID res_id, hNewResourceEventProc proc) {
    hcAssert(resourceDB_.find(res_id) == resourceDB_.end());
    auto found_item = resourceDB_.find(res_id);
    resourceNotify_.insert(hResourceNotifyTable::value_type(res_id, proc));
    if (found_item->second.resourceData_) {
        proc(res_id, hResourceEvent_DBInsert, found_item->second.typeID_, found_item->second.resourceData_);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void unregisterForResourceEvents(hStringID res_id, hNewResourceEventProc proc) {
    hcAssert(resourceDB_.find(res_id) == resourceDB_.end());
    auto found_range = resourceNotify_.equal_range(res_id);
    for (auto i=found_range.first; i!=found_range.second; ++i) {
        if (i->second == proc) {
            resourceNotify_.erase(i);
            return;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* getResourcePtrType(hStringID res_id, hStringID* out_type_id) {
    hcAssert(out_type_id);
    auto i = resourceDB_.find(res_id);
    if (i == resourceDB_.end()) {
        return nullptr;
    }
    *out_type_id = i->second.typeID_;
    return i->second.resourceData_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void collectGarbage(hFloat step) {

}

}
}