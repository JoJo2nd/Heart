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
    hUint32 len = hStrLen(packagePack)+hStrLen(file)+2; \
    outpath = (hChar*)hAlloca(len); \
    hStrCopy(outpath, len, packagePack); \
    hStrCat(outpath, len, "/"); \
    hStrCat(outpath, len, file); \
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourcePackage::hResourcePackage(hHeartEngine* engine, hIFileSystem* fileSystem, const hResourceHandlerMap* handlerMap)
        : packageState_(State_Unloaded)
        , engine_(engine)
        , handlerMap_(handlerMap)
        , zipPackage_(NULL)
        , driveFileSystem_(fileSystem)
        , fileSystem_(fileSystem)
        , currentResource_(NULL)
        , tempHeap_("ResPackageTempHeap")
        , totalResources_(0)
        , packageHeap_(NULL)
    {
        hZeroMem(packageName_, sizeof(packageName_));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourcePackage::~hResourcePackage()
    {
        hDELETE_SAFE(zipPackage_);
        resourceMap_.Clear(hTrue);
        //hDELETE_SAFE(packageHeap_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hResourcePackage::LoadPackageDescription(const hChar* packname)
    {
        hChar zipname[MAX_PACKAGE_NAME+10];
        
        hStrCopy(packageName_, MAX_PACKAGE_NAME, packname);
        hStrCopy(zipname, MAX_PACKAGE_NAME, packname);
        hStrCat(zipname,MAX_PACKAGE_NAME,".PKG");

        packageCRC_ = hCRC32::StringCRC(packname);

        zipPackage_ = hNEW(hZipFileSystem)(zipname);

        if (zipPackage_->IsOpen())
        {
            fileSystem_ = zipPackage_;
        }
        else
        {
            hDELETE_SAFE(zipPackage_);
        }

        hcAssert(fileSystem_);

        hStrCopy(packageName_, MAX_PACKAGE_NAME, packname);
        hStrCopy(zipname, MAX_PACKAGE_NAME, packname);
        hStrCat(zipname,MAX_PACKAGE_NAME,"/DAT");

        hIFile* pakDesc = fileSystem_->OpenFile(zipname, FILEMODE_READ);

        if (!pakDesc)
        {
            return -1;
        }

        tempHeap_.create(0, hTrue);

        hChar* xmldata = (hChar*)hHeapMalloc("general", (hUint32)(pakDesc->Length()+1));

        pakDesc->Read(xmldata, (hUint32)pakDesc->Length());
        xmldata[pakDesc->Length()] = 0;

        if (!descXML_.ParseSafe<rapidxml::parse_default>(xmldata))
        {
            return -1;
        }

        const hChar* memuse = hXMLGetter(&descXML_).FirstChild("package").FirstChild("memory").GetAttributeString("alloc", NULL);
        hUint32 sizeBytes = 0;
        if (memuse)
        {
            // Create stack allocator size
            if (hStrWildcardMatch("*MB",memuse))
            {
                hFloat mbs;
                hScanf("%fMB", &mbs);
                sizeBytes = (hUint32)((mbs*(1024.f*1024.f*1024.f))+.5f);
            }
            else if (hStrWildcardMatch("*KB",memuse))
            {
                hFloat kbs;
                hScanf("%fMB", &kbs);
                sizeBytes = (hUint32)((kbs*(1024.f*1024.f))+.5f);
            }
            else
            {
                sizeBytes = hAtoI(memuse);
            }
        }

        if (!packageHeap_) {
            packageHeap_=Heart::hFindMemoryHeapByName("general");// maybe resources?
//             if (sizeBytes == 0)
//             {
//                 // Create base/normal allocator, useful for debug
//                 packageHeap_ = hNEW(hMemoryHeap)(packageName_);
//                 packageHeap_->create(0, hTrue);
//             }
//             else
//             {
//                 packageHeap_ = hNEW(hStackMemoryHeap)();
//                 packageHeap_->create(sizeBytes, hTrue);
//             }
        }

        links_.Resize(0);
        links_.Reserve(16);
        for (hXMLGetter i = hXMLGetter(&descXML_).FirstChild("package").FirstChild("packagelinks").FirstChild("link"); i.ToNode(); i = i.NextSibling())
        {
            if (hStrCmp(i.GetAttributeString("name"), packname)) {
                links_.PushBack(i.GetAttributeString("name"));
            }
        }

        currentResource_.SetNode(hXMLGetter(&descXML_).FirstChild("package").FirstChild("resources").FirstChild("resource").ToNode());

        loadedResources_ = 0;
        totalResources_ = 0;
        for (hXMLGetter i = hXMLGetter(&descXML_).FirstChild("package").FirstChild("resources").FirstChild("resource"); i.ToNode(); i = i.NextSibling())
        {
            ++totalResources_;
        }

        fileSystem_->CloseFile(pakDesc);

        //TODO: resource Package links/deps requests

        packageState_ = State_Load_DepPkgs;
        doReload_ = hFalse;

        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hResourcePackage::GetPackageDependancyCount() const
    {
        return links_.GetSize();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hChar* hResourcePackage::GetPackageDependancy( hUint32 i ) const
    {
        return links_[i];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourcePackage::Update(hResourceManager* manager)
    {
        hBool ret = hFalse;
        switch(packageState_)
        {
        case State_Load_DepPkgs: {
                for (hUint i=0, n=links_.GetSize(); i<n; ++i) {
                    manager->ltLoadPackage(links_[i]);
                }
                packageState_ = State_Load_WaitDeps; 
            } break;
        case State_Load_WaitDeps: {
                hBool loaded=hTrue;
                for (hUint i=0, n=links_.GetSize(); i<n; ++i) {
                    loaded &= manager->ltIsPackageLoaded(links_[i]);
                }
                if (loaded) {
                    hcPrintf("Package %s Beginning Load Resources", packageName_);
                    packageState_ = State_Load_Reources;
                }
            } break;
        case State_Load_Reources: {
                LoadResourcesState();
            } break;
        case State_Link_Resources: {
                if (DoPostLoadLink())
                {
                    tempHeap_.destroy();
                    hcPrintf("Package %s is Loaded & Linked", packageName_);
                    packageState_ = State_Ready;
                    ret = hTrue;
                }
            } break;
        case State_Unlink_Resoruces: {
                DoPreUnloadUnlink();
                hcPrintf("Package %s Unload started", packageName_);
                packageState_ = State_Unload_Resources;
            } break;
        case State_Unload_Resources: {
                DoUnload();
                hcPrintf("Package %s is Unloaded", packageName_);
                packageState_ = State_Unload_DepPkg;
            } break;
        case State_Unload_DepPkg: {
                for (hUint i=0, n=links_.GetSize(); i<n; ++i) {
                    manager->ltUnloadPackage(links_[i]);
                }
                if (doReload_) {
                    hcPrintf("Package %s is Reloading", packageName_);
                    LoadPackageDescription(packageName_);
                } else {
                    packageState_ = State_Unloaded;
                }
            } break;
        case State_Unloaded:
        case State_Ready:
        default:{
            } break;
        }

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::Unload()
    {
        hcPrintf("Package %s Unlink started", packageName_);
        packageState_ = State_Unlink_Resoruces;
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::LoadResourcesState()
    {
        hResourceMemAlloc memAlloc = { packageHeap_, &tempHeap_ };
        while (currentResource_.ToNode())
        {
            //hIFile* file, const hChar* packagePath, const hChar* resName, const hChar* resourcePath, hUint32 parameterHash
            if (currentResource_.GetAttributeString("name"))
            {
                hResourceType typehandler;
                hResourceClassBase* res = NULL;
                hUint32 crc = hCRC32::StringCRC(currentResource_.GetAttributeString("name"));

                hChar* binFilepath;
                hBuildResFilePath(binFilepath, packageName_, currentResource_.GetAttributeString("name"));
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
                    hcPrintf("Loading %s (crc:0x%08X.0x%08X)", binFilepath, GetKey(), crc);
                    hResourceSection* sections=(hResourceSection*)hAlloca(sizeof(hResourceSection)*resheader.sections_size());
                    for (hUint i=0, n=resheader.sections_size(); i<n; ++i) {
                        sections[i].sectionData_ = hHeapMalloc("general", resheader.sections(i).size());
                        sections[i].sectionName_ = resheader.sections(i).sectionname().c_str();
                        sections[i].sectionSize_ = resheader.sections(i).size();
                        sections[i].memType_ = resheader.sections(i).type();
                        hUint read=file->Read(sections[i].sectionData_, (hUint32)sections[i].sectionSize_);
                        hcAssertMsg(read == sections[i].sectionSize_, "Failed to read resource section %s (expected size %u, got %u)",
                            sections[i].sectionName_, sections[i].sectionSize_, read);
                    }
                    
                    hClock::BeginTimer(timer);
                    res = handler->loadProc_(sections, resheader.sections_size());
                    hClock::EndTimer(timer);
                    for (hUint i=0, n=resheader.sections_size(); i<n; ++i) {
                        if (sections[i].memType_==proto::eResourceSection_Temp) {
                            hFreeSafe(sections[i].sectionData_);
                        }
                    }
                    hcPrintf("Load Time = %f Secs", timer.ElapsedMS()/1000.0f);
                }

                if (file) {
                    fileSystem_->CloseFile(file);
                }
                hcAssertMsg(res, "Binary resource failed to load. Possibly out of date or broken file data"
                                 "and so serialiser could not load the data correctly" 
                                 "This is a Fatal Error.");
                if (res)
                {
                    res->SetName(currentResource_.GetAttributeString("name"));
                    res->SetType(typehandler);
                    resourceMap_.Insert(crc, res);
                    ++loadedResources_;
                    currentResource_ = currentResource_.NextSibling();
                }
            }
        }
        //else
        {
            //finished
            hcPrintf("Package %s Beginning Link Resources", packageName_);
            packageState_ = State_Link_Resources;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourcePackage::DoPostLoadLink()
    {
        hUint32 totallinked = 0;
        for (hResourceClassBase* res = resourceMap_.GetHead(); res; res = res->GetNext())
        {
            hResourceHandler* handler = handlerMap_->Find(res->GetType());
            if (!res->GetIsLinked())
            {
                if (handler->linkProc_(res))
                {
                    hcPrintf("Resource 0x%08X.0x%08X is linked", GetKey(), res->GetKey());
                    res->SetIsLinked(hTrue);
                }
            }

            totallinked = res->GetIsLinked() ? totallinked + 1 : totallinked;
        }

        return totallinked == totalResources_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::DoPreUnloadUnlink()
    {
        for (hResourceClassBase* res = resourceMap_.GetHead(); res; res = res->GetNext())
        {
            hResourceHandler* handler = handlerMap_->Find(res->GetType());
            handler->unlinkProc_(res);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::DoUnload()
    {
        for (hResourceClassBase* res = resourceMap_.GetHead(), *next = NULL; res;)
        {
            hResourceHandler* handler = handlerMap_->Find(res->GetType());
            resourceMap_.Erase(res, &next);
            handler->unloadProc_(res);
            res = next;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourcePackage::GetResource( hUint32 crc ) const
    {
        hResourceClassBase* res = resourceMap_.Find(crc);
        if (res && res->GetIsLinked())
        {
            return res;
        }
        return NULL;
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackage::printResourceInfo()
    {
        for (hResourceClassBase* res = resourceMap_.GetHead(), *next = NULL; res; res = res->GetNext()) {
            hcPrintf("  Resource %s:"
                " Type: 0x%08X | Linked: %s | crc: 0x%08X", 
                res->GetName(), res->GetType().typeCRC, res->GetIsLinked() ? "Yes" : " No", res->GetKey());
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hChar* hResourcePackage::getPackageStateString() const
    {
        static const hChar* stateStr [] = {
            "State_Load_DepPkgs"     ,
            "State_Load_WaitDeps"    ,
            "State_Load_Reources"    ,
            "State_Link_Resources"   ,
            "State_Ready"            ,
            "State_Unlink_Resoruces" ,
            "State_Unload_Resources" ,
            "State_Unload_DepPkg"    ,
            "State_Unloaded"         ,
        };
        return stateStr[packageState_];
    }

#undef hBuildResFilePath
}