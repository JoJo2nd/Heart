/********************************************************************

    filename: 	hResourceManager.h
    
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

#ifndef RESOURCEMANAGER_H__
#define RESOURCEMANAGER_H__

#include "base/hTypes.h"
#include "base/hStringID.h"
#include "core/hIFileSystem.h"
#include "threading/hJobManager.h"
#include <unordered_map>

namespace Heart
{
    typedef std::vector< hStringID > hResourceNodeLinks;

    namespace hResourceColour
    {
        enum Type {
            White,
            Grey,
            Black,
        };
    }

    struct hResourceGraphNode
    {
        hResourceGraphNode()
            : resourceData_(nullptr)
            , colour_(hResourceColour::White)
        {

        }

        void*                   resourceData_;
        hStringID               typeID_;
        hResourceNodeLinks      links_;
        hResourceColour::Type   colour_;
    };

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
    hBool   initialise(hIFileSystem* pFileSystem, hJobManager* jobmanager);
    void    update();
    void    shutdown();
    void    printResourceInfo();
    // Will start garbage cycle if one is not started, runs cycle for step seconds
    void    collectGarbage(hFloat step);
    void    addResourceNode(hStringID res_id);
    void    resourceAddRef(hStringID res_id);
    void    resourceDecRef(hStringID res_id);
    void    insertResourceContainer(hStringID res_id, void* res_data, hStringID type_id);
    void    removeResourceContainer(hStringID res_id);
    void    addResourceLink(hStringID res_id, hStringID* links, hUint num_links, hNewResourceEventProc proc);
    void    breakResourceLink(hStringID res_id, hStringID* links, hUint num_links, hNewResourceEventProc proc);
    void    registerForResourceEvents(hStringID res_id, hNewResourceEventProc proc);
    void    unregisterForResourceEvents(hStringID res_id, hNewResourceEventProc proc);
    void    loadPackage(const hChar* name);
    void    unloadPackage(const hChar* name);
    hBool   getIsPackageLoaded(const hChar* name);
    void*   getResourcePtrType(hStringID res_id, hStringID* out_type_id);
    template< typename t_ty>
    t_ty* getResourceForHandle(hStringID res_id) {
        hStringID type_id;
        void* ptr = getResourcePtrType(res_id, &type_id);
        if (type_id == t_ty::getTypeNameStatic()) {
            return (t_ty*)ptr;
        }
        return nullptr;
    }
}

}

#endif // RESOURCEMANAGER_H__