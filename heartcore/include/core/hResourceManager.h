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
    class HeartEngine;

	class hResourceManager;
	struct hLoadedResourceInfo;
	class hIReferenceCounted;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	struct hLoadedResourcePackages : public hMapElement< hUint32, hLoadedResourcePackages >,
                                     public hIReferenceCounted
	{
        hLoadedResourcePackages() 
            : loading_(hFalse)
            , package_(NULL)
		{
            hZeroMem(path_, sizeof(HEART_RESOURCE_PATH_SIZE));
        }

		hChar					        path_[HEART_RESOURCE_PATH_SIZE];
        hBool                           loading_;
        hResourcePackageV2*             package_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class HEARTCORE_SLIBEXPORT hResourceManager
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

        hBool                           Initialise( HeartEngine* engine, hRenderer* renderer, hIFileSystem* pFileSystem, const char** requiredResources );
        hBool                           RequiredResourcesReady();
        void                            MainThreadUpdate();
        static hUint32                  BuildResourceCRC( const hChar* resourceName );
        static hResourceID              BuildResourceID(const hChar* fullPath);
        static hResourceID              BuildResourceID(const hChar* package, const hChar* resourceName);
        void                            Shutdown( hRenderer* prenderer );
        hRenderMaterialManager*         GetMaterialManager() { return materialManager_; }
        hRenderer*                      GetRederer() { return renderer_; }

        //Main Thread interface
        hResourceClassBase*             mtGetResource(const hChar* path);
        hResourceClassBase*             mtGetResource(hResourceID crc);

        //Loader thread interface   
        hResourceClassBase*             ltGetResource(hResourceID crc);

        // New interface
        void                            mtLoadPackage(const hChar* name);
        void                            mtUnloadPackage(const hChar* name);
        hBool                           mtIsPackageLoaded(const hChar* name);

    private:
        // New Private calls
        void                            LoadGamedataDesc();

	private:

        static const hUint32 QUEUE_MAX_SIZE = 64;

        struct StreamingResouce : public hMapElement< hUint32, StreamingResouce >
        {
            hStreamingResourceBase*     stream_;
        };

        struct ResourcePackageQueueMsg
        {
            hResourcePackageV2*         package_;
        };

        struct ResourcePackageLoadMsg
        {
            hChar					    path_[HEART_RESOURCE_PATH_SIZE];
        };

        typedef hMap< hUint32, StreamingResouce >                   StreamingResourceMap;
        typedef hMap< hUint32, hResourcePackageV2 >                 ResourcePackageMap;
        typedef hMap< hUint32, hLoadedResourcePackages >            LoadedResourcePackageMap;
        typedef hQueue< ResourcePackageLoadMsg, QUEUE_MAX_SIZE >    ResourceLoadRequestQueue;
        typedef hQueue< ResourcePackageQueueMsg, QUEUE_MAX_SIZE >   ResourceMsgQueue;

        hUint32                         LoadedThreadFunc( void* );

        static void*                    resourceThreadID_;

        hBool                           requireAssetsReady_;

		//NEW
        hRenderer*                      renderer_;
        hRenderMaterialManager*         materialManager_;
        hIFileSystem*                   filesystem_;

		//
		hThread							resourceLoaderThread_;
		hSemaphore						loaderSemaphone_;
		hBool							exitSignal_;
		hAtomicInt          			resourceSweepCount_;

        //Loader Thread var
        ResourceLoadRequestQueue        ltLoadRequests_;
        ResourceMsgQueue                ltUnloadRequest_;
        ResourcePackageMap              ltLoadedPackages_;

        //Intermediate-ish Vars
        hMutex                          ltAccessMutex_;
        ResourceMsgQueue                ltPackageLoadCompleteQueue_;
        
        //Main Thread vars
        LoadedResourcePackageMap        mtLoadedPackages_;
        ResourceLoadRequestQueue        mtLoadRequests_;
        ResourceMsgQueue                mtUnloadRequest_;

        

        //New vars
        hXMLDocument                    gamedataDescXML_;
        hResourceHandlerMap             resourceHandlers_;

        //main thread loaded packages

        HeartEngine*                    engine_;
	};

}

#endif // RESOURCEMANAGER_H__