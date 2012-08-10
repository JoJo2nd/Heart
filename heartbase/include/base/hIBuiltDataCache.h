/********************************************************************

	filename: 	hIBuiltDataCache.h	
	
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
#ifndef HIBUILTDATACACHE_H__
#define HIBUILTDATACACHE_H__

namespace Heart
{
    class HEARTCORE_SLIBEXPORT hIDataParameterSet
    {
    public:
        virtual ~hIDataParameterSet() {}

        virtual const hChar* GetResourceName() const = 0;
        virtual const hChar* GetInputFilePath() const = 0;
        virtual const hChar* GetBuildParameter(const hChar* name, const hChar* defaultValue) const = 0;
        virtual hUint32      GetParameterHash() const = 0;
    };

    class HEARTCORE_SLIBEXPORT hIDataCacheFile
    {
    public:
        enum SeekOffset
        {
            BEGIN,
            CURRENT,
            END
        };

        virtual ~hIDataCacheFile() {}

        virtual hUint32 Read(void* dst, hUint32 size) = 0;
        virtual hUint32 Seek(hUint32 offset, SeekOffset from) = 0;
        virtual hUint32 Lenght() = 0;
    };

    class HEARTCORE_SLIBEXPORT hIBuiltDataCache
    {
    public:
        virtual ~hIBuiltDataCache() {}

        virtual hIDataCacheFile* OpenFile(const hChar* filename) = 0;
        virtual void             CloseFile(hIDataCacheFile* file) = 0;
        virtual hBool            IsCacheValid() = 0;
    };
}

#endif // HIBUILTDATACACHE_H__