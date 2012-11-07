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

    hResourcePackageV2::hResourcePackageV2(hHeartEngine* engine, hIFileSystem* fileSystem, const hResourceHandlerMap* handlerMap)
        : packageState_(State_Unloaded)
        , engine_(engine)
        , handlerMap_(handlerMap)
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

    hResourcePackageV2::~hResourcePackageV2()
    {
        hDELETE_SAFE(GetGlobalHeap(), packageHeap_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hResourcePackageV2::LoadPackageDescription(const hChar* packname)
    {
        hChar zipname[MAX_PACKAGE_NAME+10];
        
        hStrCopy(packageName_, MAX_PACKAGE_NAME, packname);
        hStrCopy(zipname, MAX_PACKAGE_NAME, packname);
        hStrCat(zipname,MAX_PACKAGE_NAME,".PKG");

        packageCRC_ = hCRC32::StringCRC(packname);

        zipPackage_ = hNEW(GetGlobalHeap(), hZipFileSystem)(zipname);

        if (zipPackage_->IsOpen())
        {
            fileSystem_ = zipPackage_;
        }
        else
        {
            hDELETE_SAFE(GetGlobalHeap(), zipPackage_);
        }

        hcAssert(fileSystem_);

        hStrCopy(packageName_, MAX_PACKAGE_NAME, packname);
        hStrCopy(zipname, MAX_PACKAGE_NAME, packname);
        hStrCat(zipname,MAX_PACKAGE_NAME,"/PKG.XML");

        hIFile* pakDesc = fileSystem_->OpenFile(zipname, FILEMODE_READ);

        if (!pakDesc)
        {
            return -1;
        }

        tempHeap_.create(0, hTrue);

        hChar* xmldata = (hChar*)hHeapMalloc(GetGlobalHeap(), pakDesc->Length()+1);

        pakDesc->Read(xmldata, pakDesc->Length());
        xmldata[pakDesc->Length()] = 0;

        if (!descXML_.ParseSafe<rapidxml::parse_default>(xmldata, GetGlobalHeap()))
        {
            return -1;
        }

        const hChar* memuse = hXMLGetter(&descXML_).FirstChild("memory").GetAttributeString("alloc", NULL);
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

        if (sizeBytes == 0)
        {
            // Create base/normal allocator, useful for debug
            packageHeap_ = hNEW(GetGlobalHeap(), hMemoryHeap)(packageName_);
            packageHeap_->create(0, hTrue);
        }
        else
        {
            packageHeap_ = hNEW(GetGlobalHeap(), hStackMemoryHeap)(GetGlobalHeap());
            packageHeap_->create(sizeBytes, hTrue);
        }

        links_.Reserve(16);
        for (hXMLGetter i = hXMLGetter(descXML_.first_node("packagelinks")).FirstChild("link"); i.ToNode(); i = i.NextSibling())
        {
            links_.PushBack(i.ToNode()->value());
        }

        currentResource_.SetNode(hXMLGetter(descXML_.first_node("resources")).FirstChild("resource").ToNode());

        loadedResources_ = 0;
        for (hXMLGetter i = hXMLGetter(descXML_.first_node("resources")).FirstChild("resource"); i.ToNode(); i = i.NextSibling())
        {
            ++totalResources_;
        }

        fileSystem_->CloseFile(pakDesc);

        //TODO: resource Package links/deps requests

        packageState_ = State_Load_WaitDeps;

        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hResourcePackageV2::GetPackageDependancyCount() const
    {
        return links_.GetSize();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hChar* hResourcePackageV2::GetPackageDependancy( hUint32 i ) const
    {
        return links_[i];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourcePackageV2::Update()
    {
        hBool ret = hFalse;
        switch(packageState_)
        {
        case State_Load_WaitDeps:
            {
                //TODO:
                packageState_ = State_Load_Reources;
                break;
            }
        case State_Load_Reources:
            {
                LoadResourcesState();
                break;
            }
        case State_Link_Resources:
            {
                if (DoPostLoadLink())
                {
                    tempHeap_.destroy();
                    packageState_ = State_Ready;
                    ret = hTrue;
                }
                break;
            }
        case State_Unlink_Resoruces:
            {
                DoPreUnloadUnlink();
                packageState_ = State_Unload_Resources;
                break;
            }
        case State_Unload_Resources:
            {
                DoUnload();
                packageState_ = State_Unloaded;
                break;
            }
        case State_Unloaded:
        case State_Ready:
        default:
            {
                break;
            }
        }

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackageV2::Unload()
    {
        packageState_ = State_Unlink_Resoruces;
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackageV2::LoadResourcesState()
    {
        hResourceMemAlloc memAlloc = { packageHeap_, &tempHeap_ };
        if (currentResource_.ToNode())
        {
            //hIFile* file, const hChar* packagePath, const hChar* resName, const hChar* resourcePath, hUint32 parameterHash
            if (currentResource_.GetAttributeString("name") &&
                currentResource_.GetAttributeString("input") && 
                currentResource_.GetAttributeString("type"))
            {
                hResourceType typehandler;
                hStrCopy(typehandler.ext, 4, currentResource_.GetAttributeString("type"));
                hResourceHandler* handler = handlerMap_->Find(typehandler);
                hDataParameterSet paramSet(&currentResource_);
#ifdef HEART_ALLOW_DATA_COMPILE
                hBuiltDataCache dataCache(
                    fileSystem_,
                    packageName_,
                    currentResource_.GetAttributeString("name"),
                    currentResource_.GetAttributeString("input"),
                    paramSet.GetParameterHash(),
                    hd_GetSharedLibTimestamp(handler->loaderLib_));
#endif//HEART_ALLOW_DATA_COMPILE
                hResourceClassBase* res = NULL;
                hUint32 crc = hCRC32::StringCRC(currentResource_.GetAttributeString("name"));

                hChar* binFilepath;
                hBuildResFilePath(binFilepath, packageName_, currentResource_.GetAttributeString("name"));
#ifdef HEART_ALLOW_DATA_COMPILE
                if (!dataCache.IsCacheValid())
                {
                    hBool success = hFalse;
                    while (!success)
                    {
                        hSerialiserFileStream outStream;
                        outStream.Open(binFilepath, hTrue, fileSystem_);
                        hIDataCacheFile* infile = dataCache.OpenFile(currentResource_.GetAttributeString("input"));
                        if (infile && outStream.IsOpen())
                        {
                            hTimer timer;
                            hcPrintf("Compiling Resource %s", binFilepath);
                            hClock::BeginTimer(timer);
                            success = (*handler->rawCompiler_)(infile, &dataCache, &paramSet, &memAlloc, engine_, &outStream);
                            hClock::EndTimer(timer);
                            if (success)
                            {
                                hcPrintf("Compile Time = %f Secs", timer.ElapsedMS()/1000.0f);
                            }
                        }

                        if (infile)
                            dataCache.CloseFile(infile);
                        if (outStream.IsOpen())
                            outStream.Close();

                        if (!success)
                        {
                            // Wait ~5 seconds and try again...
                            // TODO: Add a file watch?
                            Device::ThreadSleep(5000);
                        }
                    }
                }
#endif // HEART_ALLOW_DATA_COMPILE
                hSerialiserFileStream inStream;
                inStream.Open(binFilepath, hFalse, fileSystem_);

                if (inStream.IsOpen())
                {
                    // Load the binary file
                    hTimer timer;
                    hcPrintf("Loading %s(CRC:0x%08X.0x%08X)", binFilepath, GetKey(), crc);
                    hClock::BeginTimer(timer);
                    res = (*handler->binLoader_)(&inStream, &paramSet, &memAlloc, engine_);
                    hClock::EndTimer(timer);
                    inStream.Close();
                    hcPrintf("Load Time = %f Secs", timer.ElapsedMS()/1000.0f);
                }

                hcAssertMsg(res, "Binary resource failed to load. Possibly out of date or broken file data"
                                 "and so serialiser could not load the data correctly" 
                                 "This is a Fatal Error.");
                if (res)
                {
                    res->SetType(typehandler);
                    resourceMap_.Insert(crc, res);
                    ++loadedResources_;
                    currentResource_ = currentResource_.NextSibling();
                }
            }
        }
        else
        {
            //finished
            packageState_ = State_Link_Resources;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourcePackageV2::DoPostLoadLink()
    {
        hResourceMemAlloc memAlloc = { packageHeap_, &tempHeap_ };
        hUint32 totallinked = 0;
        for (hResourceClassBase* res = resourceMap_.GetHead(); res; res = res->GetNext())
        {
            hResourceHandler* handler = handlerMap_->Find(res->GetType());
            if (!res->GetIsLinked())
            {
                if ((*handler->packageLink_)(res, &memAlloc, engine_))
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

    void hResourcePackageV2::DoPreUnloadUnlink()
    {
        hResourceMemAlloc memAlloc = { packageHeap_, NULL };

        for (hResourceClassBase* res = resourceMap_.GetHead(); res; res = res->GetNext())
        {
            hResourceHandler* handler = handlerMap_->Find(res->GetType());
            (*handler->packageUnlink_)(res, &memAlloc, engine_);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackageV2::DoUnload()
    {
        hResourceMemAlloc memAlloc = { packageHeap_, NULL };
        for (hResourceClassBase* res = resourceMap_.GetHead(), *next = NULL; res;)
        {
            hResourceHandler* handler = handlerMap_->Find(res->GetType());
            resourceMap_.Erase(res, &next);
            (*handler->resourceDataUnload_)(res, &memAlloc, engine_);
            res = next;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hResourcePackageV2::GetResource( hUint32 crc ) const
    {
        hResourceClassBase* res = resourceMap_.Find(crc);
        if (res && res->GetIsLinked())
        {
            return res;
        }
        return NULL;
    }

#undef hBuildResFilePath
}