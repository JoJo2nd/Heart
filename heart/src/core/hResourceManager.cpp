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
    hResourceManager* hResourceManager::instance_ = nullptr;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceManager::hResourceManager() 
    {
        // Do this as soon as possible, them resource handles can be created globally
        hResourceHandle initResourceStatics(this, hStringID());
        (void)initResourceStatics;
        instance_ = this;
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

    hBool hResourceManager::initialise(hIFileSystem* pFileSystem, hJobManager* jobmanager) {
        filesystem_ = pFileSystem;
        jobManager_ = jobmanager;
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
            pkg = hNEW(hResourcePackage)();
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

}