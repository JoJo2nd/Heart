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

#include "base/hTypes.h"
#include "base/hMap.h"
#include "base/hReferenceCounted.h"
#include "base/hProtobuf.h"
#include "base/hClock.h"
#include "core/hIFile.h"
#include "core/hIFileSystem.h"
#include "core/hResource.h"
#include "components/hObjectFactory.h"
#include "threading/hJobManager.h"
#include "base/hDeviceFileWatch.h"
#include <queue>

namespace Heart
{
    class hHeartEngine;

    static const hUint32			HEART_RESOURCE_PATH_SIZE = 1024;

    class hResourceFileStream : public google::protobuf::io::ZeroCopyInputStream
    {
    public:
        hResourceFileStream(hIFile* file) 
            : file_(file)
            , pos_(0)
            , bytesRead_(0)
        {
            bytes_=new hByte[s_bufferSize];
        }
        ~hResourceFileStream() {
            delete[] bytes_;
            bytes_ = nullptr;
        }

        virtual bool Next(const void** data, int* size) {
            // read in 32K
            hUint32 read = file_->Read(bytes_, s_bufferSize);
            *data = bytes_;
            *size = read;
            pos_=file_->Tell();
            bytesRead_ += read;
            return read != 0;
        };

        virtual void BackUp(int count) {
            file_->Seek(count, SEEKOFFSET_CURRENT);
            pos_=file_->Tell();
        };

        virtual bool Skip(int count) { 
            hUint64 expectpos=pos_+count;
            file_->Seek(count, SEEKOFFSET_CURRENT);
            pos_=file_->Tell();
            return expectpos==file_->Tell();
        };
        virtual google::protobuf::int64 ByteCount() const { return bytesRead_; };

    private:
        hResourceFileStream(const hResourceFileStream& rhs);
        hResourceFileStream& operator = (hResourceFileStream);

        static const hUint32 s_bufferSize = 32*1024;

        hIFile* file_;
        hUint64 pos_;
        hUint64 bytesRead_;
        hByte*  bytes_;
    };


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

    class hResourcePackage
    {
    public:
        hObjectType(Heart::hResourcePackage, Heart::proto::PackageHeader);

        hResourcePackage();
        ~hResourcePackage();

        void                    initialise(hIFileSystem* filesystem, hJobQueue* fileQueue, const hChar* packageName);
        const hChar*            getPackageName() const { return packageName_.c_str(); }
        hUint32                 getPackageCRC() const { return packageName_.hash(); }
        void                    beginLoad() { packageState_=State_Load_PkgDesc; }
        void                    unload();
        hBool                   update();
        hBool                   isInReadyState() const { return packageState_ == State_Ready; }
        void                    printResourceInfo();
        const hChar*            getPackageStateString() const;

    private:

        static const hUint32    MAX_PACKAGE_NAME = 128;

        struct hResourceLoadJobInputOutput
        {
            hByte*              resMemStart_;
            hByte*              resMemEnd_;
            void*               createdResource_;
            hStringID           resourceID_;
            hStringID           resourceType_;
        };

        typedef hVector< hStringID > PkgLinkArray;
        typedef std::vector< hResourceLoadJobInputOutput > ResourceJobArray;

        // Jobs
        void  loadPackageDescription(void*, void*);
        void  loadResource(void* in, void* out);

        enum State
        {
            State_Load_PkgDesc,
            State_Load_WaitPkgDesc,
            State_Load_DepPkgs,
            State_Kick_ResourceLoads,
            State_Ready,
            State_Unload_Resources,
            State_Unload_DepPkg,
            State_Unloaded,
        };

        enum class PkgState {
            LoadPkgDesc,
            LoadingResources,
            RequestLinkedPkgs,
            ResourceLinking,
            Loaded,
        };

        hStringID                   packageName_;
        hChar                       packagePath_[MAX_PACKAGE_NAME];
        State                       packageState_;
        hIFileSystem*               fileSystem_;
        hUint                       nextResourceToLoad_;
        hUint                       totalResources_;
        hUint                       linkedResources_;
        hIFile*                     pkgFileHandle_;
        PkgLinkArray                packageLinks_;

        proto::PackageHeader        packageHeader_;
        ResourceJobArray            resourceJobArray_;
        hTimer                      timer_;
        hJobQueue*                  fileQueue_;
    };

}

#endif // HRESOURCEPACKAGE_H__
