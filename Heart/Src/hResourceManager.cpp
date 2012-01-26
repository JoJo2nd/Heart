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

#include "Common.h"
#include "hResourceManager.h"
#include "hTexture.h"
#include "hCRC32.h"
#include "hRenderer.h"
#include "hIReferenceCounted.h"
#include <stdio.h>
#include <algorithm>
#include "hIFile.h"

namespace Heart
{

	hResourceManager* hResourceManager::pInstance_ = NULL;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hResourceManager::hResourceManager() 
		: exitSignal_( hFalse )
        , resourceDatabaseLocked_( hFalse )
        , gotRequiredResources_( hFalse )
        , remappingNames_(NULL)
        , remappingNamesSize_(0)
	{
		hcAssert( pInstance_ == NULL );

		pInstance_ = this;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hResourceManager::~hResourceManager()
	{
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
			Device::Thread::ThreadFunc::bind< hResourceManager, &hResourceManager::ProcessDataFixup >( this ), NULL );

		for ( const char** c = requiredResources; *c; ++c )
        {    
            ++nRequiredResources; 
        }
		requiredResourceKeys_.Resize( nRequiredResources );
		requiredResources_.Resize( nRequiredResources );

        for ( hUint32 i = 0; i < nRequiredResources; ++i )
        {    
            requiredResourcesPackage_.AddResourceToPackage( requiredResources[i], requiredResources_[i] );
        }

		//Begin loading the required resources
		requiredResourcesPackage_.BeginPackageLoad( this );

		return hTrue;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hResourceManager::Shutdown( hRenderer* prenderer )
	{
		for ( hUint32 i = 0, c = requiredResources_.GetSize(); i < c; ++i )
		{
		
		}

		ForceResourceSweep();

		exitSignal_ = hTrue;
		loaderSemaphone_.Post();

		hUint32 exitBail = 0;

		while ( !resourceLoaderThread_.IsComplete() && exitBail < 2000 ) 
		{
			prenderer->ReleasePendingRenderResources();
			ForceResourceSweep();
			Threading::ThreadSleep(1);
			++exitBail;
		}

#ifdef HEART_DEBUG
// 		if ( loadedResources_.size() > 0 )
// 		{
// 			hcPrintf( "Unreleased Resources..." );
// 			for ( ResourceMap::iterator i = loadedResources_.begin(); i != loadedResources_.end(); ++i )
// 			{
// 				hcPrintf( "hResource \"%s\"(ext: %s): 0x%08X", i->second.name_, i->second.ext_, i->second.pOutputData_ );
// 			}
// 		}
// 		if ( createdResources_.size() > 0 )
// 		{
// 			hcPrintf( "Unreleased Runtime Resources..." );
// 			for ( ResourceList::iterator i = createdResources_.begin(); i != createdResources_.end(); ++i )
// 			{
// 				hcPrintf( "hResource \"%s\"(ext: %s): 0x%08X", i->name_, i->ext_, i->pOutputData_ );
// 			}
// 		}
#endif

		hcAssert( resourceSweepCount_ == 0 );
		requiredResources_.Clear();
		requiredResourceKeys_.Clear();
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::SetResourceHandlers( const hChar* typeExt, ResourceLoadCallback onLoad, ResourceUnloadCallback onUnload, void* pUserData )
    {
        ResourceType type;
        ResourceHandler* handler = hNEW ( hGeneralHeap ) ResourceHandler();

        strcpy_s( type.ext, 4, typeExt );

        handler->loadCallback	= onLoad;
        handler->unloadCallback	= onUnload;

        resHandlers_.Insert( type, handler );
    }

    //////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hResourceManager::BeginResourceLoads( const hChar** resourceKeys, hUint32 count, hUint32* loadCounter )
	{
		hcAssert( (resourceKeys && count) || (!resourceKeys && !count) );

		resourceDatabaseMutex_.Lock();
		for ( hUint32 i = 0; i < count; ++i )
		{
			LoadResource( resourceKeys[i], loadCounter );
		}
		resourceDatabaseMutex_.Unlock();

		loaderSemaphone_.Post();
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourceManager::ResourceLoadsComplete( const hUint32* resourceKeys, hUint32 count )
    {
        return hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::GetResources( const hChar** resourceKeys, hResourceClassBase** outPtrs, hUint32 count )
    {
        resourceDatabaseMutex_.Lock();

        for ( hUint32 i = 0; i < count; ++i )
        {
            outPtrs[i] = loadedResources_.Find( BuildResourceCRC( resourceKeys[i] ) );
        }

        resourceDatabaseMutex_.Unlock();
    }

    //////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hResourceManager::ProcessDataFixup( void* )
	{
		//process the requests
		while( !exitSignal_ /*|| !canQuit_*/ )
		{
			loaderSemaphone_.Wait();

			if ( exitSignal_ )
			{
				hAtomic::Increment( &resourceSweepCount_ );
			}

			while ( resourceSweepCount_ > 0 )
			{
				DoResourceSweep();
				hAtomic::Decrement( &resourceSweepCount_ );
			}

			// get the all pushed requests and move them in to the processed load request
			// along with dependencies. 
			for ( hResourceLoadRequest* i = loadRequests_.GetHead(); i; i = i->GetNext() )
			{
                CompleteLoadRequest( i );
                hAtomic::Increment( i->loadCounter_ );
                delete i->path_;
			}

            loadRequests_.Clear( hTrue );

// 			if ( loadRequestsProcessed_.GetSize() > 0 )
// 			{
// 				//signal the thread to continue processing
// 				//but yield some cpu time
// 				loaderSemaphone_.Post();
// 				Threading::ThreadSleep( 1 );
// 			}
		}

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hResourceManager::LoadResource( const hChar* resourceName, hUint32* loadCounter )
	{
        hUint32 rescrc32 = BuildResourceCRC( resourceName );
        resourceDatabaseMutex_.Lock();
		//is the resource loaded?
		hResourceClassBase* resInfo = loadedResources_.Find( rescrc32 );
        resourceDatabaseMutex_.Unlock();

		if ( resInfo )
		{
			// The resource is already loaded in memory increase its ref count
			// then return
            hAtomic::Increment( loadCounter );
			resInfo->AddRef();
			return;
		}

		//JM TODO: check the cache
		//pRet = QueryResourceCache( rescrc32 );

		//not found in cache

		// Request already open?
		hResourceLoadRequest* request;


		//create a load request.
		request = hNEW ( hGeneralHeap ) hResourceLoadRequest();
		//create space in the request in the loadedResources
		request->crc32_ = rescrc32;
        request->loadCounter_ = loadCounter;
		request->path_ = hNEW ( hGeneralHeap ) hChar[strlen(resourceName)+1];
        strcpy( request->path_, resourceName );

		loadRequests_.Insert( rescrc32, request );
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourceManager::CompleteLoadRequest( hResourceLoadRequest* request )
    {
        hUint32 crc = BuildResourceCRC( request->path_ );

        LoadResourceFromPath( request->path_ );

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

    void hResourceManager::LoadResourceFromPath( const hChar* path )
    {
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

        // must load all the dependency folder first
        hUint32 extLen = strlen( path );
        hcAssertMsg( path[ extLen-4 ] == '.', "resource %s doesn't seem to have a valid extention", path );

        depPath = (hChar*)alloca( extLen + 1 );

        strcpy_s( depPath, extLen+1, path );

        depPath[ extLen - 4 ] = '_';
        depPath[ extLen - 3 ] = 'D';
        depPath[ extLen - 2 ] = 'E';
        depPath[ extLen - 1 ] = 'P';

        filesystem_->EnumerateFiles( depPath, EnumerateFilesCallback::bind< hResourceManager, &hResourceManager::EnumerateAndLoadDepFiles >( this ) );

        // everything ready, init this resources data add remove from the 
        // list
        ResourceType resType;
        //ext is ALWAYS 4 bytes long ( e.g. '.tex' )
        const hChar* ext = path + (strlen( path ) - 4);
        hcAssert( *ext == '.' );
        strcpy_s( resType.ext, 4, ext+1 );

        ResourceHandler* handler = resHandlers_.Find( resType );
        hcAssertMsg( handler, "Can't find resource handler data for extention %s", ext );

        resourceDatabaseMutex_.Lock();
        hResourceClassBase* resource = loadedResources_.Find( crc );
        resourceDatabaseMutex_.Unlock();

        // The resource isn't loaded, so grab it
        if ( !resource )
        {
            hSerialiserFileStream loaderStream;
            loaderStream.Open( path, hFalse, filesystem_ );
            hcPrintf( "Loading Resource %s (crc32: %u)", path, crc );
            resource = handler->loadCallback( 
                resType.ext, 
                crc, 
                &loaderStream, 
                this );
            loaderStream.Close();

            hcAssert( resource );
            resource->SetResID( crc );
            resource->IsDiskResource( hTrue );
            resource->manager_ = this;
            resourceDatabaseMutex_.Lock();
            // Push the new resource into loaded resource map
            loadedResources_.Insert( crc, resource );
            resourceDatabaseMutex_.Unlock();
        }

        // increment the resource count
        resource->AddRef();
    }

    //////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hResourceManager::OptimiseReads()
	{
		//TODO:
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hResourceManager::DoResourceSweep()
	{

	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hBool hResourceManager::RequiredResourcesReady()
	{
        if ( requiredResourcesPackage_.IsPackageLoaded() )
        {
            if ( !gotRequiredResources_ )
            {
                requiredResourcesPackage_.GetResourcePointers();
                gotRequiredResources_ = hTrue;
            }
            return hTrue;
        }
		return hFalse;
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
                    ResourceRemap* remap = hNEW( hGeneralHeap ) ResourceRemap();
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

}