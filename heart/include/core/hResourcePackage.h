/********************************************************************

    filename: 	hResourcePackage.h	
    
    Copyright (c) 14:8:2011 James Moran
    
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
#ifndef HRESOURCEPACKAGE_H__
#define HRESOURCEPACKAGE_H__

namespace Heart
{
    class hResourceManager;
    class hResourceClassBase;
    class hHeartEngine;

    static const hUint32			HEART_RESOURCE_PATH_SIZE = 1024;

    //typedef hUint64 hResourceID;
    struct hResourceID
    {
        hUint64 hash_;

        hResourceID() : hash_(0) {}
        explicit hResourceID(hUint64 v) : hash_(v) {}
        hResourceID(const hResourceID& rhs) : hash_(rhs.hash_) {}
        hBool operator == (const hResourceID& rhs) const { return hash_ == rhs.hash_; }
        hBool operator != (const hResourceID& rhs) const { return hash_ != rhs.hash_; }
        hResourceID& operator = (const hResourceID& rhs) { hash_ = rhs.hash_; return *this; }
        operator hUint64() { return hash_; }
    };

    

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    typedef hResourceClassBase* (*OnResourceDataLoad)       (hISerialiseStream*, hIDataParameterSet*, hResourceMemAlloc*, hHeartEngine*);
    typedef hBool               (*OnResourceDataCompile)    (hIDataCacheFile*, hIBuiltDataCache*, hIDataParameterSet*, hResourceMemAlloc*, hHeartEngine*, hISerialiseStream*);
    typedef hBool               (*OnPackageLoadComplete)    (hResourceClassBase*, hResourceMemAlloc*, hHeartEngine*);
    typedef void                (*OnResourceDataUnload)     (hResourceClassBase*, hResourceMemAlloc*, hHeartEngine*);
    typedef void                (*OnPackageUnloadComplete)  (hResourceClassBase*, hResourceMemAlloc*, hHeartEngine*);
    typedef hTime               (*GetTimestamp)             ();

    struct hResourceHandler : public hMapElement< hResourceType, hResourceHandler >
    {
        hResourceType           type_;
        hSharedLibAddress       loaderLib_;
        OnResourceDataLoad      binLoader_;
        OnResourceDataCompile   rawCompiler_;
        OnPackageLoadComplete   packageLink_;
        OnResourceDataUnload    resourceDataUnload_;
        OnPackageUnloadComplete packageUnlink_;
        GetTimestamp            getTimestamp_;
    };

    typedef hMap< hResourceType, hResourceHandler > hResourceHandlerMap;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hResourcePackage : public hMapElement< hUint32, hResourcePackage >
                           , public hIReferenceCounted
    {
    public:
        hResourcePackage(hHeartEngine* engine, hIFileSystem* filesystem, const hResourceHandlerMap* handlerMap );
        ~hResourcePackage();

        const hChar*            GetPackageName() const { return packageName_; }
        hUint32                 GetPackageCRC() const { return packageCRC_; }
        hResourceClassBase*     GetResource(hUint32 crc) const;

        //Only call the following on the loader thread...
        hUint32                 LoadPackageDescription(const hChar* );
        void                    prepareReload() { doReload_ = hTrue; }
        hUint32                 GetPackageDependancyCount() const;
        const hChar*            GetPackageDependancy(hUint32 i) const;
        hUint32                 GetLoadCompletionPercent() { return (loadedResources_*100) / totalResources_;  }
        hBool                   Update(hResourceManager* manager);//Returns true when package whiches to loaded state
        void                    Unload();
        hBool                   IsInPassiveState() const { return packageState_ == State_Ready; }
        hBool                   isUnloading() const { return packageState_ > State_Ready; }
        hBool                   ToUnload() const { return packageState_ == State_Unloaded; }
        void                    printResourceInfo();
        const hChar*            getPackageStateString() const;
        hUint                   getLinkCount() const { return links_.GetSize(); }
        const hChar*            getLink(hUint i) const { return links_[i]; }

    private:

        static const hUint32    MAX_PACKAGE_NAME = 128;

        typedef hVector< const hChar* > PkgLinkArray;
        typedef hMap< hUint32, hResourceClassBase > ResourceMap;

        void                        LoadResourcesState();
        hBool                       DoPostLoadLink();
        void                        DoPreUnloadUnlink();
        void                        DoUnload();
        enum State
        {
            State_Load_DepPkgs,
            State_Load_WaitDeps,
            State_Load_Reources,
            State_Link_Resources,
            State_Ready,
            State_Unlink_Resoruces,
            State_Unload_Resources,
            State_Unload_DepPkg,
            State_Unloaded,
        };

        hChar                       packageName_[MAX_PACKAGE_NAME];
        hUint32                     packageCRC_;
        hBool                       doReload_;
        State                       packageState_;
        hHeartEngine*                engine_;
        const hResourceHandlerMap*  handlerMap_;
        hMemoryHeapBase*            packageHeap_;
        hMemoryHeap                 tempHeap_;
        hZipFileSystem*             zipPackage_;
        hIFileSystem*               driveFileSystem_;
        hIFileSystem*               fileSystem_;
        hUint32                     loadedResources_;
        hUint32                     totalResources_;
        hXMLDocument                descXML_;
        PkgLinkArray                links_;
        hXMLGetter                  currentResource_;
        ResourceMap                 resourceMap_;
    };

}

#endif // HRESOURCEPACKAGE_H__
