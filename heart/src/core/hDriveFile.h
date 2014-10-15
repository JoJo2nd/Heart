/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef DRIVEFILE_H__
#define DRIVEFILE_H__

#include "core/hIFile.h"
#include "base/hDeviceFileSystem.h"

namespace Heart
{
	class hDriveFile : public hIFile
	{
	public:

        hDriveFile()
            : mmap_(nullptr)
        {}
        ~hDriveFile()
        {}

        hUint32				Read( void* pBuffer, hUint32 size );
        hUint32				Write( const void* pBuffer, hUint32 size );
        hUint32     		Seek( hInt64 offset, hdSeekOffset from );
		hUint64				Tell();
		hUint64				Length();
        hTime               GetTimestamp();
        hBool               getIsMemMapped() const;
        void*               getMemoryMappedBase() const;

	private:

		friend class		hDriveFileSystem;

		hdFileHandle*       fileHandle_;
        hdFileStat          stat_;
        hdMemoryMappedFile*	mmap_;
        hUint64             mmapPos_;
        hUint64             size_;
	};
}

#endif // DRIVEFILE_H__