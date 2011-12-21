/********************************************************************
	created:	2010/05/31
	created:	31:5:2010   10:46
	filename: 	FileSystem.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef FILESYSTEM_H__
#define FILESYSTEM_H__

#include "hTypes.h"
#include "huFunctor.h"

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

	struct FileHandleInfo
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
	hBool		Fopen( const hChar* filename, const hChar* mode, FileHandle** pOut );
	/**
	* Fclose 
	*
	* @param 	FileHandle * pHandle
	* @return   hBool
	*/
	hBool		Fclose( FileHandle* pHandle );
	/**
	* Fread 
	*
	* @param 	FileHandle * pHandle
	* @param 	void * pBuffer
	* @param 	hUint32 size
	* @return   hUint32
	*/
	FileError	Fread( FileHandle* pHandle, void* pBuffer, hUint32 size, hUint32* read );
	/**
	* Fseek 
	*
	* @param 	FileHandle * pHandle
	* @param 	hUint32 offset
	* @param 	hUint32 from
	* @return   hUint32
	*/
	FileError	Fseek( FileHandle* pHandle, hUint64 offset, SeekOffset from );
	/**
	* Ftell 
	*
	* @param 	FileHandle * pHandle
	* @return   hUint32
	*/
	hUint64		Ftell( FileHandle* pHandle );
	/**
	* Fsize 
	*
	* @param 	FileHandle * pHandle
	* @return   hUint32
	*/
	hUint64		Fsize( FileHandle* pHandle );
	/**
	* Fwrite 
	*
	* @param 	FileHandle * pHandle
	* @param 	const void * pBuffer
	* @param 	hUint32 size
	* @return   hUint32
	*/
	FileError	Fwrite( FileHandle* pHandle, const void* pBuffer, hUint32 size, hUint32* written );

	/**
	* EnumerateFiles 
	*
	* @param 	const hChar * path
	* @param 	hBool incDirectories
	* @return   void
	*/
	void		EnumerateFiles( const hChar* path, EnumerateFilesCallback fn );
}
}
}

#endif // FILESYSTEM_H__