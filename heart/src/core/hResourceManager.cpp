/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "core/hResourceManager.h"
#include "threading/hMutexAutoScope.h"
#include "base/hMap.h"
#include "base/hStringID.h"
#include "base/hProfiler.h"
#include "base/hCRC32.h"
#include "core/hResource.h"
#include "core/hResourcePackage.h"
#include <unordered_map>
#include <map>
#include <stack>
#include <algorithm>

namespace Heart
{
namespace hResourceManager
{
namespace Hidden
{
    enum class hResourceColour {
        White,
        Grey,
        Black,
    };

    struct hCollectableResource {
        void*               resource_;
        hAtomicInt          rootCount_;
        std::vector<void*>  links_;
        hResourceColour     colour_;

        hCollectableResource(void* resource) 
            : resource_(resource)
            , colour_(hResourceColour::Black) {
            hAtomic::AtomicSet(rootCount_, 1);
        }

        hUint32 addRef() {
            return hAtomic::Increment(rootCount_);
        }
        hUint32 decRef() {
            return hAtomic::Decrement(rootCount_);
        }
        void dispose() {
            hAtomic::AtomicSet(rootCount_, 0);
        }
        hUint32 ref() const {
            return hAtomic::AtomicGet(rootCount_);
        }
    };

    struct hResourceGraphNode {
        hResourceGraphNode()
            : resourceData_(nullptr)
            , colour_(hResourceColour::White)
        {

        }

        hStringID          typeID_;
        hResourceColour    colour_;
        void*              resourceData_;
        hResourceNodeLinks links_;
    };

    typedef std::unordered_map< void*, hCollectableResource >   hResourceTable;
    typedef std::unordered_map< hStringID, void* >              hResourceNameToDataTable;
    typedef std::vector< hResourcePackage* >                    hPackageArray;

    enum class hGCState {
        GatherRoots,
        Mark,
        Sweep,
        Idle,
    };

    struct hResourceGarbageCollector {
        typedef std::stack< hResourceGraphNode* > hGCStack;

        hResourceGarbageCollector() 
            : state_(hGCState::Idle)
        {}

        hGCState                    state_;
        hGCStack                    markStack_;
        hResourceTable::iterator    sweepItr_;

        void step(hFloat limit) {

        }
    };

    hIFileSystem*               filesystem_ = nullptr;
    hJobManager*                jobManager_ = nullptr;
    hJobQueue                   fileReadJobQueue_;
    hJobQueue                   workerQueue_;

    //
    hMutex                      resourceDBMtx_;
    hResourceNameToDataTable    resourceNameLookUp_;
    hResourceTable              resources_;
    hPackageArray               packages_;

    hResourceGarbageCollector   garbageCollector_;
}

using namespace Hidden;

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
    for (auto i : packages_) {
        //i.dis();
    }
    garbageCollector_.step(0.f);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void update() {
    HEART_PROFILE_FUNC();
    for (auto i : packages_) {
        i->update();
    }

    // Kick off any waiting jobs
    jobManager_->kickQueueJobs(&fileReadJobQueue_);
    jobManager_->kickQueueJobs(&workerQueue_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void loadPackage( const hChar* name ) {
    hMutexAutoScope sentry(&resourceDBMtx_);
    auto pkcrc = hStringID(name);
    auto lp=std::find_if(packages_.begin(), packages_.end(), [=](const hResourcePackage* lhs) {
        return lhs->getPackageCRC() == pkcrc.hash();
    });
    if (lp == packages_.end()) {
         hResourcePackage* pkg;
         pkg = new hResourcePackage;
         pkg->initialise(filesystem_, &fileReadJobQueue_, name);
         pkg->beginLoad();
         packages_.push_back(pkg);
    } else {
        resources_.find(*lp)->second.addRef();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool getIsPackageLoaded( const hChar* name ) {
    hMutexAutoScope sentry(&resourceDBMtx_);
    auto pkcrc = hStringID(name);
    auto lp=std::find_if(packages_.begin(), packages_.end(), [=](const hResourcePackage* lhs) {
        return lhs->getPackageCRC() == pkcrc.hash();
    });
    return (lp != packages_.end() && (*lp)->isInReadyState()) ? hTrue : hFalse;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void unloadPackage(const hChar* name)
{
//     hUint32 pkcrc = hCRC32::StringCRC(name);
//     auto pkg=activePackages_.find(pkcrc);
//     hcAssertMsg(pkg != activePackages_.end(), "Couldn't find package \"%s\" to unload", name);
//     pkg->second.defRef();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void printResourceInfo()
{
//     hcPrintf("=== Loaded Package Info Start ===");
//     for (hResourcePackage* pack = activePackages_.GetHead(); pack; pack = pack->GetNext()) {
//         hcPrintf("Package %s -- State: %s -- RC %u", 
//             pack->getPackageName(), pack->getPackageStateString(), pack->GetRefCount());
//         pack->printResourceInfo();
//     }
//     hcPrintf("=== Loaded Package Info End ===");
}

void    addResource(void* ptr, hStringID res_id, hObjectDestroyProc destructor) {
    hMutexAutoScope sentry(&resourceDBMtx_);
    hcAssert(resources_.find(ptr) == resources_.end());
    resources_.emplace(std::piecewise_construct, std::forward_as_tuple(ptr), std::forward_as_tuple(ptr));
    resourceNameLookUp_.insert(hResourceNameToDataTable ::value_type(res_id, ptr));
}

void    makeLink(void* resource, void* other) {
    hMutexAutoScope sentry(&resourceDBMtx_);
    hcAssert(resources_.find(resource) != resources_.end() && resources_.find(other) != resources_.end());
    resources_.find(resource)->second.links_.push_back(other);
}

void    removeLink(void* resource, void* other) {
    hMutexAutoScope sentry(&resourceDBMtx_);
    auto it = resources_.find(resource);
    auto& itres = it->second;
    hcAssert(it != resources_.end() && resources_.find(other) != resources_.end());
    auto pend = std::remove_if(itres.links_.begin(), itres.links_.end(), [=](void* lhs) {
       return  lhs == other;
    });
    itres.links_.erase(pend);
}

void*   pinResource(hStringID res_id) {
    hMutexAutoScope sentry(&resourceDBMtx_);
    auto it = resourceNameLookUp_.find(res_id);
    if (it == resourceNameLookUp_.end()) {
        return nullptr;
    }
    resources_.find(it->second)->second.addRef();
    return it->second;
}

void*   weakResourceRef(hStringID res_id) {
    hMutexAutoScope sentry(&resourceDBMtx_);
    auto it = resourceNameLookUp_.find(res_id);
    if (it == resourceNameLookUp_.end()) {
        return nullptr;
    }
    return it->second;
}

void    unpinResource(void* ptr) {
    hMutexAutoScope sentry(&resourceDBMtx_);
    auto it = resources_.find(ptr);
    hcAssert(it != resources_.end());
    hcAssert(it->second.ref() > 0);
    it->second.decRef();
}
/*
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void addResourceNode(hStringID res_id) {
    if (resourceNameLookUp_.find(res_id) == resourceNameLookUp_.end()) {
        hResourceGraphNode new_node;
        new_node.colour_ = hResourceColour::Black; // protect if from the GC until the next cycle.
        new_node.resourceData_ = nullptr;
        resourceNameLookUp_.insert(hResourceNameToDataTable::value_type(res_id, new_node));
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
    hcAssert(resourceNameLookUp_.find(res_id) != resourceNameLookUp_.end());
    auto found_item = resourceNameLookUp_.find(res_id);
    found_item->second.typeID_ = type_id;
    found_item->second.resourceData_ = res_data;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void removeResourceContainer(hStringID res_id) {
    hcAssert(resourceNameLookUp_.find(res_id) != resourceNameLookUp_.end());
    auto found_item = resourceNameLookUp_.find(res_id);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void addResourceLink(hStringID res_id, hStringID* links, hUint num_links) {
    hcAssert(resourceNameLookUp_.find(res_id) != resourceNameLookUp_.end());
    auto found_item = resourceNameLookUp_.find(res_id);
    for (hUint i=0; i<num_links; ++i) {
        found_item->second.links_.push_back(links[i]);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void breakResourceLink(hStringID res_id, hStringID* links, hUint num_links) {
    hcAssert(resourceNameLookUp_.find(res_id) == resourceNameLookUp_.end());
    auto found_item = resourceNameLookUp_.find(res_id);
    for (hUint i=0; i<num_links; ++i) {
        for (auto link = found_item->second.links_.begin(), nlink=found_item->second.links_.end(); link!=nlink; ++link) {
            if (*link == links[i]) {
                found_item->second.links_.erase(link);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void* getResourcePtrType(hStringID res_id, hStringID* out_type_id) {
    hcAssert(out_type_id);
    auto i = resourceNameLookUp_.find(res_id);
    if (i == resourceNameLookUp_.end()) {
        return nullptr;
    }
    *out_type_id = i->second.typeID_;
    return i->second.resourceData_;
}
*/
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void collectGarbage(hFloat step) {

}

}
}