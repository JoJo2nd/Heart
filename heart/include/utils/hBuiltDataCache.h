/********************************************************************

	filename: 	hBuiltDataCache.h	
	
	Copyright (c) 16:7:2012 James Moran
	
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
#ifndef HBUILTDATACACHE_H__
#define HBUILTDATACACHE_H__

namespace Heart
{
    class HEART_DLLEXPORT hDataParameterSet : public hIDataParameterSet
    {
    public:
        hDataParameterSet(hXMLGetter* resourceRoot)
            : root_(resourceRoot)
        {
            name_ = root_->GetAttributeString("name");
            path_ = root_->GetAttributeString("input");
        }
        ~hDataParameterSet() 
        {}

        const hChar* GetResourceName() const { return name_; }
        const hChar* GetInputFilePath() const { return path_; }
        const hChar* GetBuildParameter(const hChar* name, const hChar* defaultValue) const
        {
            if (!root_->FirstChild(name).ToNode())
                return defaultValue;
            const hChar* ret = root_->FirstChild(name).ToNode()->value();
            return ret != NULL ? ret : defaultValue;
        }
        const hUint getBuildParameterCount(const hChar* name) const {
            hUint i=0;
            for (hXMLGetter xi(root_->FirstChild(name)); xi.ToNode(); xi=xi.NextSibling(), ++i) {
            }
            return i;
        }
        const hChar* getBuildParameter(const hChar* name, hUint idx, const hChar* defaultValue) const {
            hUint i=0;
            for (hXMLGetter xi(root_->FirstChild(name)); xi.ToNode(); xi=xi.NextSibling(), ++i) {
                if (i==idx) {
                    return xi.GetValueString(defaultValue);
                }
            }
            return defaultValue;
        }
        const hChar* getBuildParameterAttrib(const hChar* name, hUint idx, const hChar* attrib, const hChar* defaultValue) const {
            hUint i=0;
            for (hXMLGetter xi(root_->FirstChild(name)); xi.ToNode(); xi=xi.NextSibling(), ++i) {
                if (i==idx) {
                    return xi.GetAttributeString(attrib, defaultValue);
                }
            }
            return defaultValue;
        }
        hUint32      GetParameterHash() const
        {
            hUint32 ret;
            hCRC32::StartCRC32(&ret, path_, hStrLen(path_));

            for (hXMLGetter i(root_->FirstChild(NULL)); i.ToNode(); i = i.NextSiblingAny())
            {
                hCRC32::ContinueCRC32(&ret, i.ToNode()->name(), (hUint32)i.ToNode()->name_size());
                if (i.ToNode()->value())
                {
                    hCRC32::ContinueCRC32(&ret, i.ToNode()->value(), (hUint32)i.ToNode()->value_size());
                }
            }

            hCRC32::FinishCRC32(&ret);

            return ret;
        }
    private:

        hXMLGetter*     root_;
        const hChar*    name_;
        const hChar*    path_;
    };

    class HEART_DLLEXPORT hDataCacheFile : public hIDataCacheFile
    {
    public:
        hDataCacheFile(hIFile* handle)
            : handle_(handle)
        {

        }

        ~hDataCacheFile()
        {

        }

        hUint32 Read(void* dst, hUint32 size)
        {
            return handle_->Read(dst, size);
        }
        hUint32 Seek(hUint32 offset, SeekOffset from)
        {
            if (from == BEGIN)
                return handle_->Seek(offset, SEEKOFFSET_BEGIN);
            else if (from == CURRENT)
                return handle_->Seek(offset, SEEKOFFSET_CURRENT);
            else
                return handle_->Seek(offset, SEEKOFFSET_END);
        }
        hUint32 Lenght()
        {
            return (hUint32)handle_->Length();
        }
        hUint32 Tell()
        {
            return (hUint32)handle_->Tell();
        }

    private:

        friend class hBuiltDataCache;

        hIFile* handle_;
    };

    class HEART_DLLEXPORT hBuiltDataCache : public hIBuiltDataCache
    {
    public:
        hBuiltDataCache(hIFileSystem* fileSystem, 
                        const hChar* packagePath, 
                        const hChar* resName, 
                        const hChar* resourcePath, 
                        hUint32 parameterHash,
                        hTime libTimeStamp,
                        hUint32 libVerMajor,
                        hUint32 libVerMinor);

        ~hBuiltDataCache()
        {
            SaveCacheData();
        }

        hIDataCacheFile* OpenFile(const hChar* filename);
        void             CloseFile(hIDataCacheFile* file);
        void             LoadCacheData();
        void             SaveCacheData();
        hBool            IsCacheValid();

    private:

        void                AppendFileTimestampToCache( hIFile* file, const hChar* filepath );

        hXMLDocument        doc_;
        hUint32             libVerMajor_;
        hUint32             libVerMinor_;
        hTime               libTimestamp_;
        hUint32             parameterHash_;
        hIFileSystem*       fileSystem_;
        const hChar*        packagePack_;
        const hChar*        resName_;
        const hChar*        resourcePath_;
        hBool               checkedCache_;
        hBool               validCache_;
        hBool               useCache_;
    };
}

#endif // HBUILTDATACACHE_H__