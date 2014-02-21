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

    typedef std::unordered_multimap< hResourceID, hResourceEventProc, hResourceID::hash > hResourceEventMap;
    typedef std::unordered_map< hResourceID, hResourceClassBase*, hResourceID::hash > hResourceHandleMap;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hResourceEventUpdateProcess
    {
    public:
        hResourceEventUpdateProcess();
        ~hResourceEventUpdateProcess();

        hBool update();
        hBool isComplete() const;

    private:
        HEART_PRIVATE_COPY(hResourceEventUpdateProcess);
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

        hBool                           initialise(hHeartEngine* engine, hRenderer* renderer, hIFileSystem* pFileSystem, hJobManager* jobmanager, const char** requiredResources);
        void                            registerResourceHandler(const hChar* resourcetypename, hResourceHandler handler);
        void                            registerResourceEventHandler(hResourceID resid, hResourceEventProc proc);
        void                            unregisterResourceEventHandler(hResourceID resid, hResourceEventProc proc);
        void                            submitResourceEvent(hResourceEventUpdateProcess& updater);
        void                            update();
        static hResourceID              BuildResourceID(const hChar* fullPath){ return hResourceID::buildResourceID(fullPath); }
        static hResourceID              BuildResourceID(const hChar* package, const hChar* resourceName) { return hResourceID::buildResourceID(package, resourceName); }
        void                            shutdown( hRenderer* prenderer );
        void                            printResourceInfo();

        //Main Thread interface
        void                            insertResource(const hChar* name, hResourceClassBase* res) {
            insertResource(hResourceManager::BuildResourceID(name), res);
        }
        void                            insertResource(hResourceID id, hResourceClassBase* res);
        hUint                           removeResource(const hChar* name) {
            return removeResource(hResourceManager::BuildResourceID(name));
        }
        hUint                           removeResource(hResourceID id);

        // New interface
        void                            loadPackage(const hChar* name);
        void                            unloadPackage(const hChar* name);
        hBool                           getIsPackageLoaded(const hChar* name);

    private:

        friend hResourceClassBase* hResourceHandle::weakPtr() const;

        struct StreamingResouce : public hMapElement< hUint32, StreamingResouce >
        {
            hStreamingResourceBase*     stream_;
        };

        typedef hMap< hUint32, StreamingResouce >                       StreamingResourceMap;
        typedef hMap< hUint32, hResourcePackage >                       ResourcePackageMap;

        hResourceClassBase* getResourceForHandle(hResourceID crc);

        //NEW
        hRenderer*                      renderer_;
        hRenderMaterialManager*         materialManager_;
        hIFileSystem*                   filesystem_;
        hJobManager*                    jobManager_;

        //New vars
        hXMLDocument                    gamedataDescXML_;
        hResourceHandlerMap             resourceHandlers_;

        //main thread loaded packages
        hHeartEngine*                    engine_;

        hJobQueue fileReadJobQueue_;
        hJobQueue workerQueue_;

        ResourcePackageMap  activePackages_;

        hResourceHandleMap  resourceHandleMap_;
        hResourceEventMap   resourceEventMap_;
    };

}

#endif // RESOURCEMANAGER_H__