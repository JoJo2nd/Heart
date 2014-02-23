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

    hResourcePackage::hResourcePackage(hHeartEngine* engine, hIFileSystem* fileSystem, const hResourceHandlerMap* handlerMap, hJobQueue* fileQueue, hJobQueue* workerQueue, const hChar* packageName)
        : packageState_(State_Unloaded)
        , engine_(engine)
        , handlerMap_(handlerMap)
        , fileSystem_(fileSystem)
        , tempHeap_("ResPackageTempHeap")
        , totalResources_(0)
        , packageHeap_(hNullptr)
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
        //hDELETE_SAFE(packageHeap_);
        hotSwapSignal_.Destroy();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::loadPackageDescription(void*, void*)
    {
        hChar zipname[MAX_PACKAGE_NAME+10];
        
        hStrCopy(zipname, MAX_PACKAGE_NAME, "data:/");
        hStrCat(zipname, MAX_PACKAGE_NAME, packageName_);
        hStrCopy(packageRoot_, MAX_PACKAGE_NAME, zipname);
        hStrCat(packageRoot_, MAX_PACKAGE_NAME, "/");
        hStrCat(zipname,MAX_PACKAGE_NAME,".PKG");

        packageCRC_ = hCRC32::StringCRC(packageName_);
        hcAssert(fileSystem_);

        hStrCopy(zipname, MAX_PACKAGE_NAME, "data:/");
        hStrCat(zipname, MAX_PACKAGE_NAME, packageName_);
        hStrCat(zipname,MAX_PACKAGE_NAME,"/DAT");

        pkgDescFile_ = fileSystem_->OpenFile(zipname, FILEMODE_READ);

        tempHeap_.create(0, hTrue);

        hChar* xmldata = (hChar*)hHeapMalloc("general", (hUint32)(pkgDescFile_->Length()+1));
        pkgDescFile_->Read(xmldata, (hUint32)pkgDescFile_->Length());
        xmldata[pkgDescFile_->Length()] = 0;

        if (!descXML_.ParseSafe<rapidxml::parse_default>(xmldata)) {
            return;
        }

        const hChar* memuse = hXMLGetter(&descXML_).FirstChild("package").FirstChild("memory").GetAttributeString("alloc", NULL);
        hUint32 sizeBytes = 0;
        if (memuse) {
            // Create stack allocator size
            if (hStrWildcardMatch("*MB",memuse)) {
                hFloat mbs;
                hScanf("%fMB", &mbs);
                sizeBytes = (hUint32)((mbs*(1024.f*1024.f*1024.f))+.5f);
            } else if (hStrWildcardMatch("*KB",memuse)) {
                hFloat kbs;
                hScanf("%fMB", &kbs);
                sizeBytes = (hUint32)((kbs*(1024.f*1024.f))+.5f);
            } else {
                sizeBytes = hAtoI(memuse);
            }
        }

        if (!packageHeap_) {
            packageHeap_=Heart::hFindMemoryHeapByName("general");// maybe resources?
        }

        links_.Resize(0);
        links_.Reserve(16);
        for (hXMLGetter i = hXMLGetter(&descXML_).FirstChild("package").FirstChild("packagelinks").FirstChild("link"); i.ToNode(); i = i.NextSibling()) {
            if (hStrCmp(i.GetAttributeString("name"), packageName_)) {
                links_.PushBack(i.GetAttributeString("name"));
            }
        }

        totalResources_ = 0;
        for (hXMLGetter i = hXMLGetter(&descXML_).FirstChild("package").FirstChild("resources").FirstChild("resource"); i.ToNode(); i = i.NextSibling()) {
            ++totalResources_;
        }

        fileSystem_->CloseFile(pkgDescFile_);
        pkgDescFile_=hNullptr;

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
                for (hUint i=0, n=links_.GetSize(); i<n && !hotSwapping_; ++i) {
                    manager->loadPackage(links_[i]);
                }
                packageState_ = State_Kick_ResourceLoads; 
            } break;
        case State_Load_WaitDeps: {
                hBool loaded=hTrue;
                for (hUint i=0, n=links_.GetSize(); i<n; ++i) {
                    loaded &= manager->getIsPackageLoaded(links_[i]);
                }
                if (loaded) {
                    packageState_ = State_Link_Resources;
                }
            } break;
        case State_Kick_ResourceLoads: {
                hXMLGetter currentResource_ = hXMLGetter(descXML_.first_node()).FirstChild("resources").FirstChild("resource");
                for (hUint i=0; currentResource_.ToNode(); currentResource_=currentResource_.NextSibling(), ++i) {
                    hJob* loadjob=hNEW(hJob)();
                    resourceJobArray_[i].resourceDesc_=currentResource_;
                    resourceJobArray_[i].createdResource_=hNullptr;
                    resourceJobArray_[i].crc=0;
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
                        resourceMap_.Insert(resourceJobArray_[i].crc, resourceJobArray_[i].createdResource_);
                    }
                    hcPrintf("Package %s: %u resources loaded in %f sec", packageName_, totalResources_, timer_.elapsedMilliSec()/1000.f);
                    packageState_=State_Link_Resources;
                }
            } break;
        case State_Link_Resources: {
                if (doPostLoadLink(manager)) {
                    tempHeap_.destroy();
                    hcPrintf("Package %s is Loaded & Linked", packageName_);
                    packageState_ = State_Ready;
                    hotSwapping_ = hFalse;
                    resourceFilewatch_ = hdBeginFilewatch(packageRoot_, hdFilewatchEvents_FileModified|hdFilewatchEvents_AddRemove, hFUNCTOR_BINDMEMBER(hdFilewatchEventCallback, hResourcePackage, resourceDirChange, this));
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
                for (hUint i=0, n=links_.GetSize(); i<n && !hotSwapping_; ++i) {
                    manager->unloadPackage(links_[i]);
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
        const hXMLGetter& resxml=jobinfo->resourceDesc_;
        if (resxml.GetAttributeString("name"))
        {
            hResourceType typehandler;
            hResourceClassBase* res = NULL;
            hUint32 crc = hCRC32::StringCRC(resxml.GetAttributeString("name"));
            jobinfo->crc=crc;

            hChar* binFilepath;
            hBuildResFilePath(binFilepath, packageName_, resxml.GetAttributeString("name"));
            hIFile* file=hNullptr;
            file=fileSystem_->OpenFile(binFilepath, FILEMODE_READ);

            if (file) {
                proto::ResourceHeader resheader;
                hResourceFileStream resourcefilestream(file);
                google::protobuf::io::CodedInputStream resourcestream(&resourcefilestream);

                google::protobuf::uint32 headersize;
                resourcestream.ReadVarint32(&headersize);
                auto limit=resourcestream.PushLimit(headersize);
                resheader.ParseFromCodedStream(&resourcestream);
                resourcestream.PopLimit(limit);
                file->Seek(resourcestream.CurrentPosition(), SEEKOFFSET_BEGIN);
                hResourceType restypecrc(hCRC32::StringCRC(resheader.type().c_str()));
                hResourceHandler* handler = handlerMap_->Find(restypecrc);
                hcAssertMsg(handler, "Couldn't file handler for data type 0x%X", handler->GetKey().typeCRC);
                typehandler=handler->GetKey();
                // Load the binary file
                hTimer timer;
                hUint64 offset=resourcestream.CurrentPosition();
                hResourceSection* sections=(hResourceSection*)hAlloca(sizeof(hResourceSection)*resheader.sections_size());
                for (hUint i=0, n=resheader.sections_size(); i<n; ++i) {
                    sections[i].sectionName_ = resheader.sections(i).sectionname().c_str();
                    sections[i].sectionSize_ = resheader.sections(i).size();
                    sections[i].memType_ = resheader.sections(i).type();
                    if (!file->getIsMemMapped() || sections[i].memType_!=proto::eResourceSection_Temp) {
                        sections[i].sectionData_ = hHeapMalloc("general", resheader.sections(i).size());
                        hUint read=file->Read(sections[i].sectionData_, (hUint32)sections[i].sectionSize_);
                        hcAssertMsg(read == sections[i].sectionSize_, "Failed to read resource section %s (expected size %u, got %u)", sections[i].sectionName_, sections[i].sectionSize_, read);
                    } else {
                        sections[i].sectionData_ = (hByte*)file->getMemoryMappedBase()+offset;
                        offset+=resheader.sections(i).size();
                    }
                }

                timer.reset();
                res = handler->loadProc_(sections, resheader.sections_size());
                timer.setPause(hTrue);
                for (hUint i=0, n=resheader.sections_size(); i<n; ++i) {
                    if (sections[i].memType_==proto::eResourceSection_Temp && !file->getIsMemMapped()) {
                        hFreeSafe(sections[i].sectionData_);
                    }
                }
                hcPrintf("Loaded %s (crc:0x%08X.0x%08X) in %f Secs on thread %p", binFilepath, GetKey(), crc, timer.elapsedMilliSec()/1000.0f, Device::GetCurrentThreadID());
            }

            if (file) {
                fileSystem_->CloseFile(file);
            }
            hcAssertMsg(res, "Binary resource failed to load. Possibly out of date or broken file data"
                "and so serialiser could not load the data correctly" 
                "This is a Fatal Error.");
            if (res)
            {
                res->setResourceID(hResourceID(GetKey(), crc));
                res->SetName(resxml.GetAttributeString("name"));
                res->SetType(typehandler);
                jobinfo->createdResource_=res;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::unloadResource(void* in, void* out) {
        hcAssert(in==out);
        hResourceLoadJobInputOutput* jobinfo=(hResourceLoadJobInputOutput*)in;
        hResourceClassBase* res=jobinfo->createdResource_;
        hResourceHandler* handler = handlerMap_->Find(res->GetType());
        handler->unloadProc_(res);
        jobinfo->createdResource_=hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourcePackage::doPostLoadLink(hResourceManager* manager)
    {
        hUint32 totallinked = 0;
        for (hResourceClassBase* res = resourceMap_.GetHead(); res; res = res->GetNext()) {
            hResourceHandler* handler = handlerMap_->Find(res->GetType());
            handler->postLoadProc_(manager, res);
            ++totallinked;
        }

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::doPreUnloadUnlink(hResourceManager* manager)
    {
        for (hResourceClassBase* res = resourceMap_.GetHead(); res; res = res->GetNext()) {
            hResourceHandler* handler = handlerMap_->Find(res->GetType());
            handler->preUnloadProc_(manager, res);
        }
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
        for (hResourceClassBase* res = resourceMap_.GetHead(), *next = NULL; res; res = res->GetNext()) {
            hcPrintf("  Resource %s:"
                " Type: 0x%08X | crc: 0x%08X", 
                res->GetName(), res->GetType().typeCRC, res->GetKey());
        }
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