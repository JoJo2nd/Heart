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
namespace Device
{
namespace FileSystem
{
	class FileHandle;

	enum SeekOffset
	{
		SEEKOFFSET_BEGIN,
		SEEKOFFSET_CURRENT,
		SEEKOFFSET_END
	};

	enum FileError
	{
		FILEERROR_NONE		= 0,
		FILEERROR_PENDING	= -1,
		FILEERROR_FAILED	= -2,
	};

	struct HEARTDEV_SLIBEXPORT FileHandleInfo
	{
		const hChar*	path_;
		const hChar*	name_;
		hBool			directory_;
	};

	typedef huFunctor< hBool(*)(const FileHandleInfo*) >::type		EnumerateFilesCallback;

	/**
	* Fopen 
	*
	* @param 	const hChar * filename
	* @param 	const hChar * access
	* @param 	FileHandle * pOut
	* @return   hBool
	*/
    HEARTDEV_SLIBEXPORT
	hBool		HEART_API Fopen( const hChar* filename, const hChar* mode, FileHandle** pOut );
	/**
	* Fclose 
	*
	* @param 	FileHandle * pHandle
	* @return   hBool
	*/
    HEARTDEV_SLIBEXPORT
	hBool		HEART_API Fclose( FileHandle* pHandle );
	/**
	* Fread 
	*
	* @param 	FileHandle * pHandle
	* @param 	void * pBuffer
	* @param 	hUint32 size
	* @return   hUint32
	*/
    HEARTDEV_SLIBEXPORT
	FileError	HEART_API Fread( FileHandle* pHandle, void* pBuffer, hUint32 size, hUint32* read );
	/**
	* Fseek 
	*
	* @param 	FileHandle * pHandle
	* @param 	hUint32 offset
	* @param 	hUint32 from
	* @return   hUint32
	*/
    HEARTDEV_SLIBEXPORT
	FileError	HEART_API Fseek( FileHandle* pHandle, hUint64 offset, SeekOffset from );
	/**
	* Ftell 
	*
	* @param 	FileHandle * pHandle
	* @return   hUint32
	*/
    HEARTDEV_SLIBEXPORT
	hUint64		HEART_API Ftell( FileHandle* pHandle );
	/**
	* Fsize 
	*
	* @param 	FileHandle * pHandle
	* @return   hUint32
	*/
    HEARTDEV_SLIBEXPORT
	hUint64		HEART_API Fsize( FileHandle* pHandle );
	/**
	* Fwrite 
	*
	* @param 	FileHandle * pHandle
	* @param 	const void * pBuffer
	* @param 	hUint32 size
	* @return   hUint32
	*/
    HEARTDEV_SLIBEXPORT
	FileError	HEART_API Fwrite( FileHandle* pHandle, const void* pBuffer, hUint32 size, hUint32* written );

	/**
	* EnumerateFiles 
	*
	* @param 	const hChar * path
	* @param 	hBool incDirectories
	* @return   void
	*/
    HEARTDEV_SLIBEXPORT
	void		HEART_API EnumerateFiles( const hChar* path, EnumerateFilesCallback fn );
}
}
}

#endif // FILESYSTEM_H__