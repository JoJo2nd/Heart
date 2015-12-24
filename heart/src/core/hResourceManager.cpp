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
#include "components/hEntityFactory.h"
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
        hResourceContainer(hStringID type_name, hStringID in_res_name, void* resource, hResourcePackage* package) 
            : typeName(type_name)
            , resource(resource)
            , owningPackage(package) {
        }
        hStringID           typeName;
        hStringID           resName;
        void*               resource;
        hResourcePackage*   owningPackage;
    };

    typedef std::unordered_map< void*, hResourceContainer >     hResourceTable;
    typedef std::unordered_map< hStringID, hResourceContainer > hResourceNameToDataTable;
    typedef std::vector< hResourcePackage* >                    hPackageArray;

    hIFileSystem*               filesystem = nullptr;

    //
    hMutex                      resourceDBMtx;
    hResourceNameToDataTable    resourceNameLookUp;
    hResourceTable              resources;
    hPackageArray               packages;
    hPackageArray               packagesToLoad;
    hPackageArray               packagesToUnload;
    hBool                       awatingHotReload;
}

using namespace Hidden;

#if HEART_DEBUG_INFO
void debugMenuRender();
#endif

hBool initialise(hIFileSystem* pFileSystem) {
    filesystem = pFileSystem;
    awatingHotReload = hFalse;
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
    for (auto& i : packagesToLoad) {
        packages.push_back(i);
    }
    packagesToLoad.clear();
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
    auto lp2 = std::find_if(packagesToLoad.begin(), packagesToLoad.end(), [=](const hResourcePackage* lhs) {
        return lhs->getPackageID() == pkcrc;
    });
    if (lp == packages.end() && lp2 == packagesToLoad.end()) {
         hResourcePackage* pkg;
         pkg = new hResourcePackage;
         pkg->initialise(filesystem, name);
         packagesToLoad.push_back(pkg);
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
    resources.insert(hResourceTable::value_type(ptr, hResourceContainer(type_id, res_id, ptr, *lp)));
    resourceNameLookUp.insert(hResourceNameToDataTable::value_type(res_id, hResourceContainer(type_id, res_id, ptr, *lp)));
}

void    removeResource(hStringID res_id) {
    hMutexAutoScope sentry(&resourceDBMtx);
    const auto it = resourceNameLookUp.find(res_id);
    if (it != resourceNameLookUp.end()) {
        resources.erase(it->second.resource);
        resourceNameLookUp.erase(it);
    }
}
/*
void*   pinResource(hStringID res_id) {
    hMutexAutoScope sentry(&resourceDBMtx);
    auto it = resourceNameLookUp.find(res_id);
    if (it == resourceNameLookUp.end()) {
        return nullptr;
    }
    it->second.owningPackage->AddRef();
    return it->second.resource;
}
*/
void*   weakResourceRef(hStringID res_id) {
    hMutexAutoScope sentry(&resourceDBMtx);
    auto it = resourceNameLookUp.find(res_id);
    if (it == resourceNameLookUp.end()) {
        return nullptr;
    }
    return it->second.resource;
}
/*
void    unpinResource(void* ptr) {
    hMutexAutoScope sentry(&resourceDBMtx);
    auto it = resources.find(ptr);
    hcAssert(it != resources.end());
    it->second.owningPackage->DecRef();
}
*/
void* getResourcePtrType(hStringID res_id, hStringID* out_type_id) {
    hMutexAutoScope sentry(&resourceDBMtx);
    auto it = resourceNameLookUp.find(res_id);
    if (it == resourceNameLookUp.end()) {
        return nullptr;
    }
    *out_type_id = it->second.typeName;
    return it->second.resource;
}

Heart::hStringID getResourceID(void* res_ptr) {
    hMutexAutoScope sentry(&resourceDBMtx);
    auto it = resources.find(res_ptr);
    if (it == resources.end()) {
        return hStringID();
    }
    hcAssert(res_ptr == it->second.resource);
    return it->second.resName;
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

hBool canHotReload() {
    hMutexAutoScope sentry(&resourceDBMtx);
    for (auto i = packages.begin(); i != packages.end(); ++i) {
        if (!(*i)->isInReadyState()) return hFalse;
    }
    return hTrue;
}

hBool hotReload() {
    if (!canHotReload()) {
        return hFalse;
    }
    hMutexAutoScope sentry(&resourceDBMtx);
    // Serialise everything to disk to hot_reload.bin (include transient entities and optional entity data)
    hSerialisedEntitiesParameters state_obj;
    for (auto i = packages.begin(); i != packages.end(); ++i) {
        auto* lp = state_obj.engineState.add_loadedpacakges();
        lp->set_packagename((*i)->getPackageName());
        lp->set_refcount((*i)->GetRefCount());
    }
    hEntityFactory::serialiseEntities(&state_obj);
    // unload all packages.
    for (auto i = packages.begin(); i != packages.end(); ++i) {
        while ((*i)->DecRef()) {}
        (*i)->unload();
    }
    hEntityFactory::destroyAllEntities();
    collectGarbage(0.f);
    // reload packages
    for (hUint i = 0, n = state_obj.engineState.loadedpacakges_size(); i < n; ++i) {
        loadPackage(state_obj.engineState.loadedpacakges(i).packagename().c_str());
    }
    // Deserialise back in to memory from hot_reload.bin
    hEntityFactory::deserialiseEntities(state_obj);
    // let the normal frame flow re hook up resources pointers, etc.
    // NOTE: we don't correctly deal with reference counts on the packages yet. This 
    // may require a system to serialise them into the entities in such a way that only 
    // entities can request, load & unload packages (and thus know what's required in memory)
    return hTrue;
}

#if HEART_DEBUG_INFO
void debugMenuRender() {
    hMutexAutoScope sentry(&resourceDBMtx);
    ImGui::Begin("Packages & Resources", nullptr, ImGuiWindowFlags_ShowBorders);
    if (ImGui::Button("Hot Reload Packages")) {
        awatingHotReload = hTrue;
    }
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