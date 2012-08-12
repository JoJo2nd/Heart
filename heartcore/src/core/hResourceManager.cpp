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
        : exitSignal_(hFalse)
        , requireAssetsReady_(hFalse)
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

    hBool hResourceManager::Initialise( HeartEngine* engine, hRenderer* renderer, hIFileSystem* pFileSystem, const char** requiredResources )//< NEEDS FileSystem??
	{
		hUint32 nRequiredResources = 0;
		filesystem_ = pFileSystem;
        renderer_ = renderer;
        materialManager_ = renderer->GetMaterialManager();
        engine_ = engine;

        LoadGamedataDesc();

        resourceLoaderThread_.Create(
            "hResource Loader hThread",
            hThread::PRIORITY_NORMAL,
            hThread::ThreadFunc::bind< hResourceManager, &hResourceManager::LoadedThreadFunc >( this ), NULL );

		return hTrue;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hResourceManager::Shutdown( hRenderer* prenderer )
	{
#if 0
        while (!requiredResourcesPackage_.IsPackageLoaded()) {Device::ThreadSleep(1);}

		for ( hUint32 i = 0, c = requiredResources_.GetSize(); i < c; ++i )
		{
            hResourceClassBase* res = requiredResourcesPackage_.GetResource(i);
		    if (res)
            {
                res->DecRef();
            }
		}

		exitSignal_ = hTrue;
		loaderSemaphone_.Post();

		hUint32 exitBail = 0;

		while ( !resourceLoaderThread_.IsComplete() && exitBail < 1000 ) 
		{
			prenderer->ReleasePendingRenderResources();
			MainThreadUpdate();
			Device::ThreadSleep(1);
			++exitBail;
		}

        //
        for (hResourceClassBase* i = loadedResources_.GetHead(); i; i = i->GetNext())
        {
            hcPrintf("Resource ID [0x%08X] still has reference count > 0 [ref count = %d]", i->GetResourceID(), i->GetRefCount());
        }

		requiredResources_.Clear();
		requiredResourceKeys_.Clear();
        streamingResources_.Clear( hTrue );

        hFreeSafe(remappingNames_);
#endif
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hResourceManager::LoadedThreadFunc( void* )
    {
        hBool stuffToDo = hFalse;
        resourceThreadID_ = Device::GetCurrentThreadID();

        while( !exitSignal_ )
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
                        hResourcePackageV2* toload = hNEW(GetGlobalHeap(), hResourcePackageV2)(engine_, filesystem_, &resourceHandlers_);

                        toload->LoadPackageDescription(loadmsg.path_);

                        ltLoadedPackages_.Insert(hCRC32::StringCRC(loadmsg.path_), toload);
                    }
        
                    //TODO: process unload requests
                    while (!ltUnloadRequest_.isEmpty())
                    {
                        ResourcePackageQueueMsg unload = ltUnloadRequest_.pop();
                        unload.package_->Unload();
                    }
                }

                for (hResourcePackageV2* pack = ltLoadedPackages_.GetHead(); pack; pack = pack->GetNext())
                {
                    if (pack->Update())
                    {
                        //Package has just finised a load...
                        hMutexAutoScope autoLock(&ltAccessMutex_);
                        ResourcePackageQueueMsg msg;
                        msg.package_ = pack;
                        ltPackageLoadCompleteQueue_.push(msg);
                    }
                    stuffToDo &= pack->IsInPassiveState();
                }

                for (hResourcePackageV2* pack = ltLoadedPackages_.GetHead(), *nextpack = NULL; pack;)
                {
                    nextpack = pack->GetNext();
                    if (pack->ToUnload())
                    {
                        ltLoadedPackages_.Remove(pack);
                        hDELETE(GetGlobalHeap(), pack);
                    }

                    pack = nextpack;
                }

                //Probably waiting on a read so sleep a bit
                Device::ThreadSleep(16);
            }
        }

		return 0;
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::MainThreadUpdate()
    {
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
            hLoadedResourcePackages* pak = mtLoadedPackages_.Find(loadedPak.package_->GetPackageCRC());
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
        void* xmldata = hHeapMalloc(GetGlobalHeap(), f->Length()+1);
        f->Read(xmldata, f->Length());
        ((hChar*)xmldata)[f->Length()] = 0;
        filesystem_->CloseFile(f);
        gamedataDescXML_.ParseSafe< rapidxml::parse_default >((hChar*)xmldata,GetGlobalHeap());

        for (hXMLGetter e = hXMLGetter(&gamedataDescXML_).FirstChild("gamedata").FirstChild("loader"); e.ToNode() != NULL; e = e.NextSibling())
        {
            hResourceHandler handler;
            hStrCopy(handler.type_.ext, 4, e.GetAttributeString("type") );
            handler.libPath_ = e.GetAttributeString("libpath");

            handler.loaderLib_ = hd_OpenSharedLib(handler.libPath_);
            if (handler.loaderLib_ != HEART_SHAREDLIB_INVALIDADDRESS)
            {
                handler.binLoader_              = (OnResourceDataLoad)      hd_GetFunctionAddress(handler.loaderLib_,"HeartBinLoader");
                handler.rawLoader_              = (OnResourceDataLoadRaw)   hd_GetFunctionAddress(handler.loaderLib_,"HeartRawLoader");
                handler.packageLink_    = (OnPackageLoadComplete)   hd_GetFunctionAddress(handler.loaderLib_,"HeartPackageLink");
                handler.resourceDataUnload_     = (OnResourceDataUnload)    hd_GetFunctionAddress(handler.loaderLib_,"HeartPackageUnlink");
                handler.packageUnlink_          = (OnPackageUnloadComplete) hd_GetFunctionAddress(handler.loaderLib_,"HeartPackageUnload");

                if ( handler.binLoader_           &&
                     handler.rawLoader_           &&   
                     handler.packageLink_ &&   
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
        hLoadedResourcePackages* pk = mtLoadedPackages_.Find(pkcrc);
        if (pk)
        {
            pk->AddRef();
        }
        else
        {
            pk = hNEW(GetGlobalHeap(), hLoadedResourcePackages);
            hStrCopy(pk->path_, HEART_RESOURCE_PATH_SIZE, name);
            pk->AddRef();
            mtLoadedPackages_.Insert(pkcrc, pk);

            //Add to be loaded
            ResourcePackageLoadMsg msg;
            hStrCopy(msg.path_, HEART_RESOURCE_PATH_SIZE, name);
            mtLoadRequests_.push(msg);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourceManager::mtIsPackageLoaded( const hChar* name )
    {
        hLoadedResourcePackages* pk = mtLoadedPackages_.Find(hCRC32::StringCRC(name));
        return pk == NULL ? hFalse : hTrue;
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
            pk->DecRef();
            if (pk->GetRefCount() == 0)
            {
                mtLoadedPackages_.Remove(pk);
               
                //Add to be unloaded
                ResourcePackageQueueMsg msg;
                msg.package_ = pk->package_;
                mtUnloadRequest_.push(msg);

                //Clean up
                hDELETE(GetGlobalHeap(), pk);
            }
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

        hResourcePackageV2* pak = ltLoadedPackages_.Find(pakCRC);
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
        
        hLoadedResourcePackages* corepak = mtLoadedPackages_.Find(hCRC32::StringCRC("CORE"));
        if (corepak != NULL)
        {
            requireAssetsReady_ = corepak->package_ != NULL;
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