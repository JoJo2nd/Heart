/********************************************************************
 
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

namespace Heart
{
hRegisterObjectType(package, Heart::hResourcePackage, Heart::proto::PackageHeader);

#define hBuildResFilePath(outpath, packagePack, file) \
    { \
    hUint32 len = hStrLen(packagePack)+hStrLen(file)+2+6; \
    outpath = (hChar*)hAlloca(len); \
    hStrCopy(outpath, len, "data:/"); \
    hStrCat(outpath, len, packagePack); \
    hStrCat(outpath, len, "/"); \
    hStrCat(outpath, len, file); \
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourcePackage::hResourcePackage()
        : packageState_(State_Unloaded)
        , totalResources_(0)
        , hotSwapping_(hFalse)
    {
        hotSwapSignal_.Create(0, 4096);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourcePackage::~hResourcePackage()
    {
        hotSwapSignal_.Destroy();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::initialise(hIFileSystem* filesystem, hJobQueue* fileQueue, hJobQueue* workerQueue, const hChar* packageName) {
        fileSystem_ = filesystem;
        fileQueue_ = fileQueue;
        workerQueue_ = workerQueue;
        packageName_ = hStringID(packageName);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    hBool hResourcePackage::serialiseObject(Heart::proto::PackageHeader* ) const {
        hcAssertMsg(false, "Not expected to call this function");
        return hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourcePackage::deserialiseObject(Heart::proto::PackageHeader* ) {
        hcAssertMsg(false, "Not expected to call this function");
        return hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::loadPackageDescription(void*, void*)
    {
        hStrCopy(packagePath_, MAX_PACKAGE_NAME, "data:/");
        hStrCat (packagePath_, MAX_PACKAGE_NAME, packageName_.c_str());
        hStrCat (packagePath_, MAX_PACKAGE_NAME, ".pkg");

        hcAssert(fileSystem_);
        pkgFileHandle_ = fileSystem_->OpenFile(packagePath_, FILEMODE_READ);
        hResourceFileStream resourcefilestream(pkgFileHandle_);
        google::protobuf::io::CodedInputStream resourcestream(&resourcefilestream);

        google::protobuf::uint32 headersize;
        resourcestream.ReadVarint32(&headersize);
        headersize += resourcestream.CurrentPosition();
        auto limit=resourcestream.PushLimit(headersize);
        packageHeader_.ParseFromCodedStream(&resourcestream);
        resourcestream.PopLimit(limit);

        for (hInt i=0, n=packageHeader_.packagedependencies_size(); i<n; ++i) {
            packageLinks_.push_back(hStringID(packageHeader_.packagedependencies(i).c_str()));
        }
        for (hInt i=0, n=packageHeader_.entries_size(); i<n; ++i) {
            packageHeader_.mutable_entries(i)->set_entryoffset(packageHeader_.entries(i).entryoffset()+headersize);
        }
        totalResources_ = packageHeader_.entries_size();

        resourceJobArray_.resize(totalResources_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourcePackage::update() {
        hBool ret = hFalse;
        switch(packageState_) {
        case State_Load_PkgDesc: {
                hJob* descreadjob=hNEW(hJob)();
                descreadjob->setJobProc(hFUNCTOR_BINDMEMBER(hJobProc, hResourcePackage, loadPackageDescription, this));
                descreadjob->setWorkerMask(1);
                fileQueue_->pushJob(descreadjob);
                //the resource manager will kick the queue
                packageState_=State_Load_WaitPkgDesc;
                nextResourceToLoad_ = 0;
                linkedResources_ = 0;
            } break;
        case State_Load_WaitPkgDesc: {
                if (fileQueue_->queueIdle()) {
                    //Now that we know about the package add ourselves to the resource manager
                    hResourceManager::resourceAddRef(packageName_);
                    hResourceManager::addResourceNode(packageName_);
                    for (hUint i=0, n=packageHeader_.entries_size(); i<n; ++i) {
                        hStringID link_id(packageHeader_.entries(i).entryname().c_str());
                        hResourceManager::addResourceLink(packageName_, &link_id, 1, 
                            hFUNCTOR_BINDMEMBER(hNewResourceEventProc, hResourcePackage, onLinkEvent, this));
                    }
                    //
                    packageState_=State_Load_DepPkgs;
                }
            } break;
        case State_Load_DepPkgs: {
                for (hUint i=0, n=packageLinks_.size(); i<n && !hotSwapping_; ++i) {
                    hResourceManager::loadPackage(packageLinks_[i].c_str());
                }
                packageState_ = State_Kick_ResourceLoads; 
            } break;
        case State_Kick_ResourceLoads: {
                hcPrintf("Stub "__FUNCTION__);
                hcAssert(pkgFileHandle_->getIsMemMapped());
                hByte* file_base = (hByte*)pkgFileHandle_->getMemoryMappedBase();
                hFloat start_time = hClock::elapsed();
                for (hInt i=nextResourceToLoad_, n=packageHeader_.entries_size(); i<n && (hClock::elapsed()-start_time) < 0.01; ++i, ++nextResourceToLoad_) {
                    hJob* loadjob=hNEW(hJob)();
                    resourceJobArray_[i].resMemStart_= file_base+packageHeader_.entries(i).entryoffset();
                    resourceJobArray_[i].resMemEnd_= resourceJobArray_[i].resMemStart_+packageHeader_.entries(i).entrysize();
                    resourceJobArray_[i].createdResource_=nullptr;
                    resourceJobArray_[i].resourceID_=hStringID(packageHeader_.entries(i).entryname().c_str());
                    loadResource(resourceJobArray_.data()+i, resourceJobArray_.data()+i);
//                     loadjob->setInput(&resourceJobArray_[i]);
//                     loadjob->setOutput(&resourceJobArray_[i]);
//                     loadjob->setJobProc(hFUNCTOR_BINDMEMBER(hJobProc, hResourcePackage, loadResource, this));
//                     workerQueue_->pushJob(loadjob);
                }
                timer_.reset();
                if (nextResourceToLoad_ == packageHeader_.entries_size()) {
                    hotSwapping_ = hFalse;
                    resourceFilewatch_ = hdBeginFilewatch(packagePath_, hdFilewatchEvents_FileModified|hdFilewatchEvents_AddRemove, hFUNCTOR_BINDMEMBER(hdFilewatchEventCallback, hResourcePackage, resourceDirChange, this));
                    packageState_=State_Ready;
                }
            } break;
        case State_Unload_Resources: {
                // mark ourselves as a non-root resource any more. Will allow the GC to work its magic
                hResourceManager::resourceDecRef(packageName_);
                hResourceManager::collectGarbage(0.f);
                packageState_ = State_Unload_DepPkg;
            } break;
        case State_Unload_DepPkg: {
                for (hUint i=0, n=packageLinks_.size(); i<n && !hotSwapping_; ++i) {
                    hResourceManager::unloadPackage(packageLinks_[i].c_str());
                }
                hdEndFilewatch(resourceFilewatch_);
                resourceFilewatch_=0;
                packageState_ = State_Unloaded;
            } break;
        case State_Unloaded: {
                if (hotSwapping_) {
                    hotSwapping_=hFalse;
                }
            } break;
        case State_Ready: {
                if (hotSwapSignal_.poll()) {
                    beginUnload();
                    hotSwapping_=hTrue;
                }
            } break;
        default:{
            } break;
        }

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::beginUnload()
    {
        if (isInReadyState()) {
            hotSwapping_=hFalse;
            hcPrintf("Package %s Unload started", packageName_);
            packageState_ = State_Unload_Resources;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::loadResource(void* in, void* out) {
        hcAssert(in==out);
        hResourceLoadJobInputOutput* jobinfo=(hResourceLoadJobInputOutput*)in;

        proto::ResourceHeader resheader;
        google::protobuf::io::ArrayInputStream resourcefilestream(jobinfo->resMemStart_, (hInt)((hPtrdiff_t)jobinfo->resMemEnd_-(hPtrdiff_t)jobinfo->resMemStart_));
        google::protobuf::io::CodedInputStream resourcestream(&resourcefilestream);

        proto::MessageContainer data_container;
        data_container.ParseFromCodedStream(&resourcestream);
        jobinfo->createdResource_ = hObjectFactory::deserialiseObject(&data_container, &jobinfo->resourceType_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourcePackage::onLinkEvent(hStringID res_id, hResurceEvent event_type, hStringID type_id, void* data_ptr) {
        if (event_type == hResurceEvent::hResourceEvent_DBInsert) {
            ++linkedResources_;
        } else if (event_type == hResurceEvent::hResourceEvent_DBRemove) {
            --linkedResources_;
        }
        if (linkedResources_ == totalResources_) {
            hResourceManager::insertResourceContainer(packageName_, this, getTypeName());
        }
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::printResourceInfo()
    {
#if 0
        for (hResourceClassBase* res = resourceMap_.GetHead(), *next = NULL; res; res = res->GetNext()) {
            hcPrintf("  Resource %s:"
                " Type: 0x%08X | crc: 0x%08X", 
                res->GetName(), res->GetType().typeCRC, res->GetKey());
        }
#else
        hcPrintf("Stub "__FUNCTION__);
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hChar* hResourcePackage::getPackageStateString() const
    {
        static const hChar* stateStr [] = {
            "State_Load_PkgDesc",
            "State_Load_WaitPkgDesc",
            "State_Load_DepPkgs",
            "State_Load_WaitDeps",
            "State_Kick_ResourceLoads",
            "State_Wait_ReourcesLoads",
            "State_Link_Resources",
            "State_Ready",
            "State_Unlink_Resoruces",
            "State_Unload_Resources",
            "State_Wait_Unload_Resources",
            "State_Unload_DepPkg",
            "State_Unloaded",
        };
        return stateStr[packageState_];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::resourceDirChange(const hChar* watchDirectory, const hChar* filepath, hdFilewatchEvents fileevent) {
        hotSwapSignal_.Post();
//         if (fileevent&(hdFilewatchEvents_Added|hdFilewatchEvents_Removed|hdFilewatchEvents_Rename)) {
//             // These options require an entire reload of the package
//             hcPrintf("Package %s needs Hot-Swapping", packageRoot_); // !! Not thread safe, but just testing
//         } else if (fileevent&hdFilewatchEvents_Modified) {
//             // This is simply a case of reloading a resource
//             hcPrintf("Resource %s needs Hot-Swapping", filepath);
//         }
    }

#undef hBuildResFilePath
}