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

	hResourceManager* hResourceManager::pInstance_ = NULL;
    void*             hResourceManager::resourceThreadID_ = NULL;


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    hResourceManager::hResourceManager() 
        : exitSignal_(hFalse)
        , gotRequiredResources_(hFalse)
        , remappingNames_(NULL)
        , remappingNamesSize_(0)
        , resourceSweepCount_(hNEW_ALIGN(hGeneralHeap, 32, hUint32))
	{
		hcAssert( pInstance_ == NULL );

		pInstance_ = this;

        loadedResources_.SetAutoDelete(false);
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hResourceManager::~hResourceManager()
	{
        hDELETE_ALIGNED(hGeneralHeap, resourceSweepCount_);
        resourceSweepCount_ = NULL;
		pInstance_ = NULL;
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

        //Load up the Resource Remap Table
        LoadResourceRemapTable();

		loaderSemaphone_.Create( 0, 4096 );

		resourceLoaderThread_.Begin(
			"hResource Loader hThread",
			hThread::PRIORITY_ABOVENORMAL,
			Device::Thread::ThreadFunc::bind< hResourceManager, &hResourceManager::LoadedThreadFunc >( this ), NULL );

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

		return hTrue;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hResourceManager::Shutdown( hRenderer* prenderer )
	{
        while (!requiredResourcesPackage_.IsPackageLoaded()) {hThreading::ThreadSleep(1);}

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
			hThreading::ThreadSleep(1);
			++exitBail;
		}

        //
        for (hResourceClassBase* i = loadedResources_.GetHead(); i; i = i->GetNext())
        {
            hcPrintf("Resource ID [0x%08X] still has reference count > 0 [ref count = %d]", i->GetResourceID(), i->GetRefCount());
        }

		hcAssert( *resourceSweepCount_ == 0 );
		requiredResources_.Clear();
		requiredResourceKeys_.Clear();
        streamingResources_.Clear( hTrue );

        hFreeSafe(remappingNames_);
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::SetResourceHandlers( const hChar* typeExt, ResourceLoadCallback onLoad, ResourceUnloadCallback onUnload, void* pUserData )
    {
        hResourceType type;
        ResourceHandler* handler = hNEW(hGeneralHeap, ResourceHandler);

        strcpy_s( type.ext, 4, typeExt );

        handler->loadCallback	= onLoad;
        handler->unloadCallback	= onUnload;

        resHandlers_.Insert( type, handler );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hResourceManager::LoadedThreadFunc( void* )
    {
        resourceThreadID_ = hThreading::GetCurrentThreadID();
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
               
                hDELETE_ARRAY_SAFE(hGeneralHeap, request.path_);
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

		return 0;
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourceManager::CompleteLoadRequest( hResourceLoadRequest* request )
    {
        hUint32 crc = BuildResourceCRC( request->path_ );
        return LoadResourceFromPath( request->path_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourceManager::EnumerateAndLoadDepFiles( const FileInfo* fileInfo )
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

    hResourceClassBase* hResourceManager::LoadResourceFromPath( const hChar* path )
    {
        hcAssert(hThreading::GetCurrentThreadID() == resourceThreadID_);
        hChar* depPath;
        hcAssert( path );

        hUint32 crc = BuildResourceCRC( path );

        //Check the remappings
        ResourceRemap* remap = remappings_.Find( crc );

        if ( remap )
        {
            path = remappingNames_+remap->mapToPath;
            crc = BuildResourceCRC( path );
        }

        // everything ready, init this resources data add remove from the 
        // list
        hResourceType resType;
        //ext is ALWAYS 4 bytes long ( e.g. '.tex' )
        const hChar* ext = path + (strlen( path ) - 4);
        hcAssert( *ext == '.' );
        strcpy_s( resType.ext, 4, ext+1 );

        ResourceHandler* handler = resHandlers_.Find( resType );
        hcAssertMsg( handler, "Can't find resource handler data for extention %s", ext );

        hResourceClassBase* resource = loadedResources_.Find( crc );

        // The resource isn't loaded, so grab it
        if ( !resource )
        {
            // must load all the dependency folder first
            // if the resource is already loaded, it's safe to assume that the 
            // dependencies are loaded along side it
            hUint32 extLen = strlen( path );
            hcAssertMsg( path[ extLen-4 ] == '.', "resource %s doesn't seem to have a valid extention", path );

            depPath = (hChar*)alloca( extLen + 1 );

            strcpy_s( depPath, extLen+1, path );

            depPath[ extLen - 4 ] = '_';
            depPath[ extLen - 3 ] = 'D';
            depPath[ extLen - 2 ] = 'E';
            depPath[ extLen - 1 ] = 'P';

            filesystem_->EnumerateFiles( depPath, EnumerateFilesCallback::bind< hResourceManager, &hResourceManager::EnumerateAndLoadDepFiles >( this ) );

            //Load the actual resource
            hSerialiserFileStream loaderStream;
            loaderStream.Open( path, hFalse, filesystem_ );
            hcPrintf( "Loading Resource %s (crc32: 0x%08X)", path, crc );
            resource = handler->loadCallback( 
                resType.ext, 
                crc, 
                &loaderStream, 
                this );

            resource->SetType(resType);

            if ( resource->GetFlags() & hResourceClassBase::ResourceFlags_STREAMING )
            {
                hStreamingResourceBase* stres = static_cast< hStreamingResourceBase* >( resource );
                StreamingResouce* sr = hNEW(hGeneralHeap, StreamingResouce);
                sr->stream_ = stres;

                stres->SetFileStream( loaderStream );
                streamingResources_.Insert( crc, sr );
            }
            else
            { 
                loaderStream.Close();
            }

            hcAssert( resource );
            resource->SetResID( crc );
            resource->IsDiskResource( hTrue );
            resource->manager_ = this;

            // Push the new resource into loaded resource map
            loadedResources_.Insert( crc, resource );

            MainThreadResourceHandle loaded;
            loaded.resource_ = resource;
            loaded.resourceKey_ = resource->GetResourceID();

            // In queue to be pushed back
            completedLoads_.push(loaded);
        }

        // increment the resource count
        resource->AddRef();

        return resource;
    }

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hBool hResourceManager::RequiredResourcesReady()
	{
        if ( !gotRequiredResources_ && requiredResourcesPackage_.IsPackageLoaded() )
        {
            gotRequiredResources_ = hTrue;
        }
		return gotRequiredResources_;
	}
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::LoadResourceRemapTable()
    {
        /* 
         * The remap table follows this format.
         * (4 bytes) CRC of valid resource, (4 bytes) CRC of remapped resource (xN), (4 bytes) 0 null terminator
         * (Repeat)...
         */
        hIFile* rrt = filesystem_->OpenFile( "RRT", FILEMODE_READ );
        remappingNames_ = 0;
        remappingNamesSize_ = 0;
        hUint32 remapCRC = 0;
        hChar* currentPath = NULL;
        hUint32 pathOffset = 0;

        if (rrt->Length() == 0)
            return;

        for ( hUint64 i = rrt->Length(); i <= rrt->Length(); )
        {
            if ( !currentPath )
            {
                hUint32 len;
                rrt->Read( &len, sizeof(len) );
                remappingNames_ = (hChar*)hRealloc( remappingNames_, remappingNamesSize_+len+1 );
                rrt->Read( remappingNames_+remappingNamesSize_, len );
                remappingNames_[remappingNamesSize_+len] = 0;
                currentPath = remappingNames_+remappingNamesSize_;
                pathOffset = remappingNamesSize_;
                remappingNamesSize_ += len+1;

                i -= (sizeof(len)+len);
            }
            else
            {
                rrt->Read( &remapCRC, sizeof(remapCRC) );
                i -= sizeof(remapCRC);

                if ( remapCRC )
                {
                    ResourceRemap* remap = hNEW(hGeneralHeap, ResourceRemap);
                    remap->mapToPath = pathOffset;

                    remappings_.Insert( remapCRC, remap );
                }
                else
                {
                    currentPath = NULL;
                }
            }
        }

        filesystem_->CloseFile( rrt );

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hResourceManager::GetResourceKeyRemapping( hUint32 key ) const
    {
        ResourceRemap* remap = remappings_.Find( key );
        if ( remap )
        {
            return BuildResourceCRC( remappingNames_+remap->mapToPath );
        }
        return key;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::MainThreadUpdate()
    {
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
            MainThreadResourceHandle* loadedResource = hNEW(hGeneralHeap, MainThreadResourceHandle);
            *loadedResource = ivResourceLoadedQueue_.pop();
            mtResourceMap_.Insert(loadedResource->resourceKey_, loadedResource);
        }

        //Push Unloaded Resources
        hAtomic::LWMemoryBarrier();
        if (*resourceSweepCount_ > 0)
        {
            for (MainThreadResourceHandle* i = mtResourceMap_.GetHead(); i; )
            {
                if (i->resource_->GetRefCount() == 0)
                {
                    MainThreadResourceHandle* rem = i;
                    mtResourceMap_.Erase(i,&i);
                    ivResourceUnloadedQueue_.push(*rem);
                    hDELETE(hGeneralHeap, rem);
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
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::mtLoadResource( const hChar* path )
    {
        hUint32 rescrc32 = BuildResourceCRC( path );
        hUint32 len = hStrLen(path)+1;
        hResourceLoadRequest request;
        request.crc32_ = rescrc32;
        request.loadCounter_ = NULL;
        request.path_ = hNEW_ARRAY(hGeneralHeap, hChar, len);
        hStrCopy(request.path_, len, path);
        mtLoadRequests_.push(request);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourceManager::mtGetResourceAddRef( hUint32 crc )
    {
        hUint32 rescrc32 = GetResourceKeyRemapping(crc);
        MainThreadResourceHandle* resource = mtResourceMap_.Find(rescrc32);
        if (resource)
        {
            resource->resource_->AddRef();
            return resource->resource_;
        }
        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourceManager::mtGetResourceAddRef( const hChar* path )
    {
        return mtGetResourceAddRef(BuildResourceCRC(path));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourceManager::mtGetResourceWeak( hUint32 crc )
    {
        hUint32 rescrc32 = GetResourceKeyRemapping(crc);
        MainThreadResourceHandle* resource = mtResourceMap_.Find(rescrc32);
        if (resource)
        {
            return resource->resource_;
        }
        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourceManager::mtGetResourceWeak( const hChar* path )
    {
        return mtGetResourceWeak(BuildResourceCRC(path));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourceManager::ltGetResourceAddRef(hUint32 crc)
    {
        hcAssert(hThreading::GetCurrentThreadID() == resourceThreadID_);
        hResourceClassBase* res = loadedResources_.Find(GetResourceKeyRemapping(crc));
        res->AddRef();
        return res;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourceManager::ltGetResourceWeak(hUint32 crc)
    {
        hcAssert(hThreading::GetCurrentThreadID() == resourceThreadID_);
        return loadedResources_.Find(GetResourceKeyRemapping(crc));
    }

}