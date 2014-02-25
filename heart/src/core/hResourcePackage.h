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

    class hResourceFileStream : public google::protobuf::io::ZeroCopyInputStream
    {
    public:
        hResourceFileStream(hIFile* file) 
            : file_(file)
            , pos_(0)
            , bytesRead_(0)
        {
            bytes_=hNEW_ARRAY(hByte, s_bufferSize);
        }
        ~hResourceFileStream() {
            hDELETE_ARRAY_SAFE(bytes_);
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
        hUint32                          sectionSize_;
        void*                            sectionData_;
        proto::ResourceSectionMemoryType memType_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

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

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    class hResourcePackage : public hMapElement< hUint32, hResourcePackage >
                           , public hIReferenceCounted
    {
    public:
        hResourcePackage(hHeartEngine* engine, hIFileSystem* filesystem, const hResourceHandlerMap* handlerMap, hJobQueue* fileQueue, hJobQueue* workerQueue, const hChar* packageName);
        ~hResourcePackage();

        const hChar*            getPackageName() const { return packageName_; }
        hUint32                 getPackageCRC() const { return packageCRC_; }
        hResourceClassBase*     getResource(hUint32 crc) const;
        void                    beginLoad() { packageState_=State_Load_PkgDesc; }
        hBool                   update(hResourceManager* manager);
        void                    beginUnload();
        hBool                   isInReadyState() const { return packageState_ == State_Ready; }
        hBool                   isUnloading() const { return packageState_ > State_Ready; }
        hBool                   unloaded() const { return packageState_ == State_Unloaded; }
        void                    printResourceInfo();
        const hChar*            getPackageStateString() const;
        hUint                   getLinkCount() const { return links_.size(); }
        const hChar*            getLink(hUint i) const { return links_[i]; }

    private:

        static const hUint32    MAX_PACKAGE_NAME = 128;

        struct hResourceLoadJobInputOutput
        {
            hXMLGetter  resourceDesc_;
            hResourceClassBase* createdResource_;
            hUint32 crc;
        };

        typedef hVector< const hChar* > PkgLinkArray;
        typedef std::vector< hResourceLoadJobInputOutput > ResourceJobArray;
        typedef hMap< hUint32, hResourceClassBase > ResourceMap;
        typedef std::deque<hUint32> hHotSwapQueue;

        // Jobs
        void                        loadPackageDescription(void*, void*);
        void                        loadResource(void* in, void* out);
        void                        unloadResource(void* in, void* out);
        hBool                       doPostLoadLink(hResourceManager* manager);
        void                        doPreUnloadUnlink(hResourceManager* manager);
        void                        resourceDirChange(const hChar* watchDirectory, const hChar* filepath, hdFilewatchEvents fileevent);

        enum State
        {
            State_Load_PkgDesc,
            State_Load_WaitPkgDesc,
            State_Load_DepPkgs,
            State_Load_WaitDeps,
            State_Kick_ResourceLoads,
            State_Wait_ReourcesLoads,
            State_Link_Resources,
            State_Ready,
            State_Unlink_Resoruces,
            State_Unload_Resources,
            State_Wait_Unload_Resources,
            State_Unload_DepPkg,
            State_Unloaded,
        };

        hChar                       packageName_[MAX_PACKAGE_NAME];
        hChar                       packageRoot_[MAX_PACKAGE_NAME];
        hUint32                     packageCRC_;
        State                       packageState_;
        hHeartEngine*               engine_;
        const hResourceHandlerMap*  handlerMap_;
        hMemoryHeapBase*            packageHeap_;
        hMemoryHeap                 tempHeap_;
        hIFileSystem*               fileSystem_;
        hUint32                     totalResources_;
        hIFile*                     pkgDescFile_;
        hXMLDocument                descXML_;
        PkgLinkArray                links_;
        ResourceMap                 resourceMap_;
        ResourceJobArray            resourceJobArray_;
        hTimer                      timer_;
        hJobQueue*                  fileQueue_;
        hJobQueue*                  workerQueue_;
        hdFilewatchHandle           resourceFilewatch_;
        hSemaphore                  hotSwapSignal_;
        hBool                       hotSwapping_;
    };

}

#endif // HRESOURCEPACKAGE_H__
