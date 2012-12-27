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

    void*             hResourceManager::resourceThreadID_ = NULL;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceManager::hResourceManager() 
        : requireAssetsReady_(hFalse)
    {
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

    hUint32 hResourceManager::BuildResourceCRC( const hChar* resourceName )
    {
        return hCRC32::FullCRC( resourceName, strlen( resourceName ) - 4 );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourceManager::Initialise( hHeartEngine* engine, hRenderer* renderer, hIFileSystem* pFileSystem, const char** requiredResources )//< NEEDS FileSystem??
    {
        hUint32 nRequiredResources = 0;
        filesystem_ = pFileSystem;
        renderer_ = renderer;
        materialManager_ = renderer->GetMaterialManager();
        engine_ = engine;

        LoadGamedataDesc();
        loaderSemaphone_.Create(0, 128);
        resourceLoaderThread_.create(
            "hResource Loader hThread",
            hThread::PRIORITY_NORMAL,
            hFUNCTOR_BINDMEMBER(hThreadFunc, hResourceManager, LoadedThreadFunc, this), NULL );

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::Shutdown( hRenderer* prenderer )
    {
        //remove references to loaded packages from the main thread
        mtLoadedPackages_.Clear(hTrue);
        // Signals the loader thread to finish up
        exitSignal_.Signal();
        loaderSemaphone_.Post();
        // The loader thread will unload everything & clean up
        // Wait for it to complete
        resourceLoaderThread_.join();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hResourceManager::LoadedThreadFunc( void* )
    {
        hBool stuffToDo = hFalse;
        resourceThreadID_ = Device::GetCurrentThreadID();

        while(!exitSignal_.TryWait())
        {
            Device::ThreadSleep(16);
            loaderSemaphone_.Wait();

            stuffToDo = hTrue;

            while (stuffToDo)
            {
                //Scope for mutex
                {
                    hMutexAutoScope autoLock(&ltAccessMutex_);
                    while (!ltLoadRequests_.isEmpty())
                    {
                        ResourcePackageLoadMsg loadmsg = ltLoadRequests_.pop();
                        hUint32 key = hCRC32::StringCRC(loadmsg.path_);
                        hResourcePackage* toload = ltLoadedPackages_.Find(key);
                        if (toload == NULL) {
                            toload = hNEW(GetGlobalHeap(), hResourcePackage)(engine_, filesystem_, &resourceHandlers_);

                            toload->LoadPackageDescription(loadmsg.path_);
                            toload->AddRef();

                            ltLoadedPackages_.Insert(key, toload);
                        }
                        else {
                            toload->AddRef();
                            if (toload->isUnloading()) {
                                toload->prepareReload();
                            }
                        }
                    }
        
                    while (!ltUnloadRequest_.isEmpty())
                    {
                        ResourcePackageQueueMsg unload = ltUnloadRequest_.pop();
                        unload.package_->DecRef();
                    }
                }

                stuffToDo = hFalse;

                for (hResourcePackage* pack = ltLoadedPackages_.GetHead(); pack; pack = pack->GetNext())
                {
                    if (pack->GetRefCount() == 0 && !pack->isUnloading()) {
                        //Package needed unloading, push across to main thread
                        hMutexAutoScope autoLock(&ltAccessMutex_);
                        ResourcePackageQueueMsg msg;
                        msg.packageCRC_ = pack->GetPackageCRC();
                        msg.package_ = NULL;
                        ltPackageLoadCompleteQueue_.push(msg);
                        pack->Unload(); 
                    }
                    if (pack->Update())
                    {
                        //Package has just finised a load, push across to main thread
                        hMutexAutoScope autoLock(&ltAccessMutex_);
                        ResourcePackageQueueMsg msg;
                        msg.packageCRC_ = pack->GetPackageCRC();
                        msg.package_ = pack;
                        ltPackageLoadCompleteQueue_.push(msg);
                    }
                    stuffToDo |= !pack->IsInPassiveState();
                }

                for (hResourcePackage* pack = ltLoadedPackages_.GetHead(), *nextpack = NULL; pack;)
                {
                    nextpack = pack->GetNext();
                    if (pack->ToUnload())
                    {
                        hMutexAutoScope autoLock(&ltAccessMutex_);
                        if (ltPackageLoadCompleteQueue_.isEmpty()) {
                            ltLoadedPackages_.Remove(pack);
                            hDELETE(GetGlobalHeap(), pack);
                        }
                    }

                    pack = nextpack;
                }

                //Probably waiting on a read so sleep a bit
                Device::ThreadSleep(16);
            }
        }

        /*
        * Bailing, so unload all packages. Done by removing the 
        * packages with the smallest ref count first so dependant packages 
        * are left until last
        */
        while (ltLoadedPackages_.GetSize() > 0) {
            hResourcePackage* packToUnload;
            for (hResourcePackage* pack = ltLoadedPackages_.GetHead(); pack; pack = pack->GetNext()) {
                pack->DecRef();
                if (pack->GetRefCount() == 0) {
                    packToUnload = pack;
                    break;
                }
            }
            packToUnload->Unload();
            while (!packToUnload->ToUnload()) {
                packToUnload->Update();
            }

            ltLoadedPackages_.Remove(packToUnload);
            hDELETE(GetGlobalHeap(), packToUnload);
        }

        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::MainThreadUpdate()
    {
        HEART_PROFILE_FUNC();
        hMutexAutoScope autoLock(&ltAccessMutex_);

        //Push reqeusts
        while (!mtLoadRequests_.isEmpty())
        {
            ResourcePackageLoadMsg req = mtLoadRequests_.pop();
            ltLoadRequests_.push(req);
            loaderSemaphone_.Post();
        }

        //Get Loaded Resources
        while (!ltPackageLoadCompleteQueue_.isEmpty())
        {
            ResourcePackageQueueMsg loadedPak = ltPackageLoadCompleteQueue_.pop();
            hLoadedResourcePackages* pak = mtLoadedPackages_.Find(loadedPak.packageCRC_);
            if (!pak) {
                pak = hNEW(GetGlobalHeap(), hLoadedResourcePackages);
                mtLoadedPackages_.Insert(loadedPak.packageCRC_, pak);
            }
            pak->package_ = loadedPak.package_;
        }

        //Push Unloaded Resources
        while (!mtUnloadRequest_.isEmpty())
        {
            ltUnloadRequest_.push(mtUnloadRequest_.pop());
            loaderSemaphone_.Post();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::LoadGamedataDesc()
    {
        hIFile* f = filesystem_->OpenFile("loaders", FILEMODE_READ);
        void* xmldata = hHeapMalloc(GetGlobalHeap(), (hUint32)(f->Length()+1));
        f->Read(xmldata, (hUint32)f->Length());
        ((hChar*)xmldata)[f->Length()] = 0;
        filesystem_->CloseFile(f);
        gamedataDescXML_.ParseSafe< rapidxml::parse_default >((hChar*)xmldata,GetGlobalHeap());

        for (hXMLGetter e = hXMLGetter(&gamedataDescXML_).FirstChild("gamedata").FirstChild("loader"); e.ToNode() != NULL; e = e.NextSibling())
        {
            hResourceHandler handler;
            hStrCopy(handler.type_.ext, 4, e.GetAttributeString("type") );
            hUint32 fullPathSize = hStrLen(e.GetAttributeString("libpath"))+hStrLen(HEART_SUFFIX)+hStrLen(HEART_SHARED_LIB_EXT);
            hChar* fullLibPath = (hChar*)hAlloca(fullPathSize+1);
            hStrCopy(fullLibPath, fullPathSize, e.GetAttributeString("libpath"));
            hStrCat(fullLibPath, fullPathSize, HEART_SUFFIX);
            hStrCat(fullLibPath, fullPathSize, HEART_SHARED_LIB_EXT);

            handler.loaderLib_ = hd_OpenSharedLib(fullLibPath);
            hcAssertMsg(handler.loaderLib_ != HEART_SHAREDLIB_INVALIDADDRESS, 
                "Failed to load %s.This will cause data to fail to load", fullLibPath);
            if (handler.loaderLib_ != HEART_SHAREDLIB_INVALIDADDRESS)
            {
                handler.binLoader_              = (OnResourceDataLoad)      hd_GetFunctionAddress(handler.loaderLib_,"HeartBinLoader");
                handler.rawCompiler_            = (OnResourceDataCompile)   hd_GetFunctionAddress(handler.loaderLib_,"HeartDataCompiler");
                handler.packageLink_            = (OnPackageLoadComplete)   hd_GetFunctionAddress(handler.loaderLib_,"HeartPackageLink");
                handler.packageUnlink_          = (OnResourceDataUnload)    hd_GetFunctionAddress(handler.loaderLib_,"HeartPackageUnlink");
                handler.resourceDataUnload_     = (OnPackageUnloadComplete) hd_GetFunctionAddress(handler.loaderLib_,"HeartPackageUnload");

                if ( handler.binLoader_           &&
                     handler.packageLink_         &&
                     handler.resourceDataUnload_  &&
                     handler.packageUnlink_       )
                {
                    //Add
                    hResourceHandler* toadd = hNEW(GetGlobalHeap(), hResourceHandler)();
                    *toadd = handler;
                    resourceHandlers_.Insert(handler.type_, toadd);
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::mtLoadPackage( const hChar* name )
    {
        hUint32 pkcrc = hCRC32::StringCRC(name);
        //Add to be loaded
        ResourcePackageLoadMsg msg;
        hStrCopy(msg.path_, HEART_RESOURCE_PATH_SIZE, name);
        mtLoadRequests_.push(msg);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourceManager::mtIsPackageLoaded( const hChar* name )
    {
        hLoadedResourcePackages* pk = mtLoadedPackages_.Find(hCRC32::StringCRC(name));
        return (pk && pk->package_) ? hTrue : hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::mtUnloadPackage(const hChar* name)
    {
        hUint32 pkcrc = hCRC32::StringCRC(name);
        hLoadedResourcePackages* pk = mtLoadedPackages_.Find(pkcrc);
        if (pk)
        {
            //Add to be unloaded
            // Should just the id be sent
            ResourcePackageQueueMsg msg;
            msg.packageCRC_ = pkcrc;
            msg.package_ = pk->package_;
            mtUnloadRequest_.push(msg);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    Heart::hResourceID hResourceManager::BuildResourceID( const hChar* fullPath )
    {
        if (!fullPath)
            return 0;

        const hChar* resName = hStrChr(fullPath, '.');
        if (!resName)
            return 0;
        
        hUint32 pakCRC = hCRC32::FullCRC(fullPath, (hUint32)resName-(hUint32)fullPath);
        hUint32 resCRC = hCRC32::StringCRC(resName+1);

        return ((hUint64)pakCRC << 32) | ((hUint64)resCRC);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    Heart::hResourceID hResourceManager::BuildResourceID( const hChar* package, const hChar* resourceName )
    {
        if (!package || !resourceName)
            return 0;

        hUint32 pakCRC = hCRC32::StringCRC(package);
        hUint32 resCRC = hCRC32::StringCRC(resourceName);

        return ((hUint64)pakCRC << 32) | ((hUint64)resCRC);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourceManager::ltGetResource( hResourceID resid )
    {
        hUint32 pakCRC = (hUint32)((resid&0xFFFFFFFF00000000)>>32);
        hUint32 resCRC = (hUint32)((resid&0x00000000FFFFFFFF));

        hResourcePackage* pak = ltLoadedPackages_.Find(pakCRC);
        if (pak)
        {
            return pak->GetResource(resCRC);
        }
        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourceManager::RequiredResourcesReady()
    {
        if (requireAssetsReady_) return hTrue;
        
        if (mtIsPackageLoaded("CORE"))
        {
            requireAssetsReady_ = hTrue;
        }

        return requireAssetsReady_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourceManager::mtGetResource( const hChar* path )
    {
        return mtGetResource(BuildResourceID(path));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourceManager::mtGetResource( hResourceID resid )
    {
        hUint32 pakCRC = (hUint32)((resid&0xFFFFFFFF00000000)>>32);
        hUint32 resCRC = (hUint32)((resid&0x00000000FFFFFFFF));

        hLoadedResourcePackages* pak = mtLoadedPackages_.Find(pakCRC);
        if (pak && pak->package_)
        {
            return pak->package_->GetResource(resCRC);
        }
        return NULL;
    }

}