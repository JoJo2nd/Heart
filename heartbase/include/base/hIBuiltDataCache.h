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
    class hIDataCacheFile
    {
    public:
        virtual ~hIDataCacheFile() {}

        virtual hUint32 Read(void* dst, hUint32 size) = 0;
        virtual hUint32 Lenght() = 0;
    };

    class hIBuiltDataCache
    {
    public:
        virtual ~hIBuiltDataCache() {}

        virtual hIDataCacheFile* OpenFile(const hChar* filename) = 0;
        virtual void             CloseFile(hIBuiltDataCache* file) = 0;
        virtual void             LoadCacheData(const hChar* resorucePath) = 0;
        virtual void             SaveCacheData() = 0;
        virtual hBool            IsCacheValid() const = 0;
    };
}

#endif // HIBUILTDATACACHE_H__