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

	struct HEARTDEV_SLIBEXPORT hdFileHandleInfo
	{
		const hChar*	path_;
		const hChar*	name_;
		hBool			directory_;
	};

    struct HEARTDEV_SLIBEXPORT hdFileStat
    {
        hTime   createTime_;
        hTime   lastModTime_;
        hTime   lastAccessTime_;
    };

	typedef huFunctor< hBool(*)(const hdFileHandleInfo*) >::type		hdEnumerateFilesCallback;

	/**
	* Fopen 
	*
	* @param 	const hChar * filename
	* @param 	const hChar * access
	* @param 	FileHandle * pOut
	* @return   hBool
	*/
    HEARTDEV_SLIBEXPORT
	hBool		HEART_API hdFopen( const hChar* filename, const hChar* mode, hdFileHandle** pOut );
	/**
	* Fclose 
	*
	* @param 	FileHandle * pHandle
	* @return   hBool
	*/
    HEARTDEV_SLIBEXPORT
	hBool		HEART_API hdFclose( hdFileHandle* pHandle );
	/**
	* Fread 
	*
	* @param 	FileHandle * pHandle
	* @param 	void * pBuffer
	* @param 	hUint32 size
	* @return   hUint32
	*/
    HEARTDEV_SLIBEXPORT
	hdFileError	HEART_API hdFread( hdFileHandle* pHandle, void* pBuffer, hUint32 size, hUint32* read );
	/**
	* Fseek 
	*
	* @param 	FileHandle * pHandle
	* @param 	hUint32 offset
	* @param 	hUint32 from
	* @return   hUint32
	*/
    HEARTDEV_SLIBEXPORT
	hdFileError	HEART_API hdFseek( hdFileHandle* pHandle, hUint64 offset, hdSeekOffset from );
	/**
	* Ftell 
	*
	* @param 	FileHandle * pHandle
	* @return   hUint32
	*/
    HEARTDEV_SLIBEXPORT
	hUint64		HEART_API hdFtell( hdFileHandle* pHandle );
	/**
	* Fsize 
	*
	* @param 	FileHandle * pHandle
	* @return   hUint32
	*/
    HEARTDEV_SLIBEXPORT
	hUint64		HEART_API hdFsize( hdFileHandle* pHandle );
	/**
	* Fwrite 
	*
	* @param 	FileHandle * pHandle
	* @param 	const void * pBuffer
	* @param 	hUint32 size
	* @return   hUint32
	*/
    HEARTDEV_SLIBEXPORT
	hdFileError	HEART_API hdFwrite( hdFileHandle* pHandle, const void* pBuffer, hUint32 size, hUint32* written );

	/**
	* EnumerateFiles 
	*
	* @param 	const hChar * path
	* @param 	hBool incDirectories
	* @return   void
	*/
    HEARTDEV_SLIBEXPORT
	void		HEART_API hdEnumerateFiles( const hChar* path, hdEnumerateFilesCallback fn );

    HEARTDEV_SLIBEXPORT
    hdFileStat   HEART_API hdFstat(hdFileHandle* handle);
}

#endif // FILESYSTEM_H__