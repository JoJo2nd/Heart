/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef HRESOURCEPACKAGE_H__
#define HRESOURCEPACKAGE_H__

#include "base/hTypes.h"
#include "base/hMap.h"
#include "base/hClock.h"
#include "base/hReferenceCounted.h"
#include "core/hProtobuf.h"
#include "core/hIFileSystem.h"
#include "core/hResource.h"
#include "components/hObjectFactory.h"
#include "threading/hJobManager.h"
#include <queue>

namespace Heart
{
    class hHeartEngine;

    static const hUint32			HEART_RESOURCE_PATH_SIZE = 1024;

    typedef google::protobuf::io::ArrayInputStream hResourceFileStream;


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    struct hResourceSection
    {
        const hChar*                     sectionName_;
        hInt                             sectionSize_;
        const void*                      sectionData_;
        proto::ResourceSectionMemoryType memType_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

#if 0 // Removing, now handled by hObjectFactory methods
    hFUNCTOR_TYPEDEF(hResourceClassBase* (*)(const hResourceSection*,   hUint), hResourceLoadProc);
    hFUNCTOR_TYPEDEF(void                (*)(hResourceManager*, hResourceClassBase*), hResourcePostLoadProc);
    hFUNCTOR_TYPEDEF(void                (*)(hResourceManager*, hResourceClassBase*), hResourcePreUnloadProc);
    hFUNCTOR_TYPEDEF(void                (*)(hResourceClassBase*), hResourceUnloadProc);

    struct hResourceHandler : public hMapElement< hResourceType, hResourceHandler >
    {
        hResourceLoadProc       loadProc_;
        hResourcePostLoadProc   postLoadProc_; 
        hResourcePreUnloadProc  preUnloadProc_;
        hResourceUnloadProc     unloadProc_;
    };

    typedef hMap< hResourceType, hResourceHandler > hResourceHandlerMap;
#endif

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hResourcePackage : public hIReferenceCounted
    {
    public:
        hObjectType(Heart::hResourcePackage, Heart::proto::PackageHeader);

        hResourcePackage();
        ~hResourcePackage();

        void                    initialise(hIFileSystem* filesystem, hJobQueue* fileQueue, const hChar* packageName);
        const hChar*            getPackageName() const { return packageName_.c_str(); }
        hUint32                 getPackageCRC() const { return packageName_.hash(); }
        const hStringID&        getPackageID() const { return packageName_; }
        void                    unload();
        void                    beginPackageDestroy() { hcAssert(isUnloaded()); packageState_ = PkgState::Destroying; }
        hBool                   update();
        hBool                   isInReadyState() const { return packageState_ == PkgState::Loaded; }
        hBool                   isUnloaded() const { return packageState_ == PkgState::Unloaded; }
        hBool                   isDestroyed() const { return packageState_ == PkgState::Destroyed; }
        void                    printResourceInfo();
        const hChar*            getPackageStateString() const;

    private:

        static const hUint32    MAX_PACKAGE_NAME = 128;

        struct hResourceLoadJobInputOutput {
            const hObjectDefinition* objectDef_;
            hByte*                   resMemStart_;
            hByte*                   resMemEnd_;
            void*                    createdResource_;
            hStringID                resourceID_;
            hStringID                resourceType_;
            hBool                    linked_;
        };

        typedef hVector< hStringID > PkgLinkArray;
        typedef std::vector< hResourceLoadJobInputOutput > ResourceJobArray;

        // Jobs
        void  loadPackageDescription(void*, void*);

        enum class PkgState {
            Null,
            LoadPkgDesc,
            FileReadWait,
            RequestLinkedPkgs,
            LoadingResources,
            ResourceLinking,
            Loaded,
            Unloaded,
            Destroying,
            Destroyed,
        };

        hStringID                   packageName_;
        hChar                       packagePath_[MAX_PACKAGE_NAME];
        PkgState                    packageState_;
        hIFileSystem*               fileSystem_;
        hUint                       nextResourceToLoad_;
        hUint                       totalResources_;
        hUint                       linkedResources_;
        std::vector<hChar>          pkgFileData;
        PkgLinkArray                packageLinks_;

        proto::PackageHeader        packageHeader_;
        ResourceJobArray            resourceJobArray_;
        hTimer                      timer_;
        hJobQueue*                  fileQueue_;
    };

}

#endif // HRESOURCEPACKAGE_H__
