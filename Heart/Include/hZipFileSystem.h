/********************************************************************

	filename: 	hZipFileSystem.h	
	
	Copyright (c) 20:3:2011 James Moran
	
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

#ifndef ZIPFILESYSTEM_H__
#define ZIPFILESYSTEM_H__

#include "hIFileSystem.h"
#include "hSemaphore.h"
#include "unzip.h"
#include "HeartSTL.h"

namespace Heart
{
namespace Device
{
	class FileHandle;
}

	typedef huFunctor< void(*)(unzFile) >::type		ZipIOJobCallback;
	
	class hZipFileSystem : public hIFileSystem
	{
	public:

		hZipFileSystem() :
			zipFileHandle_( NULL )
		{
			requestSemaphore_.Create( 0, 4096 );
		}

		~hZipFileSystem()
		{
		}

		hBool			Initialise( const hChar* zipFile );

		void			Destory();

		/**
		* OpenFile 
		*
		* @param 	const hChar * filename
		* @param 	FileMode mode
		* @return   hIFile*
		*/
		virtual hIFile*	OpenFile( const hChar* filename, FileMode mode ) const;
		
		/**
		* CloseFile 
		*
		* @return   void
		*/
		virtual void	CloseFile( hIFile* pFile ) const;

		/**
		* EnumerateFiles 
		*
		* @param 	const hChar * path
		* @param 	EnumerateFilesCallback fn
		* @return   void
		*/
		virtual void	EnumerateFiles( const hChar* path, EnumerateFilesCallback fn ) const;

		void			PushFileIOJob( ZipIOJobCallback fn ) const;

	private:

		struct ZipFileEntry
		{
			hChar*				name_;
			union
			{
				struct
				{
					hUint16				nReserve_;
					hUint16				nEntries_;
					ZipFileEntry*		pEntries_;
				};
				struct 
				{
					unz64_file_pos		zipEntry_;
					hUint64				size_;
				};
			};
			hByte				directory_;
		};

		hUint32					FileIO( void* );
		hChar*					GetStringMem( hUint32 size )
		{
			hChar* pRet = pStringOfffset_;
			pStringOfffset_ += size;
			hcAssert( pStringOfffset_ < (pStringPool_+stringPoolSize_) );
			return pRet;
		}
		ZipFileEntry*			AddEntry( ZipFileEntry* pParent, const hChar* name, hUint32 len, hBool isDir, unz64_file_pos zipEntry, const unz_file_info64& info );
		void					RemoveEntry( ZipFileEntry* toremove );
		ZipFileEntry*			FindEntry( ZipFileEntry* pParent, const hChar* name, hUint32 len ) const;
		ZipFileEntry*			FindPath( const hChar* name ) const;


		zlib_filefunc64_def					zipFileIODefs_;
		unzFile								zipFileHandle_;
		hThread								zipIOThread_;
		hUint32								stringPoolSize_;
		hChar*								pStringPool_;
		hChar*								pStringOfffset_;
		ZipFileEntry*						pRootEntry_;
		mutable hSemaphore					requestSemaphore_;
		mutable hMutex						requestQueueMutex_;
		mutable deque< ZipIOJobCallback >	requestQueue_;
		hBool								complete_;
		volatile hBool						doneFileSystemRead_;
	};

}

#endif // ZIPFILESYSTEM_H__