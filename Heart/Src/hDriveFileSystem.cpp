/********************************************************************

	filename: 	hDriveFileSystem.cpp	
	
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

#include "Common.h"
#include "hDriveFileSystem.h"
#include "DeviceFileSystem.h"
#include "hDriveFile.h"

namespace Heart
{
namespace 
{
	struct EnumerateFilesCallbackInfo
	{
		EnumerateFilesCallback fn_;

		hBool Callback(const Device::FileSystem::FileHandleInfo* pInfo )
		{
			FileInfo fi;
			fi.name_ = pInfo->name_;
			fi.directory_ = pInfo->directory_;
			fi.path_ = pInfo->path_;

			return fn_( &fi );
		}
	};
}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hIFile* hDriveFileSystem::OpenFile( const hChar* filename, FileMode mode ) const
	{
		Device::FileSystem::FileHandle* fh;
		const hChar* devMode;

		if ( mode == FILEMODE_WRITE )
		{
			devMode = "w";
		}
		else if ( mode == FILEMODE_READ )
		{
			devMode = "r";
		}
		else 
		{
			return NULL;
		}
		

		if ( !Device::FileSystem::Fopen( filename, devMode, &fh ) )
		{
			return NULL;
		}
		
		hDriveFile* pFile = hNEW ( hGeneralHeap ) hDriveFile();
		pFile->fileHandle_ = fh;

		return pFile;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hDriveFileSystem::CloseFile( hIFile* pFile ) const
	{
		if ( !pFile )
		{
			return;
		}

		Device::FileSystem::Fclose( ((hDriveFile*)pFile)->fileHandle_ );

		delete pFile;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hDriveFileSystem::EnumerateFiles( const hChar* path, EnumerateFilesCallback fn ) const
	{
		EnumerateFilesCallbackInfo cbInfo;
        cbInfo.fn_ = fn;

		Device::FileSystem::EnumerateFiles( path, Device::FileSystem::EnumerateFilesCallback::bind< EnumerateFilesCallbackInfo, &EnumerateFilesCallbackInfo::Callback >( &cbInfo ) );
	}

}