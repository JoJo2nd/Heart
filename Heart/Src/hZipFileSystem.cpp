/********************************************************************

	filename: 	hZipFileSystem.cpp	
	
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
#include "hZipFileSystem.h"
#include "DeviceFileSystem.h"
#include "unzip.h"
#include "hZipFile.h"
#include "hAtomic.h"

namespace Heart
{
	struct CacheBlock
	{
		static const hUint32 ZIP_CACHE_BLOCK_SIZE = 1024;
		hByte	zipCacheBlock_[ZIP_CACHE_BLOCK_SIZE];
		hUint64 cacheStartAddress_;
		hUint32 offsetInCache_;
		hUint32 cacheSize_;
		hBool	valid_;
	};

	static CacheBlock gZipCache;

namespace 
{
	void InvalidateCache( CacheBlock* block )
	{
		block->offsetInCache_ = 0;
		block->cacheStartAddress_ = ~0UL;
		block->cacheSize_ = 0;
		block->valid_ = hFalse;
	}

	hBool FileBlockInCache( CacheBlock* block, hUint32 size )
	{
		if ( !block->valid_ || size >= CacheBlock::ZIP_CACHE_BLOCK_SIZE )
		{
			InvalidateCache( block );
			return hFalse;
		}

		if ( block->offsetInCache_ + size > block->cacheSize_ )
		{
			InvalidateCache( block );
			return hFalse;
		}

		return hTrue;
	}

	void UpdateCache( CacheBlock* block, Device::FileSystem::FileHandle* fh )
	{
		block->valid_ = false;

		block->cacheStartAddress_ = Device::FileSystem::Ftell( fh );
		hUint64 spaceLeft = Device::FileSystem::Fsize( fh ) - block->cacheStartAddress_;
		if ( spaceLeft < CacheBlock::ZIP_CACHE_BLOCK_SIZE )
		{
			return;
		}

        hUint32 read;
		if ( Device::FileSystem::Fread( fh, block->zipCacheBlock_, CacheBlock::ZIP_CACHE_BLOCK_SIZE, &read ) != Device::FileSystem::FILEERROR_NONE )
		{
			return;
		}

		block->valid_ = true;
		block->offsetInCache_ = 0;
	}

	void GetDataFromCache( CacheBlock* block, void* buf, hUint32 size )
	{
		if ( block->valid_ )
		{
			return;
		}
		
		memcpy( buf, block->zipCacheBlock_+block->offsetInCache_, size );
		block->offsetInCache_ += size;
	}

	voidpf ZipOpen( voidpf opaque, const void* filename, int mode )
	{
		(void)opaque;
		(void)mode;
		Device::FileSystem::FileHandle* fh;
		if ( !Device::FileSystem::Fopen( (const hChar*)filename, "r", &fh ) )
			return NULL;

#ifdef USE_ZIP_CACHE
		InvalidateCache( &gZipCache );
#endif // USE_ZIP_CACHE

		return fh;
	}

	int ZipClose( voidpf opaque, voidpf stream )
	{
		(void)opaque;
		Device::FileSystem::FileHandle* fh = (Device::FileSystem::FileHandle*)stream;

		Device::FileSystem::Fclose( fh );

		return 0;
	}

	int ZipError(voidpf opaque, voidpf stream)
	{
		(void)opaque;
		Device::FileSystem::FileHandle* fh = (Device::FileSystem::FileHandle*)stream;

		if ( Device::FileSystem::Ftell( fh ) >= Device::FileSystem::Fsize( fh ) )
		{
			return 0;
		}

		return 1;
	}

	uLong ZipRead( voidpf opaque, voidpf stream, void* buf, uLong size )
	{
		(void)opaque;
		Device::FileSystem::FileHandle* fh = (Device::FileSystem::FileHandle*)stream;

#ifdef USE_ZIP_CACHE
		if ( FileBlockInCache( &gZipCache, size ) )
		{
			GetDataFromCache( &gZipCache, buf, size );
			return size;
		}

		if ( size < CacheBlock::ZIP_CACHE_BLOCK_SIZE )
		{
			UpdateCache( &gZipCache, fh );
			if ( FileBlockInCache( &gZipCache, size ) )
			{
				GetDataFromCache( &gZipCache, buf, size );
				return size;
			}
		}
#endif // USE_ZIP_CACHE

        hUint32 bytesRead;
		if ( Device::FileSystem::Fread( fh, buf, size, &bytesRead ) != Device::FileSystem::FILEERROR_NONE )
		{
			return 0;
		}
		return bytesRead;
	}

	uLong ZipWrite( voidpf opaque, voidpf stream, const void* buf, uLong size )
	{
		(void)opaque;
		Device::FileSystem::FileHandle* fh = (Device::FileSystem::FileHandle*)stream;

        hUint32 bytesWritten;
		if ( Device::FileSystem::Fwrite( fh, buf, size, &bytesWritten ) != Device::FileSystem::FILEERROR_NONE )
		{
			return 0;
		}

		return bytesWritten;
	}

	ZPOS64_T ZipTell( voidpf opaque, voidpf stream )
	{
		(void)opaque;
		Device::FileSystem::FileHandle* fh = (Device::FileSystem::FileHandle*)stream;

		return Device::FileSystem::Ftell( fh );
	}

	long ZipSeek( voidpf opaque, voidpf stream, ZPOS64_T offset, int origin )
	{
		(void)opaque;
		Device::FileSystem::FileHandle* fh = (Device::FileSystem::FileHandle*)stream;
		Device::FileSystem::SeekOffset devOrigin;

#ifdef USE_ZIP_CACHE
		InvalidateCache( &gZipCache );
#endif // USE_ZIP_CACHE

		switch ( origin )
		{
		default:
		case ZLIB_FILEFUNC_SEEK_CUR: devOrigin = Device::FileSystem::SEEKOFFSET_CURRENT; break;
		case ZLIB_FILEFUNC_SEEK_SET: devOrigin = Device::FileSystem::SEEKOFFSET_BEGIN; break;
		case ZLIB_FILEFUNC_SEEK_END: devOrigin = Device::FileSystem::SEEKOFFSET_END; break;
		}

		Device::FileSystem::Fseek( fh, offset, devOrigin );

		return 0;
	}
}
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool hZipFileSystem::Initialise( const hChar* zipFile )
	{
		zipFileIODefs_.zopen64_file = ZipOpen;
		zipFileIODefs_.zclose_file	= ZipClose;
		zipFileIODefs_.zerror_file	= ZipError;
		zipFileIODefs_.zread_file	= ZipRead;
		zipFileIODefs_.zwrite_file	= ZipWrite;
		zipFileIODefs_.zseek64_file = ZipSeek;
		zipFileIODefs_.ztell64_file = ZipTell;
		zipFileIODefs_.opaque		= NULL;

		zipFileHandle_ = unzOpen2_64( zipFile, &zipFileIODefs_ );

		if ( !zipFileHandle_ )
			return hFalse;

		doneFileSystemRead_ = hFalse;
		complete_ = hFalse;

		zipIOThread_.Begin(
			"ZipIO",
			hThread::PRIORITY_ABOVENORMAL,
			Device::Thread::ThreadFunc::bind< hZipFileSystem, &hZipFileSystem::FileIO >( this ), NULL );

		return hTrue;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hZipFileSystem::Destory()
	{
		complete_ = true;
		requestSemaphore_.Post();

		while ( !zipIOThread_.IsComplete() ) { hThreading::ThreadSleep( 1 ); }

		hDELETE_ARRAY(hGeneralHeap, pStringPool_, stringPoolSize_);

		// delete zip entries
		RemoveEntry( pRootEntry_ );
		hDELETE(hGeneralHeap, pRootEntry_);

		unzClose( zipFileHandle_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hIFile* hZipFileSystem::OpenFile( const hChar* filename, FileMode mode ) const
	{
		if ( mode != FILEMODE_READ )
			return NULL;

		ZipFileEntry* pZip = FindPath( filename );

		if ( !pZip )
			return NULL;
		if ( pZip->directory_ )
			return NULL;

		hZipFile* pZipFile = hNEW(hGeneralHeap, hZipFile);
		//setup the zipfile
		pZipFile->pFileSystem_ = this;
		pZipFile->filePos_ = 0;
		pZipFile->size_ = pZip->size_;
		pZipFile->zipFilePos_ = pZip->zipEntry_;

		return pZipFile;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hZipFileSystem::CloseFile( hIFile* pFile ) const
	{
		if ( !pFile )
			return;

		delete pFile;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hZipFileSystem::EnumerateFiles( const hChar* path, EnumerateFilesCallback fn ) const
	{
		ZipFileEntry* pEnt = FindPath( path );

		if ( !pEnt || !pEnt->directory_ )
		{
			return;
		}

		for ( hUint32 i = 0; i < pEnt->nEntries_; ++i )
		{
			FileInfo inf;
			inf.directory_ = pEnt->pEntries_[i].directory_;
			inf.name_ = pEnt->pEntries_[i].name_;
			inf.path_ = path;

			fn( &inf );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hZipFileSystem::FileIO( void* )
	{
		stringPoolSize_ = 0;

		for ( hInt32 ret = unzGoToFirstFile( zipFileHandle_ ); ret == UNZ_OK; ret = unzGoToNextFile( zipFileHandle_ ) )
		{
			unz_file_info64 pfile_info;
			char pFileName[ 1024 ];

			unzGetCurrentFileInfo64( 
				zipFileHandle_,
				&pfile_info,
				pFileName,
				1024,
				NULL, 
				0,
				NULL,
				0);

			stringPoolSize_ += strlen( pFileName ) + 1;
		}

		pStringPool_ = hNEW_ARRAY(hGeneralHeap, hChar, stringPoolSize_);
		pStringOfffset_ = pStringPool_;
		pRootEntry_ = hNEW(hGeneralHeap, ZipFileEntry);
		pRootEntry_->name_ = NULL;
		pRootEntry_->nEntries_ = 0;
		pRootEntry_->pEntries_ = NULL;
		pRootEntry_->nReserve_ = 0;
		pRootEntry_->directory_ = hTrue;

		for ( hInt32 ret = unzGoToFirstFile( zipFileHandle_ ); ret == UNZ_OK; ret = unzGoToNextFile( zipFileHandle_ ) )
		{
			unz64_file_pos file_pos;
			unz_file_info64 pfile_info;
			char pFileName[ 1024 ];

			unzGetFilePos64( zipFileHandle_, &file_pos );
			unzGetCurrentFileInfo64( 
				zipFileHandle_,
				&pfile_info,
				pFileName,
				1024,
				NULL, 
				0,
				NULL,
				0);
	
			ZipFileEntry* pEntry = pRootEntry_;
			const hChar* pstr = pFileName;

			do
			{
				const hChar* pend = strpbrk( pstr, "\\/" );

				if ( pend )
				{
					ZipFileEntry* pFound;
					pFound = FindEntry( pEntry, pstr, (hUint32)(pend - pstr) );
					if ( !pFound )
					{
						pFound = AddEntry( pEntry, pstr, (hUint32)(pend - pstr), hTrue, file_pos, pfile_info );
					}
					pEntry = pFound;
					pstr = pend + 1;
				}
				else 
				{
					AddEntry( pEntry, pstr, strlen(pstr), hFalse, file_pos, pfile_info );
					pEntry = NULL;
				}
			}
			while ( pEntry );
		}

		hAtomic::LWMemoryBarrier();
		doneFileSystemRead_ = hTrue;

		do
		{
			requestSemaphore_.Wait();
	
			requestQueueMutex_.Lock();
			if ( !requestQueue_.empty() )
			{
				ZipIOJobCallback todofn = *requestQueue_.begin();
				requestQueue_.pop_front();
				requestQueueMutex_.Unlock();

				todofn( zipFileHandle_ );
			}
			else
			{
				requestQueueMutex_.Unlock();
			}

		}
		while( !complete_ );

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hZipFileSystem::PushFileIOJob( ZipIOJobCallback fn ) const
	{
		requestQueueMutex_.Lock();
		requestQueue_.push_back( fn );
		requestQueueMutex_.Unlock();

		requestSemaphore_.Post();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hZipFileSystem::ZipFileEntry* hZipFileSystem::FindEntry( hZipFileSystem::ZipFileEntry* pParent, const hChar* name, hUint32 len ) const
	{
		if ( !pParent )
		{
			return NULL;
		}

		//TODO: binary search
		for ( hUint32 i = 0; i < pParent->nEntries_; ++i )
		{
			if ( strncmp( name, pParent->pEntries_[i].name_, len ) == 0 )
			{
				return &pParent->pEntries_[i];
			}
		}

		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hZipFileSystem::ZipFileEntry* hZipFileSystem::AddEntry( ZipFileEntry* pParent, const hChar* name, hUint32 len, hBool isDir, unz64_file_pos zipEntry, const unz_file_info64& info )
	{
		if ( pParent->nEntries_ >= pParent->nReserve_ )
		{
			if ( pParent->nReserve_ > 0 )
			{
				pParent->nReserve_ *= 2;
			}
			else
			{
				pParent->nReserve_ = 8;
			}

			ZipFileEntry* ptmp = hNEW_ARRAY(hGeneralHeap, ZipFileEntry, pParent->nReserve_);
			memcpy( ptmp, pParent->pEntries_, pParent->nEntries_*sizeof(ZipFileEntry) );
			hDELETE_ARRAY(hGeneralHeap, pParent->pEntries_, pParent->nEntries_);
			pParent->pEntries_ = ptmp;
		}

		ZipFileEntry* pnew = &pParent->pEntries_[pParent->nEntries_];
		
		pnew->name_ = GetStringMem( len + 1 );
		strncpy( pnew->name_, name, len );
		pnew->name_[len] = '\0';

		pnew->directory_ = isDir;

		if ( !pnew->directory_ )
		{
			pnew->zipEntry_ = zipEntry;
			pnew->size_ = info.uncompressed_size;
		}
		else
		{
			pnew->nEntries_ = 0;
			pnew->pEntries_ = NULL;
			pnew->nReserve_ = 0;
		}

		++pParent->nEntries_;

		return pnew;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hZipFileSystem::ZipFileEntry* hZipFileSystem::FindPath( const hChar* name ) const 
	{
		if ( !doneFileSystemRead_ )
		{
			do 
			{
				hThreading::ThreadSleep( 1 );
			}
			while ( !doneFileSystemRead_ );
		}

		ZipFileEntry* pEntry = pRootEntry_;
		const hChar* pstr = name;

		if ( strlen(pstr) == 0 )
		{
			return pRootEntry_;
		}

		do
		{
			const hChar* pend = strpbrk( pstr, "\\/" );

			if ( pend )
			{
				ZipFileEntry* pFound;
				pFound = FindEntry( pEntry, pstr, (hUint32)(pend - pstr) );
				if ( !pFound )
				{
					return NULL;
				}
				pEntry = pFound;
				pstr = pend + 1;
			}
			else 
			{
				return FindEntry( pEntry, pstr, strlen(pstr) );
			}
		}
		while ( pEntry );

		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	// 21:33:40 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hZipFileSystem::RemoveEntry( ZipFileEntry* toremove )
	{
		if ( !toremove )
			return;

		if ( toremove->directory_ && toremove->pEntries_ )
		{
			for ( hUint32 i = 0; i < toremove->nEntries_; ++i )
			{
				RemoveEntry( &toremove->pEntries_[i] );
			}
			delete toremove->pEntries_;
			toremove->pEntries_ = NULL;
		}
	}

}