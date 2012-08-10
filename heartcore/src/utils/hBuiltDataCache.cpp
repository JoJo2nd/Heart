/********************************************************************

	filename: 	hBuiltDataCache.cpp	
	
	Copyright (c) 21:7:2012 James Moran
	
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
#define hBuildFilePath(outpath,packagePack, path, file) \
    { \
    hUint32 len = hStrLen(packagePack_)+hStrLen(path)+hStrLen(file)+3; \
    outpath = (hChar*)hAlloca(len); \
    hStrCopy(outpath, len, packagePack); \
    hStrCat(outpath, len, "/"); \
    hStrCat(outpath, len, path); \
    hStrCat(outpath, len, "/"); \
    hStrCat(outpath, len, file); \
    }

#define hBuildResFilePath(outpath,packagePack, file) \
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

    hBuiltDataCache::hBuiltDataCache( 
            hIFileSystem* fileSystem, 
            const hChar* packagePath, 
            const hChar* resName, 
            const hChar* resourcePath, 
            hUint32 parameterHash,
            hTime libTimeStamp) 
        : parameterHash_(parameterHash)
        , libTimestamp_(libTimeStamp)
        , fileSystem_(fileSystem)
        , packagePack_(packagePath)
        , resName_(resName)
        , resourcePath_(resourcePath)
        , checkedCache_(hFalse)
        , validCache_(hFalse)
    {
        using namespace rapidxml;
        hChar* cacheDir;
        hBuildResFilePath(cacheDir, packagePack_, "CACHE");

        fileSystem_->CreateDirectory(cacheDir);

        LoadCacheData();

        if (IsCacheValid() == hFalse)
        {
            //Clean up xml doc...
            doc_.~hXMLDocument();
            hPLACEMENT_NEW(&doc_) hXMLDocument();

            hChar hashstr[32];
            hStrPrintf( hashstr, 32, "0x%08X", parameterHash_ );
            hChar* xmlstr = doc_.allocate_string(hashstr);
            xml_node<>* filenode = doc_.allocate_node(node_element, "parameterhash", xmlstr);
            doc_.append_node(filenode);

            hChar mdatestr[32];
            hTime mdate = libTimeStamp;
            hStrPrintf( mdatestr, 32, "0x%016llX", mdate );
            xmlstr = doc_.allocate_string(mdatestr);
            filenode = doc_.allocate_node(node_element, "builder", NULL);
            xml_attribute<>* timestamp = doc_.allocate_attribute("mdate", xmlstr);
            filenode->append_attribute(timestamp);
            doc_.append_node(filenode);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hIDataCacheFile* hBuiltDataCache::OpenFile( const hChar* filename )
    {
        hChar* filePath;
        hBuildResFilePath(filePath, packagePack_, filename);
        hIFile* file = fileSystem_->OpenFile(filePath, FILEMODE_READ);
        hDataCacheFile* retFile = NULL;

        if (file)
        {
            AppendFileTimestampToCache(file, filename);
            retFile = hNEW(GetGlobalHeap(), hDataCacheFile)(file);
        }
        
        return retFile;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hBuiltDataCache::CloseFile( hIDataCacheFile* file )
    {
        hDataCacheFile* dfile = static_cast<hDataCacheFile*>(file);
        fileSystem_->CloseFile(dfile->handle_);
        hDELETE(GetGlobalHeap(), file);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hBuiltDataCache::LoadCacheData()
    {
        hChar* filePath;
        hBuildFilePath(filePath, packagePack_, "CACHE", resName_);

        hIFile* file = fileSystem_->OpenFile(filePath, FILEMODE_READ);
        if (file)
        {   
            hUint32 xmllen = file->Length();
            hChar* xmldata = (hChar*)hHeapMalloc(GetGlobalHeap(), xmllen+1);
            file->Read(xmldata, xmllen+1);
            xmldata[xmllen] = 0;
            doc_.ParseSafe<rapidxml::parse_default>(xmldata, GetGlobalHeap());
            IsCacheValid();
            fileSystem_->CloseFile(file);
        }
        else
        {
            checkedCache_ = hTrue;
            validCache_ = fileSystem_->WriteableSystem() ? validCache_ : hTrue;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hBuiltDataCache::SaveCacheData()
    {
        if (!IsCacheValid())
        {
            static const hUint32 bufferSize = (1024*1024);
            //TODO: hope this never goes over 1MB, find a way to warn of overwrite...
            hChar* tmpbuffer = (hChar*)hHeapMalloc(GetGlobalHeap(), bufferSize);
            hChar* end = rapidxml::print(tmpbuffer, doc_);
            *end = 0;
            hcAssert(((hUint32)end - (hUint32)tmpbuffer) < bufferSize);

            hChar* filepath;
            hBuildFilePath(filepath, packagePack_, "CACHE", resName_);
            hIFile* file = fileSystem_->OpenFile(filepath, FILEMODE_WRITE);
            if (file)
            {
                file->Write(tmpbuffer, ((hUint32)end - (hUint32)tmpbuffer));
                fileSystem_->CloseFile(file);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hBuiltDataCache::IsCacheValid()
    {
        if (!checkedCache_)
        {
            checkedCache_ = hTrue;
            validCache_   = hTrue;

            hUint32 hash;
            hXMLGetter node(doc_.first_node("parameterhash"));
            if (node.ToNode()->value() == NULL || 
                sscanf(node.ToNode()->value(), "0x%08X", &hash ) != 1 ||
                hash != parameterHash_)
            {
                validCache_ = hFalse;
                return hFalse;
            }

            node.SetNode(doc_.first_node("builder"));
            {
                const hChar* mdatestr = node.GetAttributeString("mdate");
                hTime mdate;
                if (node.ToNode()->value())
                {
                    hIFile* file = fileSystem_->OpenFile(node.ToNode()->value(), FILEMODE_READ);
                    if (mdatestr == NULL ||
                        sscanf(mdatestr, "0x%016llX", &mdate) != 1 ||
                        mdate != libTimestamp_)
                    {
                        validCache_ = hFalse;
                    }
                    fileSystem_->CloseFile(file);
                }
                else
                {
                    validCache_ = hFalse;
                }
            }

            node.SetNode(doc_.first_node("file"));

            for (; node.ToNode() && validCache_; node = node.NextSibling())
            {
                const hChar* mdatestr = node.GetAttributeString("mdate");
                hTime mdate;
                if (node.ToNode()->value())
                {
                    hChar* inputFilepath;
                    hBuildResFilePath(inputFilepath, packagePack_, node.ToNode()->value());
                    hIFile* file = fileSystem_->OpenFile(inputFilepath, FILEMODE_READ);
                    if (mdatestr == NULL || file == NULL ||
                        sscanf(mdatestr, "0x%016llX", &mdate) != 1 ||
                        mdate != file->GetTimestamp())
                    {
                        validCache_ = hFalse;
                    }
                    if (file)
                    {
                        fileSystem_->CloseFile(file);
                    }
                }
                else
                {
                    validCache_ = hFalse;
                }
            }
        }

        return validCache_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hBuiltDataCache::AppendFileTimestampToCache( hIFile* file, const hChar* filepath )
    {
        using namespace rapidxml;

        if (IsCacheValid() == hTrue)
            return;

        hChar mdatestr[32];
        hTime mdate = file->GetTimestamp();
        hStrPrintf( mdatestr, 32, "0x%016llX", mdate );
        hChar* xmlstr = doc_.allocate_string(mdatestr);
        hChar* filepathstr = doc_.allocate_string(filepath);
        xml_node<>* filenode = doc_.allocate_node(node_element, "file", filepathstr);
        xml_attribute<>* timestamp = doc_.allocate_attribute("mdate", xmlstr);
        filenode->append_attribute(timestamp);
        doc_.append_node(filenode);
    }

#undef hBuildFilePath
#undef hBuildResFilePath
}