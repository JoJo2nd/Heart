/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "base/hMap.h"
#include "base/hClock.h"
#include "base/hReferenceCounted.h"
#include "core/hIFileSystem.h"
#include "components/hObjectFactory.h"
#include "threading/hTaskGraphSystem.h"
#include <queue>
#include "package.pb.h"

namespace Heart
{
    class hHeartEngine;

    static const hUint32			HEART_RESOURCE_PATH_SIZE = 1024;

    typedef google::protobuf::io::ArrayInputStream hResourceFileStream;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hResourcePackage : public hIReferenceCounted
    {
    public:
        hObjectType(hResourcePackage, Heart::proto::PackageHeader);

        hResourcePackage();
        ~hResourcePackage();

        void                    initialise(hIFileSystem* filesystem, const hChar* packageName);
        const hChar*            getPackageName() const { return packageName_.c_str(); }
        hUint32                 getPackageCRC() const { return packageName_.hash(); }
        const hStringID&        getPackageID() const { return packageName_; }
        void                    unload();
        void                    beginPackageDestroy() { hcAssert(isUnloaded()); packageState_ = PkgState::Destroying; }
        hBool                   update();
        hBool                   isInReadyState() const { return packageState_ == PkgState::Loaded; }
        hBool                   isUnloaded() const { return packageState_ == PkgState::Unloaded; }
        hBool                   isDestroyed() const { return packageState_ == PkgState::Destroyed; }
#if HEART_DEBUG_INFO
        void                    printResourceInfo();
        const hChar*            getPackageStateString() const;
#endif

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
        hUint32 loadPackageDescription(hTaskInfo*);

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
        hTaskGraph                  taskGraph;
    };

}
