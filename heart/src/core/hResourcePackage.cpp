/********************************************************************

    filename: 	hResourcePackage.cpp	
    
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

    hResourcePackage::hResourcePackage(hIFileSystem* fileSystem, hJobQueue* fileQueue, hJobQueue* workerQueue, const hChar* packageName)
        : packageState_(State_Unloaded)
        , fileSystem_(fileSystem)
        , totalResources_(0)
        , fileQueue_(fileQueue)
        , workerQueue_(workerQueue)
        , hotSwapping_(hFalse)
    {
        hotSwapSignal_.Create(0, 4096);
        hStrCopy(packageName_, (hUint)hArraySize(packageName_), packageName);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourcePackage::~hResourcePackage()
    {
        resourceMap_.Clear(hTrue);
        hotSwapSignal_.Destroy();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::loadPackageDescription(void*, void*)
    {
        hStrCopy(packagePath_, MAX_PACKAGE_NAME, "data:/");
        hStrCat (packagePath_, MAX_PACKAGE_NAME, packageName_);
        hStrCat (packagePath_, MAX_PACKAGE_NAME, ".pkg");

        packageCRC_ = hCRC32::StringCRC(packageName_);
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
            links_.push_back(hStringID(packageHeader_.packagedependencies(i).c_str()));
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

    hBool hResourcePackage::update(hResourceManager* manager) {
        hBool ret = hFalse;
        switch(packageState_) {
        case State_Load_PkgDesc: {
                hJob* descreadjob=hNEW(hJob)();
                descreadjob->setJobProc(hFUNCTOR_BINDMEMBER(hJobProc, hResourcePackage, loadPackageDescription, this));
                descreadjob->setWorkerMask(1);
                fileQueue_->pushJob(descreadjob);
                //the resource manager will kick the queue
                packageState_=State_Load_WaitPkgDesc;
            } break;
        case State_Load_WaitPkgDesc: {
                if (fileQueue_->queueIdle()) {
                    packageState_=State_Load_DepPkgs;
                }
            } break;
        case State_Load_DepPkgs: {
                for (hUint i=0, n=links_.size(); i<n && !hotSwapping_; ++i) {
                    //manager->loadPackage(links_[i].c_str());
                }
                packageState_ = State_Kick_ResourceLoads; 
            } break;
        case State_Load_WaitDeps: {
                hBool loaded=hTrue;
                for (hUint i=0, n=links_.size(); i<n; ++i) {
                    loaded &= manager->getIsPackageLoaded(links_[i].c_str());
                }
                if (loaded) {
                    packageState_ = State_Link_Resources;
                }
            } break;
        case State_Kick_ResourceLoads: {
                hcPrintf("Stub "__FUNCTION__);
                hcAssert(pkgFileHandle_->getIsMemMapped());
                hByte* file_base = (hByte*)pkgFileHandle_->getMemoryMappedBase();
                for (hInt i=0, n=packageHeader_.entries_size(); i<n; ++i) {
                    hJob* loadjob=hNEW(hJob)();
                    resourceJobArray_[i].resMemStart_= file_base+packageHeader_.entries(i).entryoffset();
                    resourceJobArray_[i].resMemEnd_= resourceJobArray_[i].resMemStart_+packageHeader_.entries(i).entrysize();
                    resourceJobArray_[i].createdResource_=nullptr;
                    resourceJobArray_[i].resourceID_=hStringID(packageHeader_.entries(i).entryname().c_str());
                    loadjob->setInput(&resourceJobArray_[i]);
                    loadjob->setOutput(&resourceJobArray_[i]);
                    loadjob->setJobProc(hFUNCTOR_BINDMEMBER(hJobProc, hResourcePackage, loadResource, this));
                    workerQueue_->pushJob(loadjob);
                }
                timer_.reset();
                packageState_=State_Wait_ReourcesLoads;
            } break;
        case State_Wait_ReourcesLoads: {
                if (workerQueue_->queueIdle()) {
                    for (hUint i=0, n=(hUint)resourceJobArray_.size(); i<n; ++i) {
                        hcPrintf("STUB "__FUNCTION__);
                        //resourceMap_.Insert(resourceJobArray_[i].crc, resourceJobArray_[i].createdResource_);
                        manager->insertResourceContainer(resourceJobArray_[i].resourceID_, resourceJobArray_[i].createdResource_, resourceJobArray_[i].resourceType_);
                    }
                    hcPrintf("Package %s: %u resources loaded in %f sec", packageName_, totalResources_, timer_.elapsedMilliSec()/1000.f);
                    packageState_=State_Link_Resources;
                }
            } break;
        case State_Link_Resources: {
                if (doPostLoadLink(manager)) {
                    hcPrintf("Package %s is Loaded & Linked", packageName_);
                    packageState_ = State_Ready;
                    hotSwapping_ = hFalse;
                    resourceFilewatch_ = hdBeginFilewatch(packagePath_, hdFilewatchEvents_FileModified|hdFilewatchEvents_AddRemove, hFUNCTOR_BINDMEMBER(hdFilewatchEventCallback, hResourcePackage, resourceDirChange, this));
                    ret = hTrue;
                }
            } break;
        case State_Unlink_Resoruces: {
                doPreUnloadUnlink(manager);
                hcPrintf("Package %s Unload started", packageName_);
                packageState_ = State_Unload_Resources;
            } break;
        case State_Unload_Resources: {
                resourceMap_.Clear(hFalse);
                for (hUint i=0, n=(hUint)resourceJobArray_.size(); i<n; ++i) {
                    hJob* loadjob=hNEW(hJob)();
                    loadjob->setInput(&resourceJobArray_[i]);
                    loadjob->setOutput(&resourceJobArray_[i]);
                    loadjob->setJobProc(hFUNCTOR_BINDMEMBER(hJobProc, hResourcePackage, unloadResource, this));
                    workerQueue_->pushJob(loadjob);
                }
                packageState_ = State_Wait_Unload_Resources;
            } break;
        case State_Wait_Unload_Resources: {
                if (workerQueue_->queueIdle()) {
                    hcPrintf("Package %s is Unloaded", packageName_);
                    packageState_ = State_Unload_DepPkg;
                }
            } break;
        case State_Unload_DepPkg: {
                for (hUint i=0, n=links_.size(); i<n && !hotSwapping_; ++i) {
                    manager->unloadPackage(links_[i].c_str());
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
                {
                    if (hotSwapSignal_.poll()) {
                        beginUnload();
                        hotSwapping_=hTrue;
                    }
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
            hcPrintf("Package %s Unlink started", packageName_);
            packageState_ = State_Unlink_Resoruces;
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
#if 0
        hResourceType restypecrc(hCRC32::StringCRC(jobinfo->type_));
        hResourceHandler* handler = handlerMap_->Find(restypecrc);
        hcAssertMsg(handler, "Couldn't file handler for data type 0x%X", handler->GetKey().typeCRC);
        typehandler=handler->GetKey();
        // Load the binary file
        hTimer timer;
        hResourceSection sections;
        sections.sectionName_ = "any";
        sections.sectionSize_ = (hInt)data_container.messagedata().size();
        sections.memType_ = proto::eResourceSection_Temp;
        sections.sectionData_ = data_container.mutable_messagedata()->c_str();

        timer.reset();
        res = handler->loadProc_(&sections, 1);
        timer.setPause(hTrue);
        // hcPrintf("Loaded %s (crc:0x%08X.0x%08X) in %f Secs on thread %p", binFilepath, GetKey(), crc, timer.elapsedMilliSec()/1000.0f, Device::GetCurrentThreadID());
        hcAssertMsg(res, "Binary resource failed to load. Possibly out of date or broken file data"
            "and so serialiser could not load the data correctly" 
            "This is a Fatal Error.");
        if (res)
        {
            hcPrintf("Stub "__FUNCTION__);
            res->setResourceID(hResourceID(GetKey(), crc));
            res->SetName(resxml.GetAttributeString("name"));
            res->SetType(typehandler);
            jobinfo->createdResource_=res;
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::unloadResource(void* in, void* out) {
        hcAssert(in==out);
        hcPrintf("Stub "__FUNCTION__);
//         hResourceLoadJobInputOutput* jobinfo=(hResourceLoadJobInputOutput*)in;
//         void* res=jobinfo->createdResource_;
//         hResourceHandler* handler = handlerMap_->Find(res->GetType());
//         handler->unloadProc_(res);
//         jobinfo->createdResource_=hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourcePackage::doPostLoadLink(hResourceManager* manager)
    {
#if 0
        hUint32 totallinked = 0;
        for (hResourceClassBase* res = resourceMap_.GetHead(); res; res = res->GetNext()) {
            hResourceHandler* handler = handlerMap_->Find(res->GetType());
            handler->postLoadProc_(manager, res);
            ++totallinked;
        }
#else
        hcPrintf("Stub "__FUNCTION__);
#endif
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::doPreUnloadUnlink(hResourceManager* manager)
    {
#if 0
        for (hResourceClassBase* res = resourceMap_.GetHead(); res; res = res->GetNext()) {
            hResourceHandler* handler = handlerMap_->Find(res->GetType());
            handler->preUnloadProc_(manager, res);
        }
#else
        hcPrintf("Stub "__FUNCTION__);
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourcePackage::getResource( hUint32 crc ) const
    {
        return resourceMap_.Find(crc);;
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