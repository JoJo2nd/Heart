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

        hBool                           Initialise( hRenderer* renderer, hIFileSystem* pFileSystem, const char** requiredResources );
        hBool                           RequiredResourcesReady();
        void                            MainThreadUpdate();
        static hUint32                  BuildResourceCRC( const hChar* resourceName );
        void                            SetResourceHandlers( const hChar* typeExt, ResourceLoadCallback onLoad, ResourceUnloadCallback onUnload, void* pUserData );
        hUint32                         GetResourceKeyRemapping( hUint32 key ) const;
        hResourceClassBase*             LoadResourceFromPath( const hChar* path );
        void                            QueueResourceSweep() { hAtomic::Increment( resourceSweepCount_ ); }
        void                            Post() { loaderSemaphone_.Post(); }
        void                            Shutdown( hRenderer* prenderer );
        hRenderMaterialManager*         GetMaterialManager() { return materialManager_; }
        hRenderer*                      GetRederer() { return renderer_; }

        //Resource Thread interface
        hResourceClassBase*             ltGetResourceAddRef(hUint32 crc);
        hResourceClassBase*             ltGetResourceWeak(hUint32 crc);

        //Main Thread interface
        void                            mtLoadResource(const hChar* path);
        hResourceClassBase*             mtGetResourceAddRef(const hChar* path);
        hResourceClassBase*             mtGetResourceAddRef(hUint32 crc);
        hResourceClassBase*             mtGetResourceWeak(const hChar* path);
        hResourceClassBase*             mtGetResourceWeak(hUint32 crc);

	private:

        static const hUint32 QUEUE_MAX_SIZE = 64;

		struct ResourceHandler : public hMapElement< hResourceType, ResourceHandler >
		{
			ResourceLoadCallback		loadCallback;
			ResourceUnloadCallback		unloadCallback;
		};

        struct ResourceRemap : public hMapElement< hUint32, ResourceRemap >
        {
            hUint32 mapToPath;
        };

        struct StreamingResouce : public hMapElement< hUint32, StreamingResouce >
        {
            hStreamingResourceBase*     stream_;
        };

        struct MainThreadResourceHandle : public hMapElement< hUint32, MainThreadResourceHandle >
        {
            hUint32                     resourceKey_;
            hResourceClassBase*         resource_;
        };

        typedef hMap< hResourceType, ResourceHandler >              ResourceHandlerMap;
        typedef hMap< hUint32, hResourceClassBase >                 ResourceMap;
        typedef hMap< hUint32, MainThreadResourceHandle >           MainThreadResourceMap;
        typedef hMap< hUint32, StreamingResouce >                   StreamingResourceMap;
        typedef hMap< hUint32, hResourceLoadRequest >               ResourceLoadRequestMap;
        typedef hMap< hUint32, ResourceRemap >                      ResourceRemappingMapType;
        typedef hQueue< hResourceLoadRequest, QUEUE_MAX_SIZE >      ResourceLoadRequestQueue;
        typedef hQueue< MainThreadResourceHandle, QUEUE_MAX_SIZE >  ResourceHandleQueue;

        hUint32                         LoadedThreadFunc( void* );
        hResourceClassBase*             CompleteLoadRequest( hResourceLoadRequest* request );
        hBool                           EnumerateAndLoadDepFiles( const FileInfo* fileInfo );
        void                            LoadResourceRemapTable();

		static hResourceManager*		pInstance_;
        static void*                    resourceThreadID_;

		//NEW
		hIFileSystem*					filesystem_;
        hRenderer*                      renderer_;
        hRenderMaterialManager*         materialManager_;
		ResourceHandlerMap				resHandlers_;
        hUint32                         remappingNamesSize_;
        hChar*                          remappingNames_;
        ResourceRemappingMapType        remappings_;//this is const, so read accross threads are safe

		//
		hThread							resourceLoaderThread_;
		hSemaphore						loaderSemaphone_;
		hBool							exitSignal_;
		hAtomicInt          			resourceSweepCount_;

        //Loader Thread var
        ResourceLoadRequestQueue        loadRequests_;
        ResourceHandleQueue             completedLoads_;
        ResourceHandleQueue             unloadRequests_;
		ResourceMap                     loadedResources_;
        StreamingResourceMap            streamingResources_;
		hVector< const hChar* >			requiredResourceKeys_;
		hVector< hResourceClassBase* >	requiredResources_;
        hResourcePackage                requiredResourcesPackage_;
        hBool                           gotRequiredResources_;

        //Main Thread vars
        MainThreadResourceMap           mtResourceMap_;
        ResourceLoadRequestQueue        mtLoadRequests_;
        ResourceHandleQueue             mtResourceLoadedQueue_;

        //Intermediate Vars
        hMutex                          ivAccessMutex_;
        ResourceLoadRequestQueue        ivLoadRequests_;
        ResourceHandleQueue             ivResourceLoadedQueue_;
        ResourceHandleQueue             ivResourceUnloadedQueue_;
	};

}

#endif // RESOURCEMANAGER_H__