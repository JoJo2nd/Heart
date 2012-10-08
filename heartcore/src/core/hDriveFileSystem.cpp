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

namespace Heart
{
namespace 
{
    static const hChar FILE_PREFIX[] = {"GAMEDATA/"};

	struct hEnumerateFilesCallbackInfo
	{
		hEnumerateFilesCallback fn_;

		hBool Callback(const hdFileHandleInfo* pInfo )
		{
			hFileInfo fi;
			fi.name_ = pInfo->name_;
			fi.directory_ = pInfo->directory_;
			fi.path_ = pInfo->path_+(sizeof(FILE_PREFIX)-1);

			return fn_( &fi );
		}
	};
}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hIFile* hDriveFileSystem::OpenFile( const hChar* filename, hFileMode mode ) const
	{
        hUint32 len = hStrLen( filename )+hStrLen( FILE_PREFIX )+1;
        hChar* fullFilename = (hChar*)hAlloca( len );
        hStrCopy( fullFilename, len, FILE_PREFIX );
        hStrCat( fullFilename, len, filename );

		return OpenFileRoot( fullFilename, mode );
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hIFile* hDriveFileSystem::OpenFileRoot( const hChar* filename, hFileMode mode ) const
    {
        hdFileHandle* fh;
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

        if ( !hdFopen( filename, devMode, &fh ) )
        {
            return NULL;
        }

        hDriveFile* pFile = hNEW(GetGlobalHeap(), hDriveFile);
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

		hdFclose( ((hDriveFile*)pFile)->fileHandle_ );

		hDELETE(GetGlobalHeap(), pFile);
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hDriveFileSystem::EnumerateFiles( const hChar* path, hEnumerateFilesCallback fn ) const
	{
		hEnumerateFilesCallbackInfo cbInfo;
        cbInfo.fn_ = fn;

        hUint32 len = hStrLen( path )+hStrLen( FILE_PREFIX )+1;
        hChar* fullFilename = (hChar*)hAlloca( len );
        hStrCopy( fullFilename, len, FILE_PREFIX );
        hStrCat( fullFilename, len, path );

		hdEnumerateFiles( fullFilename, hdEnumerateFilesCallback::bind< hEnumerateFilesCallbackInfo, &hEnumerateFilesCallbackInfo::Callback >( &cbInfo ) );
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDriveFileSystem::CreateDirectory( const hChar* path )
    {
        hUint32 len = hStrLen( path )+hStrLen( FILE_PREFIX )+1;
        hChar* fullFilename = (hChar*)hAlloca( len );
        hStrCopy( fullFilename, len, FILE_PREFIX );
        hStrCat( fullFilename, len, path );

        hdCreateDirectory(fullFilename);
    }

}