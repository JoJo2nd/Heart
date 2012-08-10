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

    hUint32 hResourcePackage::AddResourceToPackage(const hChar* resourcePath, hResourceManager* resourceManager)
    {
        hUint32 ret = resourceDests_.GetSize();

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hResourcePackage::IsPackageLoaded()
    {
        return false;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourcePackageV2::hResourcePackageV2(HeartEngine* engine, hIFileSystem* fileSystem, const hResourceHandlerMap* handlerMap)
        : packageState_(State_Unloaded)
        , engine_(engine)
        , handlerMap_(handlerMap)
        , driveFileSystem_(fileSystem)
        , fileSystem_(fileSystem)
        , currentResource_(NULL)
    {
        hZeroMem(packageName_, sizeof(packageName_));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourcePackageV2::~hResourcePackageV2()
    {
        
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

        hChar* xmldata = (hChar*)hHeapMalloc(GetGlobalHeap(), pakDesc->Length()+1);

        pakDesc->Read(xmldata, pakDesc->Length());
        xmldata[pakDesc->Length()] = 0;

        if (!descXML_.ParseSafe<rapidxml::parse_default>(xmldata, GetGlobalHeap()))
        {
            return -1;
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
                hBuiltDataCache dataCache(
                    fileSystem_,
                    packageName_,
                    currentResource_.GetAttributeString("name"),
                    currentResource_.GetAttributeString("input"),
                    paramSet.GetParameterHash(),
                    hd_GetSharedLibTimestamp(handler->loaderLib_));
                hResourceClassBase* res = NULL;
                hUint32 crc = hCRC32::StringCRC(currentResource_.GetAttributeString("name"));
                hChar* binFilepath;
                hBuildResFilePath(binFilepath, packageName_, currentResource_.GetAttributeString("name"));
                hSerialiserFileStream inStream;
                inStream.Open(binFilepath, hFalse, fileSystem_);

                if (dataCache.IsCacheValid() && inStream.IsOpen())
                {
                    // Load the binary file
                    hcPrintf("Loading %s", binFilepath);
                    res = (*handler->binLoader_)(&inStream, &paramSet, engine_);
                    inStream.Close();
                }
                else
                {
                    // Open the raw source file
                    if (inStream.IsOpen())
                    {
                        inStream.Close();
                    }
                    while (!res)
                    {
                        hSerialiserFileStream outStream;
                        outStream.Open(binFilepath, hTrue, fileSystem_);
                        hIDataCacheFile* infile = dataCache.OpenFile(currentResource_.GetAttributeString("input"));
                        if (infile && outStream.IsOpen())
                        {
                            hcPrintf("Building %s", binFilepath);
                            res = (*handler->rawLoader_)(infile, &dataCache, &paramSet, engine_, &outStream);
                        }

                        if (infile)
                            dataCache.CloseFile(infile);
                        if (outStream.IsOpen())
                            outStream.Close();
                        if (!res)
                        {
                            //Wait ~5 Secs before retrying
                            Device::ThreadSleep(5000);
                            hcPrintf("Failed Building %s. Retrying", binFilepath);
                        }
                    }
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
        hUint32 totallinked = 0;
        for (hResourceClassBase* res = resourceMap_.GetHead(); res; res = res->GetNext())
        {
            hResourceHandler* handler = handlerMap_->Find(res->GetType());
            if (!res->GetIsLinked())
            {
                if ((*handler->packageLink_)(res, engine_))
                {
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
        for (hResourceClassBase* res = resourceMap_.GetHead(); res; res = res->GetNext())
        {
            hResourceHandler* handler = handlerMap_->Find(res->GetType());
            (*handler->packageUnlink_)(res, engine_);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hResourcePackageV2::DoUnload()
    {
        for (hResourceClassBase* res = resourceMap_.GetHead(), *next = NULL; res;)
        {
            hResourceHandler* handler = handlerMap_->Find(res->GetType());
            resourceMap_.Erase(res, &next);
            (*handler->resourceDataUnload_)(res, engine_);
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