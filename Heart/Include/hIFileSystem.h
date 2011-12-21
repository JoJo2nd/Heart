/********************************************************************
	created:	2008/06/08
	created:	8:6:2008   17:15
	filename: 	FileSystem.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef hcFileManager_h__
#define hcFileManager_h__

#include "hTypes.h"
#include "hThread.h"
#include "hThreadEvent.h"
#include "hMutex.h"
#include "hQueue.h"
#include "hIFileSystem.h"

namespace Heart
{
	class hIFile;

	enum FileMode
	{
		FILEMODE_READ,
		FILEMODE_WRITE,

		FILEMODE_MAX
	};

	struct FileInfo
	{
		const hChar*	path_;
		const hChar*	name_;
		hByte			directory_;
	};

	typedef huFunctor< hBool(*)(const FileInfo*) >::type		EnumerateFilesCallback;

	class hIFileSystem
	{
	public:
		virtual ~hIFileSystem() {}

		/**
		* OpenFile 
		*
		* @param 	const hChar * filename
		* @param 	FileMode mode
		* @return   hIFile*
		*/
		virtual hIFile*	OpenFile( const hChar* filename, FileMode mode ) const = 0;
		
		/**
		* CloseFile 
		*
		* @return   void
		*/
		virtual void	CloseFile( hIFile* pFile ) const = 0;

		/**
		* EnumerateFiles 
		*
		* @param 	const hChar * path
		* @param 	EnumerateFilesCallback fn
		* @return   void
		*/
		virtual void	EnumerateFiles( const hChar* path, EnumerateFilesCallback fn ) const = 0;

	};
}

#endif // hcFileManager_h__