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

namespace Heart
{

    class hRenderer;
    class hRenderMaterialManager;
    class hIFile;
    class hHeartEngine;

    class hResourceManager;
    struct hLoadedResourceInfo;
    class hIReferenceCounted;

    namespace hResourceEvent 
    {
        enum Type {
            AddResource,    // Adds a resource to the resource graph, does not fill in any data
            MarkAsRoot,     // Marks a node as a 

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

#if 0 // removing
    typedef std::unordered_multimap< hResourceID, hResourceEventProc, hResourceID::hash > hResourceEventMap;
#endif
    typedef std::unordered_multimap< hStringID, hNewResourceEventProc > hResourceNotifyTable;
#if 0 // removing
    typedef std::unordered_map< hResourceID, hResourceClassBase*, hResourceID::hash > hResourceHandleMap;
#endif

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class HEART_DLLEXPORT hResourceManager
    {
    public:
        hResourceManager();
        ~hResourceManager();

        // urgh...make these free functions
        static hResourceManager* get() { return instance_; }

        hBool                           initialise(hIFileSystem* pFileSystem, hJobManager* jobmanager);
        void                            update();
        void                            shutdown( hRenderer* prenderer );
        void                            printResourceInfo();
        // Will start garbage cycle if one is not started, runs cycle for step seconds
        void                            collectGarbage(hFloat step) {}
        void addResourceNode(hStringID res_id) {
            if (resourceDB_.find(res_id) == resourceDB_.end()) {
                hResourceGraphNode new_node;
                new_node.colour_ = hResourceColour::Black; // protect if from the GC until the next cycle.
                new_node.resourceData_ = nullptr;
                resourceDB_.insert(hResourceTable::value_type(res_id, new_node));
            }
        }
        void resourceAddRef(hStringID res_id) {
            auto found_item = rootResources_.find(res_id);
            if (found_item == rootResources_.end()) {
                rootResources_.insert(hRootResourceSet::value_type(res_id, 1));
            } else if (found_item != rootResources_.end()) {
                ++found_item->second;
            }
        }
        void resourceDecRef(hStringID res_id) {
            auto found_item = rootResources_.find(res_id);
            hcAssert(found_item == rootResources_.end())
            if (found_item != rootResources_.end()) {
                --found_item->second;
                if (found_item->second == 0) {
                    rootResources_.erase(found_item);
                }
            }
        }
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
        void registerForResourceEvents(hStringID res_id, hNewResourceEventProc proc) {
            hcAssert(resourceDB_.find(res_id) == resourceDB_.end());
            auto found_item = resourceDB_.find(res_id);
            resourceNotify_.insert(hResourceNotifyTable::value_type(res_id, proc));
            if (found_item->second.resourceData_) {
                proc(res_id, hResourceEvent_DBInsert, found_item->second.typeID_, found_item->second.resourceData_);
            }
        }
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

        // New interface
        void                            loadPackage(const hChar* name);
        void                            unloadPackage(const hChar* name);
        hBool                           getIsPackageLoaded(const hChar* name);

    private:

        template< typename t_ty > 
        friend t_ty* hResourceHandle::weakPtr() const;

        struct hResourceDBEvent
        {
            hResourceEvent::Type    type_;
            hStringID               resID_;
            hStringID               typeID_;
            hNewResourceEventProc   proc_;
            hResourceNodeLinks      links_;
            union {
                struct {
                    void*       dataPtr_;
                } added_;
            };
        };

        typedef hMap< hUint32, hResourcePackage > hResourcePackageMap;
        typedef std::unordered_map< hStringID, hResourceGraphNode >  hResourceTable;
        typedef std::map< hStringID, hUint > hRootResourceSet;
        typedef std::queue< hResourceDBEvent > hResourceDBEventQueue;

        template< typename t_ty>
        t_ty* getResourceForHandle(hStringID res_id) {
            auto i = resourceDB_.find(res_id);
            if (i == resourceDB_.end()) {
                return (t_ty*)nullptr;
            }
            hcAssert(i->second.typeID_ == t_ty::getTypeNameStatic());
            return (t_ty*)i->second.resourceData_;
        }

        //NEW
        static hResourceManager*        instance_;
        hIFileSystem*                   filesystem_;
        hJobManager*                    jobManager_;
        hJobQueue                       fileReadJobQueue_;
        hJobQueue                       workerQueue_;

        hResourcePackageMap  activePackages_;

        //
        hdMutex                 resourceDBMtx_;
        hResourceNotifyTable    resourceNotify_;
        hResourceDBEventQueue   resourceEventQueue_;
        hResourceTable          resourceDB_;
        hRootResourceSet        rootResources_;
    };

}

#endif // RESOURCEMANAGER_H__