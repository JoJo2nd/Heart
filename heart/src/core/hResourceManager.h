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
#include "threading/hJobManager.h"
#include <unordered_map>
#include <vector>

namespace Heart
{
    typedef std::vector< hStringID > hResourceNodeLinks;

    enum hResurceEvent 
    {
        hResourceEvent_None,
        hResourceEvent_Created,
        hResourceEvent_Linked,
        hResourceEvent_Unlinked,
        hResourceEvent_Unloaded,

        hResourceEvent_DBInsert,
        hResourceEvent_DBRemove,
        hResourceEvent_HotSwap,
    };

#if 0 // hResourceEventProc is replaced by hNewResoruceEventProc (awaiting rename)
    hFUNCTOR_TYPEDEF(hBool (*)(hResourceID , hResurceEvent, hResourceManager*, hResourceClassBase*), hResourceEventProc);
#endif
    hFUNCTOR_TYPEDEF(hBool (*)(hStringID/*res_id*/, hResurceEvent/*event_type*/, hStringID/*type_id*/, void* /*data_ptr*/), hNewResourceEventProc);

    
    class hRenderMaterialManager;
    class hIFile;
    class hHeartEngine;

    struct hLoadedResourceInfo;
    class hIReferenceCounted;

    namespace hResourceEvent 
    {
        enum Type {
            AddResource,    // Adds a resource to the resource graph, does not fill in any data
            MarkAsRoot,     // Marks a node as a Root node

            InsertResource, // Inserts resource data into a node in the graph, node must exist
            RemoveResource, // Inserts resource data into a node in the graph, node must exist

            RegisterHandler,
            UnregisterHandler,

            AddLinks,
            BreakLinks,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    typedef std::unordered_multimap< hStringID, hNewResourceEventProc > hResourceNotifyTable;

namespace hResourceManager
{
    typedef void* hPackageLoadRequest;

    hBool   initialise(hIFileSystem* pFileSystem, hJobManager* jobmanager);
    void    update();
    void    shutdown();
    void    printResourceInfo();
    // Will start garbage cycle if one is not started, runs cycle for step seconds
    void    collectGarbage(hFloat step);
    void    addResource(void* ptr, hStringID package_id, hStringID res_id, hStringID type_id, hObjectDestroyProc destructor);
    void    removeResource(hStringID res_id);
    void*   pinResource(hStringID res_id);
    void*   weakResourceRef(hStringID res_id);
    void    unpinResource(void* ptr);
    void    loadPackage(const hChar* name);
    void    unloadPackage(const hChar* name);
    hBool   getIsPackageLoaded(const hChar* name);
    void*   getResourcePtrType(hStringID res_id, hStringID* out_type_id);
    template< typename t_ty>
    t_ty* weakResource(hStringID res_id) {
        hStringID type_id;
        void* ptr = getResourcePtrType(res_id, &type_id);
        if (ptr && type_id == t_ty::getTypeNameStatic()) {
            return (t_ty*)ptr;
        }
        return nullptr;
    }
}

}

#endif // RESOURCEMANAGER_H__