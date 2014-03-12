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

    class hResourceEventUpdateProcess
    {
    public:
        hResourceEventUpdateProcess();
        ~hResourceEventUpdateProcess();

        hBool update();
        hBool isComplete() const;

    private:
        HEART_PRIVATE_COPY(hResourceEventUpdateProcess);
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class HEART_DLLEXPORT hResourceManager
    {
    public:
        hResourceManager();
        ~hResourceManager();

        hBool                           initialise(hHeartEngine* engine, hRenderer* renderer, hIFileSystem* pFileSystem, hJobManager* jobmanager, const char** requiredResources);
        void                            update();
        void                            shutdown( hRenderer* prenderer );
        void                            printResourceInfo();

        void insertResourceContainer(hStringID res_id, void* res_data, hStringID type_id) {
            resourceDBMtx_.Lock();
            hResourceDBEvent res_event;
            res_event.type_ = hResourceDBEvent::Type_InsertResource;
            res_event.resID_ = res_id;
            res_event.typeID_ = type_id;
            res_event.added_.dataPtr_ = res_data;
            resourceEventQueue_.push(res_event);
            resourceDBMtx_.Unlock();
        }
        void removeResourceContainer(hStringID res_id) {
            resourceDBMtx_.Lock();
            hResourceDBEvent res_event;
            res_event.type_ = hResourceDBEvent::Type_RemoveResource;
            res_event.resID_ = res_id;
            resourceEventQueue_.push(res_event);
            resourceDBMtx_.Unlock();
        }
        void registerForResourceEvents(hStringID res_id, hNewResourceEventProc proc) {
            resourceDBMtx_.Lock();
            hResourceDBEvent res_event;
            res_event.type_ = hResourceDBEvent::Type_RegisterHandler;
            res_event.resID_ = res_id;
            res_event.proc_ = proc;
            resourceEventQueue_.push(res_event);
            resourceDBMtx_.Unlock();
        }
        void unregisterForResourceEvents(hStringID res_id, hNewResourceEventProc proc) {
            resourceDBMtx_.Lock();
            hResourceDBEvent res_event;
            res_event.type_ = hResourceDBEvent::Type_UnregisterHandler;
            res_event.resID_ = res_id;
            res_event.proc_ = proc;
            resourceEventQueue_.push(res_event);
            resourceDBMtx_.Unlock();
        }

        // New interface
        void                            loadPackage(const hChar* name);
        void                            unloadPackage(const hChar* name);
        hBool                           getIsPackageLoaded(const hChar* name);

    private:

        friend hResourceClassBase* hResourceHandle::weakPtr() const;

        struct hResourceDBEvent
        {
            enum ResourceEventType 
            {
                Type_InsertResource,
                Type_RemoveResource,
                Type_RegisterHandler,
                Type_UnregisterHandler,
            };
            ResourceEventType       type_;
            hStringID               resID_;
            hStringID               typeID_;
            hNewResourceEventProc   proc_;
            union {
                struct {
                    void*       dataPtr_;
                } added_;
            };
        };

        typedef hMap< hUint32, hResourcePackage > hResourcePackageMap;
        typedef std::unordered_map< hStringID, hResourceContainer >  hResourceTable;
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
        hRenderer*                      renderer_;
        hRenderMaterialManager*         materialManager_;
        hIFileSystem*                   filesystem_;
        hJobManager*                    jobManager_;

        //main thread loaded packages
        hHeartEngine*                    engine_;

        hJobQueue fileReadJobQueue_;
        hJobQueue workerQueue_;

        hResourcePackageMap  activePackages_;

#if 0 //TODO: remove
        hResourceHandleMap  resourceHandleMap_;
        hResourceEventMap   resourceEventMap_;
#endif

        //

        hdMutex                 resourceDBMtx_;
        hResourceNotifyTable    resourceNotify_;
        hResourceDBEventQueue   resourceEventQueue_;
        hResourceTable          resourceDB_;
    };

}

#endif // RESOURCEMANAGER_H__