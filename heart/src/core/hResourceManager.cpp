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

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceManager::hResourceManager() 
    {
        // Do this as soon as possible, them resource handles can be created globally
        hResourceHandle initResourceStatics(this, hResourceID(0, 0));
        (void)initResourceStatics;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceManager::~hResourceManager()
    {
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourceManager::initialise( hHeartEngine* engine, hRenderer* renderer, hIFileSystem* pFileSystem, hJobManager* jobmanager, const char** requiredResources ) {
        hUint32 nRequiredResources = 0;
        hChar pluginpath[2048];
        filesystem_ = pFileSystem;
        renderer_ = renderer;
        materialManager_ = renderer->GetMaterialManager();
        jobManager_ = jobmanager;
        engine_ = engine;

        hd_AddSharedLibSearchDir(engine->GetWorkingDir());
        hStrPrintf(pluginpath, 2048, "%sPLUGIN/", engine->GetWorkingDir());
        hd_AddSharedLibSearchDir(pluginpath);
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::shutdown( hRenderer* prenderer ) {
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

    void hResourceManager::update() {
        HEART_PROFILE_FUNC();
        resourceDBMtx_.Lock();
        auto localEventQueue = resourceEventQueue_;
        resourceDBMtx_.Unlock();
        while (!localEventQueue.empty()) {
            hResourceDBEvent res_event = localEventQueue.front();
            switch (res_event.type_) {
            case hResourceDBEvent::Type_InsertResource: {
                //inject the resource
                hResourceContainer new_res;
                new_res.typeID_ = res_event.typeID_;
                new_res.resourceData_ = res_event.added_.dataPtr_;
                resourceDB_.insert(hResourceTable::value_type(res_event.resID_, new_res));
                //notify listeners
                auto container = resourceDB_.find(res_event.resID_);
                auto found_range = resourceNotify_.equal_range(res_event.resID_);
                for (auto i=found_range.first; i!=found_range.second; ++i) {
                    i->second(res_event.resID_, hResourceEvent_DBInsert, container->second.typeID_, container->second.resourceData_);
                }
            } break;
            case hResourceDBEvent::Type_RemoveResource: {
                //notify listeners
                auto container = resourceDB_.find(res_event.resID_);
                auto found_range = resourceNotify_.equal_range(res_event.resID_);
                for (auto i=found_range.first; i!=found_range.second; ++i) {
                    i->second(res_event.resID_, hResourceEvent_DBRemove, container->second.typeID_, container->second.resourceData_);
                }
                //remove the resource
                for (auto i=found_range.first; i!=found_range.second; ++i) {
                    if (i->second == res_event.proc_) {
                        resourceNotify_.erase(i);
                        break;
                    }
                }
            } break;
            case hResourceDBEvent::Type_RegisterHandler: {
            } break;
            case hResourceDBEvent::Type_UnregisterHandler: {
            } break;
            }
            localEventQueue.pop();
        }
        

        for (hResourcePackage* i=activePackages_.GetHead(); i!=hNullptr; ) {
            i->update(this);
            if (i->unloaded()) {
                if (i->GetRefCount()==0) {
                    hResourcePackage* next;
                    activePackages_.Erase(i, &next);
                    hDELETE_SAFE(i);
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

    void hResourceManager::loadPackage( const hChar* name ) {
        hUint32 pkcrc = hCRC32::StringCRC(name);
        hResourcePackage* pkg=activePackages_.Find(pkcrc);
        if (!pkg) {
            pkg = hNEW(hResourcePackage)(engine_, filesystem_, &resourceHandlers_, &fileReadJobQueue_, &workerQueue_, name);
            activePackages_.Insert(pkcrc, pkg);
            pkg->beginLoad();
        } else {
            pkg->AddRef();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourceManager::getIsPackageLoaded( const hChar* name ) {
        hUint32 pkcrc = hCRC32::StringCRC(name);
        hResourcePackage* pkg=activePackages_.Find(pkcrc);
        return (pkg && pkg->isInReadyState()) ? hTrue : hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::unloadPackage(const hChar* name)
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

    void hResourceManager::printResourceInfo()
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

    hResourceClassBase* hResourceManager::getResourceForHandle(hResourceID resid) {
        auto entry=resourceHandleMap_.find(resid);
        if (entry != resourceHandleMap_.end()) {
            return entry->second;
        }
        return hNullptr;
    }

}