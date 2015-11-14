/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "core/hResourceManager.h"
#include "base/hMutexAutoScope.h"
#include "base/hMap.h"
#include "base/hStringID.h"
#include "base/hStringUtil.h"
#include "base/hProfiler.h"
#include "base/hCRC32.h"
#include "core/hResourcePackage.h"
#include "debug/hDebugMenuManager.h"
#include "imgui.h"
#include <unordered_map>
#include <map>
#include <stack>
#include <algorithm>

namespace Heart {
namespace hResourceManager {
namespace Hidden {
    struct hResourceContainer {
        hResourceContainer(hStringID type_name, void* resource, hResourcePackage* package) 
            : typeName(type_name)
            , resource(resource)
            , owningPackage(package) {
        }
        hStringID           typeName;
        void*               resource;
        hResourcePackage*   owningPackage;
    };

    typedef std::unordered_map< void*, hResourceContainer >     hResourceTable;
    typedef std::unordered_map< hStringID, hResourceContainer > hResourceNameToDataTable;
    typedef std::vector< hResourcePackage* >                    hPackageArray;

    hIFileSystem*               filesystem = nullptr;
    hJobManager*                jobManager = nullptr;
    hJobQueue                   fileReadJobQueue;
    hJobQueue                   workerQueue;

    //
    hMutex                      resourceDBMtx;
    hResourceNameToDataTable    resourceNameLookUp;
    hResourceTable              resources;
    hPackageArray               packages;
    hPackageArray               packagesToUnload;
}

using namespace Hidden;

#if HEART_DEBUG_INFO
void debugMenuRender();
#endif

hBool initialise(hIFileSystem* pFileSystem, hJobManager* jobmanager) {
    filesystem = pFileSystem;
    jobManager = jobmanager;
    loadPackage("system");
#if HEART_DEBUG_INFO
    hDebugMenuManager::registerMenu("Packages", debugMenuRender);
#endif
    return hTrue;
}

hBool systemResourcesReady() {
    return getIsPackageLoaded("system");
}

void shutdown() {
    unloadPackage("system");
    for (auto i : packages) {
        //i.dis();
    }
}

void update() {
    hMutexAutoScope sentry(&resourceDBMtx);
    HEART_PROFILE_FUNC();
    for (auto& i : packages) {
        i->update();
    }
    for (auto p = packagesToUnload.begin(); p!=packagesToUnload.end();) {
        (*p)->update();
        if ((*p)->isDestroyed()) {
            delete (*p);
            p = packagesToUnload.erase(p);
        } else {
            ++p;
        }
    }

    // Kick off any waiting jobs
    jobManager->kickQueueJobs(&fileReadJobQueue);
    jobManager->kickQueueJobs(&workerQueue);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void loadPackage( const hChar* name ) {
    hMutexAutoScope sentry(&resourceDBMtx);
    auto pkcrc = hStringID(name);
    auto lp=std::find_if(packages.begin(), packages.end(), [=](const hResourcePackage* lhs) {
        return lhs->getPackageID() == pkcrc;
    });
    if (lp == packages.end()) {
         hResourcePackage* pkg;
         pkg = new hResourcePackage;
         pkg->initialise(filesystem, &fileReadJobQueue, name);
         packages.push_back(pkg);
    } else {
        (*lp)->AddRef();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool getIsPackageLoaded( const hChar* name ) {
    hMutexAutoScope sentry(&resourceDBMtx);
    auto pkcrc = hStringID(name);
    auto lp=std::find_if(packages.begin(), packages.end(), [=](const hResourcePackage* lhs) {
        return lhs->getPackageID() == pkcrc;
    });
    return (lp != packages.end() && (*lp)->isInReadyState()) ? hTrue : hFalse;
}

void unloadPackage(const hChar* name) {
    hMutexAutoScope sentry(&resourceDBMtx);
     auto pkcrc = hStringID(name);
     auto pkg = std::find_if(packages.begin(), packages.end(), [=](const hResourcePackage* lhs) {
         return lhs->getPackageID() == pkcrc;
     });
     hcAssertMsg(pkg != packages.end(), "Couldn't find package \"%s\" to unload", name);
     if ((*pkg)->DecRef() == 0) {
         (*pkg)->unload();
     }
     // TODO: not do this here, forces the package to clean up but package
     // could left and re-loaded cheaply until memory was needed for other packages.
     collectGarbage(0.f);
}

void    addResource(void* ptr, hStringID package_id, hStringID res_id, hStringID type_id, hObjectDestroyProc destructor) {
    hMutexAutoScope sentry(&resourceDBMtx);
    auto lp = std::find_if(packages.begin(), packages.end(), [=](const hResourcePackage* lhs) {
        return lhs->getPackageID() == package_id;
    });
    hcAssert(resources.find(ptr) == resources.end() && lp != packages.end());
    resources.insert(hResourceTable::value_type(ptr, hResourceContainer(type_id, ptr, *lp)));
    resourceNameLookUp.insert(hResourceNameToDataTable::value_type(res_id, hResourceContainer(type_id, ptr, *lp)));
}

void    removeResource(hStringID res_id) {
    hMutexAutoScope sentry(&resourceDBMtx);
    const auto it = resourceNameLookUp.find(res_id);
    if (it != resourceNameLookUp.end()) {
        resources.erase(it->second.resource);
        resourceNameLookUp.erase(it);
    }
}

void*   pinResource(hStringID res_id) {
    hMutexAutoScope sentry(&resourceDBMtx);
    auto it = resourceNameLookUp.find(res_id);
    if (it == resourceNameLookUp.end()) {
        return nullptr;
    }
    it->second.owningPackage->AddRef();
    return it->second.resource;
}

void*   weakResourceRef(hStringID res_id) {
    hMutexAutoScope sentry(&resourceDBMtx);
    auto it = resourceNameLookUp.find(res_id);
    if (it == resourceNameLookUp.end()) {
        return nullptr;
    }
    return it->second.resource;
}

void    unpinResource(void* ptr) {
    hMutexAutoScope sentry(&resourceDBMtx);
    auto it = resources.find(ptr);
    hcAssert(it != resources.end());
    it->second.owningPackage->DecRef();
}

void* getResourcePtrType(hStringID res_id, hStringID* out_type_id) {
    hMutexAutoScope sentry(&resourceDBMtx);
    auto it = resourceNameLookUp.find(res_id);
    if (it == resourceNameLookUp.end()) {
        return nullptr;
    }
    *out_type_id = it->second.typeName;
    return it->second.resource;
}

void collectGarbage(hFloat step) {
    hMutexAutoScope sentry(&resourceDBMtx);
    for (auto i = packages.begin(); i!=packages.end();) {
        if ((*i)->GetRefCount() == 0) {
            (*i)->beginPackageDestroy();
            packagesToUnload.push_back(*i);
            i = packages.erase(i);
        } else {
            ++i;
        }
    }
}

#if HEART_DEBUG_INFO
void debugMenuRender() {
    hMutexAutoScope sentry(&resourceDBMtx);
    ImGui::Begin("Packages & Resources", nullptr, ImGuiWindowFlags_ShowBorders);

    if (ImGui::TreeNode("Loaded Packages")) {
        for (auto i : packages) {
            if (ImGui::TreeNode(i->getPackageName())) {
                i->printResourceInfo();
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }

    ImGui::End();
}
#endif
}
}