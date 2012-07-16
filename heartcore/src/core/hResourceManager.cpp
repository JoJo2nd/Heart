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
        , gotRequiredResources_(hFalse)
        , remappingNames_(NULL)
        , remappingNamesSize_(0)
	{
        loadedResources_.SetAutoDelete(false);
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

	hBool hResourceManager::Initialise( hRenderer* renderer, hIFileSystem* pFileSystem, const char** requiredResources )//< NEEDS FileSystem??
	{
		hUint32 nRequiredResources = 0;
		filesystem_ = pFileSystem;
        renderer_ = renderer;
        materialManager_ = renderer->GetMaterialManager();
#if 0
        //Load up the Resource Remap Table
        LoadResourceRemapTable();

		loaderSemaphone_.Create( 0, 4096 );

		resourceLoaderThread_.Create(
			"hResource Loader hThread",
			hThread::PRIORITY_ABOVENORMAL,
			hThread::ThreadFunc::bind< hResourceManager, &hResourceManager::LoadedThreadFunc >( this ), NULL );

		for ( const char** c = requiredResources; *c; ++c )
        {    
            ++nRequiredResources; 
        }
		requiredResourceKeys_.Resize( nRequiredResources );
		requiredResources_.Resize( nRequiredResources );

        for ( hUint32 i = 0; i < nRequiredResources; ++i )
        {    
            requiredResourceKeys_[i] = requiredResources[i];
            requiredResourcesPackage_.AddResourceToPackage(requiredResources[i], this);
        }
#endif

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
        resourceThreadID_ = Device::GetCurrentThreadID();
#if 0
        //process the requests
        while( !exitSignal_ || loadedResources_.GetSize() > 0 )
        {
            loaderSemaphone_.Wait();
            {
                hMutexAutoScope autolock(&ivAccessMutex_);

                while (!ivLoadRequests_.isEmpty())
                {
                    loadRequests_.push(ivLoadRequests_.pop());
                }

                while (!ivResourceUnloadedQueue_.isEmpty())
                {
                    unloadRequests_.push(ivResourceUnloadedQueue_.pop());
                }
            }

            //Process and file read requests
            for ( StreamingResouce* i = streamingResources_.GetHead(); i; i = i->GetNext() )
            {
                i->stream_->UpdateFileOps();
            }

            // Process the load requests
            while (!loadRequests_.isEmpty())
            {
                hResourceLoadRequest request = loadRequests_.pop();
                CompleteLoadRequest(&request);
               
                hDELETE_ARRAY_SAFE(GetGlobalHeap(), request.path_);
            }

            while (!unloadRequests_.isEmpty())
            {
                MainThreadResourceHandle unload = unloadRequests_.pop();
                hResourceClassBase* del = unload.resource_;
                ResourceHandler* handler = resHandlers_.Find(del->GetType());

                loadedResources_.Remove(del);
                hcAssert(del->GetRefCount() == 0);
                hcAssert(handler->GetKey() == del->GetType());
                hcPrintf("Unloaded resource 0x%08X", del->GetResourceID());
                handler->unloadCallback(del->GetType().ext, del, this);
            }

            {
                hMutexAutoScope autolock(&ivAccessMutex_);

                while (!completedLoads_.isEmpty())
                {
                    ivResourceLoadedQueue_.push(completedLoads_.pop());
                }
            }
		}
#endif

		return 0;
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourceManager::EnumerateAndLoadDepFiles( const hFileInfo* fileInfo )
    {
        if ( fileInfo->directory_ )
            return hTrue;

        //Avoid anything that can't have a .XXX extension.
        if ( hStrLen( fileInfo->path_ ) < 3 )
            return hTrue;

        hUint32 fullpathlen = strlen( fileInfo->path_ )+strlen( fileInfo->name_ )+2;
        hChar* fullPath = (hChar*)hAlloca( fullpathlen );
        hStrCopy( fullPath, fullpathlen, fileInfo->path_ );
        hStrCat( fullPath, fullpathlen, "/" );
        hStrCat( fullPath, fullpathlen, fileInfo->name_ );
        LoadResourceFromPath( fullPath );

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::MainThreadUpdate()
    {
#if 0
        hMutexAutoScope autoLock(&ivAccessMutex_);
        hBool pushedSomething = hFalse;

        //Push reqeusts
        while (!mtLoadRequests_.isEmpty())
        {
            hResourceLoadRequest req = mtLoadRequests_.pop();
            ivLoadRequests_.push(req);
            pushedSomething = hTrue;
        }

        //Get Loaded Resources
        while (!ivResourceLoadedQueue_.isEmpty())
        {
            MainThreadResourceHandle* loadedResource = hNEW(GetGlobalHeap(), MainThreadResourceHandle);
            *loadedResource = ivResourceLoadedQueue_.pop();
            mtResourceMap_.Insert(loadedResource->resourceKey_, loadedResource);
        }

        //Push Unloaded Resources
        hAtomic::LWMemoryBarrier();
        if (resourceSweepCount_.value_ > 0)
        {
            for (MainThreadResourceHandle* i = mtResourceMap_.GetHead(); i; )
            {
                if (i->resource_->GetRefCount() == 0)
                {
                    MainThreadResourceHandle* rem = i;
                    mtResourceMap_.Erase(i,&i);
                    ivResourceUnloadedQueue_.push(*rem);
                    hDELETE(GetGlobalHeap(), rem);
                    pushedSomething = hTrue;
                    hAtomic::Decrement(resourceSweepCount_);
                }
                else
                {
                    i = i->GetNext();
                }
            }

            
        }

        if (pushedSomething)
        {
            loaderSemaphone_.Post();
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::LoadGamedataDesc()
    {
        hIFile* f = filesystem_->OpenFile("gamedatadesc", FILEMODE_READ);
        void* xmldata = hHeapMalloc(GetGlobalHeap(), f->Length());
        f->Read(xmldata, f->Length());
        gamedataDescXML_.ParseSafe< rapidxml::parse_default >((hChar*)xmldata,GetGlobalHeap());

        for (hXMLGetter e = hXMLGetter(&gamedataDescXML_).FirstChild("gamedata").FirstChild("loaders"); e.ToNode() != NULL; e = e.NextSibling())
        {
            hResourceHandler handler;
            hStrCopy(handler.type_.ext, 4, e.GetAttributeString("ext") );
            handler.libPath_ = e.GetAttributeString("libpath");

            handler.loaderLib_ = OpenSharedLib(handler.libPath_);
            if (handler.loaderLib_ != HEART_SHAREDLIB_INVALIDADDRESS)
            {
                handler.binLoader_              = (OnResourceDataLoad)      GetFunctionAddress(handler.loaderLib_,"HeartBinLoader");
                handler.rawLoader_              = (OnResourceDataLoadRaw)   GetFunctionAddress(handler.loaderLib_,"HeartRawLoader");
                handler.packageLinkComplete_    = (OnPackageLoadComplete)   GetFunctionAddress(handler.loaderLib_,"HeartPackageLink");
                handler.resourceDataUnload_     = (OnResourceDataUnload)    GetFunctionAddress(handler.loaderLib_,"HeartDataUnload");
                handler.packageUnload_          = (OnPackageUnloadComplete) GetFunctionAddress(handler.loaderLib_,"HeartPackageUnload");

                if ( handler.binLoader_           &&
                     handler.rawLoader_           &&   
                     handler.packageLinkComplete_ &&   
                     handler.resourceDataUnload_  &&   
                     handler.packageUnload_       )
                {
                    //Add
                    hResourceHandler* toadd = hNEW(GetGlobalHeap(), hResourceHandler)();
                    *toadd = handler;
                    resourceHandlers_.Insert(handler.type_, toadd);
                }
            }
        }
    }

}