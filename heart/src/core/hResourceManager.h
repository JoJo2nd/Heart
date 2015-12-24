/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef RESOURCEMANAGER_H__
#define RESOURCEMANAGER_H__

#include "base/hTypes.h"
#include "base/hStringID.h"
#include "core/hIFileSystem.h"
#include "components/hObjectFactory.h"
#include "threading/hTaskGraphSystem.h"
#include <unordered_map>
#include <vector>

namespace Heart {
namespace hResourceManager {
    struct hPackageHandle {
    };

    typedef void* hPackageLoadRequest;

    hBool   initialise(hIFileSystem* pFileSystem);
    hBool   systemResourcesReady();
    void    update();
    void    shutdown();
    // Will start garbage cycle if one is not started, runs cycle for step seconds
    void    collectGarbage(hFloat step);
    void    addResource(void* ptr, hStringID package_id, hStringID res_id, hStringID type_id, hObjectDestroyProc destructor);
    void    removeResource(hStringID res_id);
    //void*   pinResource(hStringID res_id);
    void*   weakResourceRef(hStringID res_id);
    //void    unpinResource(void* ptr);
    void    loadPackage(const hChar* name);
    void    unloadPackage(const hChar* name);
    hBool   getIsPackageLoaded(const hChar* name);
    void*   getResourcePtrType(hStringID res_id, hStringID* out_type_id);
    hStringID getResourceID(void* res_ptr);
    template< typename t_ty>
    t_ty* weakResource(hStringID res_id) {
        hStringID type_id;
        void* ptr = getResourcePtrType(res_id, &type_id);
        if (ptr && type_id == t_ty::getTypeNameStatic()) {
            return (t_ty*)ptr;
        }
        return nullptr;
    }

    // hot reload can only happen when all packages in memory are in a loaded state (for now)
    hBool canHotReload();
    hBool hotReload();
}

}

#endif // RESOURCEMANAGER_H__