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

#include "hTypes.h"
#include "hResource.h"
#include "hIFileSystem.h"
#include "huFunctor.h"
#include "hMutex.h"
#include "hDeferredReturn.h"
#include "hThread.h"
#include "hThreadEvent.h"
#include "hSemaphore.h"
#include "hSerialiserFileStream.h"
#include "hResourcePackage.h"

namespace Heart
{

	class hRenderer;
    class hRenderMaterialManager;
	class hIFile;

	class hResourceManager;
	struct hLoadedResourceInfo;
	class hIReferenceCounted;

	static const hUint32			RESOURCE_PATH_SIZE = 1024;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	struct hResourceLoadRequest : public hMapElement< hUint32, hResourceLoadRequest >
	{
		hResourceLoadRequest() 
			: path_( NULL )
			, crc32_( 0 )
            , loadCounter_(NULL)
		{}

	private:
		friend class hResourceManager; 

		hChar*					        path_;
		hUint32							crc32_;
        hUint32*                        loadCounter_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hResourceManager
	{
	public:

		enum Mode
		{
			READ,
			WRITE,
		};

		typedef huFunctor< hResourceClassBase*(*)(const hChar*, hUint32 resId, hSerialiserFileStream*, hResourceManager* ) >::type			ResourceLoadCallback;
		typedef huFunctor< hUint32(*)(const hChar*, hResourceClassBase*, hResourceManager* ) >::type			ResourceUnloadCallback;

		hResourceManager();
		~hResourceManager();

		/**
		* Initialise 
		*
		* @return   hBool
		*/
		hBool							Initialise( hRenderer* renderer, hIFileSystem* pFileSystem, const char** requiredResources );

		hBool							RequiredResourcesReady();

		static hUint32					BuildResourceCRC( const hChar* resourceName );

		/**
		* SetResourceHandlers 
		*
		* @param 	const hChar * typeExt
		* @param 	ResourceLoadCallback onLoad
		* @param 	ResourceUnloadCallback onUnload
		* @return   void
		*/
		void							SetResourceHandlers( const hChar* typeExt, ResourceLoadCallback onLoad, ResourceUnloadCallback onUnload, void* pUserData );

		/**
		* GetResource - Attempts to get a resource. 
		*
		* @param 	const hChar * resourceName
		* @param 	hResource< _Ty > & resource
		* @return   hBool
		*/
		void							BeginResourceLoads( const hChar** resourceKeys, hUint32 count, hUint32* loadCounter );

		void							CancelResourceLoads( const hUint32* resourceKeys, hUint32 count );
		
		hBool							ResourceLoadsComplete( const hUint32* resourceKeys, hUint32 count );
		
		void							GetResources( const hChar** resourceKeys, hResourceClassBase** outPtrs, hUint32 count );
        hUint32                         GetResourceKeyRemapping( hUint32 key ) const;
        void                            LockResourceDatabase() { resourceDatabaseMutex_.Lock(); resourceDatabaseLocked_ = hTrue; }
        hResourceClassBase*             GetResource( hUint32 crc ) { hcAssert( resourceDatabaseLocked_ ); return loadedResources_.Find( crc ); }
        void                            UnlockResourceDatabase() { resourceDatabaseLocked_ = hFalse; resourceDatabaseMutex_.Unlock(); }

		/**
		* QueueResourceSweep - Requestes the resource manager to search for unused resources and
		* unload them
		*
		* @return   void
		*/
		void							QueueResourceSweep()
		{
			hAtomic::Increment( &resourceSweepCount_ );
			loaderSemaphone_.Post();
		}

		/**
		* ForceResourceSweep - Forces a resource collect to happen
		*
		* @return   void
		*/
		void							ForceResourceSweep()
		{
			loaderSemaphone_.Post();
		}

		/**
		* Shutdown 
		*
		* @return   void
		*/
		void							Shutdown( hRenderer* prenderer );

        hRenderMaterialManager*         GetMaterialManager() { return materialManager_; }
        hRenderer*                      GetRederer() { return renderer_; }



	private:

		struct ResourceType
		{
			hChar						ext[4];

			hBool						operator == ( const ResourceType& b ) const 
			{
				return strcmp( ext, b.ext ) == 0;
			}
			hBool						operator < ( const ResourceType& b ) const
			{
				return strcmp( ext, b.ext ) < 0;
			}
		};

		struct ResourceHandler : public hMapElement< ResourceType, ResourceHandler >
		{
			ResourceLoadCallback		loadCallback;
			ResourceUnloadCallback		unloadCallback;
		};

        struct ResourceRemap : public hMapElement< hUint32, ResourceRemap >
        {
            hUint32 mapToPath;
        };

		typedef hMap< ResourceType, ResourceHandler >		ResourceHandlerMap;
		typedef hMap< hUint32, hResourceClassBase >			ResourceMap;
		typedef hMap< hUint32, hResourceLoadRequest >		ResourceLoadRequestMap;
        typedef hMap< hUint32, ResourceRemap >              ResourceRemappingMapType;

		/**
		* LoadResource - Attempts to get a resource. If resource is not yet IsLoaded, It begins the load process.
		*
		* @param 	const hChar * resourceName
		*/
		void							LoadResource( const hChar* resourceName, hUint32* loadCounter );
 		hUint32							ProcessDataFixup( void* );
		hResourceClassBase*				QueryResourceCache( hUint32 crc ) { return NULL; }//< TODO
		void							OptimiseReads();

		void							DoResourceSweep();
        void                            CompleteLoadRequest( hResourceLoadRequest* request );
        void                            LoadResourceFromPath( const hChar* path );
        hBool							EnumerateAndLoadDepFiles( const FileInfo* fileInfo );
        void                            LoadResourceRemapTable();

		static hResourceManager*		pInstance_;

		//NEW
		hIFileSystem*					filesystem_;
        hRenderer*                      renderer_;
        hRenderMaterialManager*         materialManager_;
		ResourceHandlerMap				resHandlers_;
        hUint32                         remappingNamesSize_;
        hChar*                          remappingNames_;
        ResourceRemappingMapType        remappings_;

		//
		hThread							resourceLoaderThread_;
		hSemaphore						loaderSemaphone_;
		hMutex							resourceDatabaseMutex_;
        hBool                           resourceDatabaseLocked_;

		hBool							exitSignal_;
		hBool							canQuit_;
		mutable hUint32					resourceSweepCount_;

		ResourceLoadRequestMap			loadRequests_;
		ResourceLoadRequestMap			loadRequestsProcessed_;
		ResourceMap						loadedResources_;

		hVector< hUint32 >				requiredResourceKeys_;
		hVector< hResourceClassBase* >	requiredResources_;
        hResourcePackage                requiredResourcesPackage_;
        hBool                           gotRequiredResources_;
		hResourceLoadRequest*			pVolatileDepResource_;
	};

}

#endif // RESOURCEMANAGER_H__