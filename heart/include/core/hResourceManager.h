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
    class hHeartEngine;

    class hResourceManager;
    struct hLoadedResourceInfo;
    class hIReferenceCounted;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    struct hLoadedResourcePackages : public hMapElement< hUint32, hLoadedResourcePackages >
    {
        hLoadedResourcePackages() 
            : loading_(hFalse)
            , package_(NULL)
        {
        }

        hBool                         loading_;
        hResourcePackage*             package_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class HEART_DLLEXPORT hResourceManager
    {
    public:

        enum Mode
        {
            READ,
            WRITE,
        };

        hFUNCTOR_TYPEDEF(hResourceClassBase*(*)(const hChar*, hUint32 resId, hSerialiserFileStream*, hResourceManager*), ResourceLoadCallback);
        hFUNCTOR_TYPEDEF(hUint32(*)(const hChar*, hResourceClassBase*, hResourceManager* ),  ResourceUnloadCallback);

        hResourceManager();
        ~hResourceManager();

        hBool                           Initialise(hHeartEngine* engine, hRenderer* renderer, hIFileSystem* pFileSystem, const char** requiredResources);
        hBool                           RequiredResourcesReady();
        void                            MainThreadUpdate();
        static hUint32                  BuildResourceCRC( const hChar* resourceName );
        static hResourceID              BuildResourceID(const hChar* fullPath);
        static hResourceID              BuildResourceID(const hChar* package, const hChar* resourceName);
        void                            Shutdown( hRenderer* prenderer );
        hRenderMaterialManager*         GetMaterialManager() { return materialManager_; }
        hRenderer*                      GetRederer() { return renderer_; }
        void                            printResourceInfo() { printInfo_.Signal(); loaderSemaphone_.Post(); }

        //Main Thread interface
        hResourceClassBase*             mtGetResource(const hChar* path);
        hResourceClassBase*             mtGetResource(hResourceID crc);

        //Loader thread interface   
        void                            ltLoadPackage(const hChar* name);
        void                            ltUnloadPackage(const hChar* name);
        hBool                           ltIsPackageLoaded(const hChar* name);
        hResourceClassBase*             ltGetResource(hResourceID crc);

        // New interface
        void                            mtLoadPackage(const hChar* name);
        void                            mtUnloadPackage(const hChar* name);
        hBool                           mtIsPackageLoaded(const hChar* name);

    private:

        static const hUint32 QUEUE_MAX_SIZE = 64;

        struct StreamingResouce : public hMapElement< hUint32, StreamingResouce >
        {
            hStreamingResourceBase*     stream_;
        };

        struct ResourcePackageQueueMsg
        {
            hUint32                   packageCRC_;
            hResourcePackage*         package_;
        };

        struct ResourcePackageLoadMsg
        {
            hChar					    path_[HEART_RESOURCE_PATH_SIZE];
        };

        typedef hMap< hUint32, StreamingResouce >                   StreamingResourceMap;
        typedef hMap< hUint32, hResourcePackage >                 ResourcePackageMap;
        typedef hMap< hUint32, hLoadedResourcePackages >            LoadedResourcePackageMap;
        typedef hQueue< ResourcePackageLoadMsg, QUEUE_MAX_SIZE >    ResourceLoadRequestQueue;
        typedef hQueue< ResourcePackageQueueMsg, QUEUE_MAX_SIZE >   ResourceMsgQueue;

        hUint32                         LoadedThreadFunc( void* );
        void                            LoadGamedataDesc();
        void loaderThreadPrintResourceInfo();
        static void*                    resourceThreadID_;

        hBool                           requireAssetsReady_;
        hThreadEvent                    printInfo_;

        //NEW
        hRenderer*                      renderer_;
        hRenderMaterialManager*         materialManager_;
        hIFileSystem*                   filesystem_;

        //
        hThread                         resourceLoaderThread_;
        hSemaphore                      loaderSemaphone_;
        hThreadEvent                    exitSignal_;
        hAtomicInt                      resourceSweepCount_;

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
        hHeartEngine*                    engine_;
    };

}

#endif // RESOURCEMANAGER_H__