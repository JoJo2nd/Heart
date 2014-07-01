/********************************************************************

	filename: 	hDeviceFileSystem.h	
	
	Copyright (c) 6:7:2012 James Moran
	
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

#ifndef FILESYSTEM_H__
#define FILESYSTEM_H__

#include "base/hLinkedList.h"
#include "base/hFunctor.h"

namespace Heart
{
	class hdFileHandle;

    enum hdSeekOffset
    {
        SEEKOFFSET_BEGIN,
        SEEKOFFSET_CURRENT,
        SEEKOFFSET_END
    };

	enum hdFileError
	{
		FILEERROR_NONE		= 0,
		FILEERROR_PENDING	= -1,
		FILEERROR_FAILED	= -2,
	};

    enum hdMMapMode
    {
        MMapMode_ReadOnly,
        MMapMode_Write,
        MMapMode_None,
    };

	struct  hdFileHandleInfo
	{
		const hChar*	path_;
		const hChar*	name_;
		hBool			directory_;
	};

    struct  hdFileStat
    {
        hTime   createTime_;
        hTime   lastModTime_;
        hTime   lastAccessTime_;
    };

    class hdFileHandle;
    class hdMemoryMappedFile;

    hFUNCTOR_TYPEDEF(hBool(*)(const hdFileHandleInfo*), hdEnumerateFilesCallback);

    
	hdFileHandle*		HEART_API hdFopen(const hChar* filename, const hChar* mode);    
	hBool				HEART_API hdFclose(hdFileHandle* pHandle);
	hdFileError			HEART_API hdFread(hdFileHandle* pHandle, void* pBuffer, hUint32 size, hUint32* read);
	hdFileError			HEART_API hdFseek(hdFileHandle* pHandle, hUint64 offset, hdSeekOffset from);
	hUint64				HEART_API hdFtell(hdFileHandle* pHandle);
	hUint64				HEART_API hdFsize(hdFileHandle* pHandle);
	hdFileError			HEART_API hdFwrite(hdFileHandle* pHandle, const void* pBuffer, hUint32 size, hUint32* written);
	void				HEART_API hdEnumerateFiles(const hChar* path, hdEnumerateFilesCallback fn);
	hdFileStat  		HEART_API hdFstat(hdFileHandle* handle);
    hdMemoryMappedFile*	HEART_API hdMMap(hdFileHandle* handle, hSize_t offset, hSize_t size, hdMMapMode mode);
    void*				HEART_API hdMMapGetBase(hdMemoryMappedFile* mmap);
    void 				HEART_API hdUnmap(hdMemoryMappedFile* mmapview);
    void 				HEART_API hdCreateDirectory(const hChar* path);
    void 				HEART_API hdMountPoint(const hChar* path, const hChar* mount);
    void 				HEART_API hdUnmountPoint(const hChar* mount);
    void 				HEART_API hdGetCurrentWorkingDir(hChar* out, hUint bufsize);
    void 				HEART_API hdGetProcessDirectory(hChar* outdir, hUint size);
    hBool 				HEART_API hdIsAbsolutePath(const hChar* path);
    void 				HEART_API hdGetSystemPath(const hChar* path, hChar* outdir, hUint size);
    hUint 				HEART_API hdGetSystemPathSize(const hChar* path);
}

#endif // FILESYSTEM_H__