/********************************************************************

	filename: 	hDriveFile.h	
	
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

#ifndef DRIVEFILE_H__
#define DRIVEFILE_H__

#include "core/hIFile.h"
#include "pal/hDeviceFileSystem.h"

namespace Heart
{
	class hDriveFile : public hIFile
	{
	public:

        hDriveFile()
            : mmap_(hNullptr)
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

		hdFileHandle        fileHandle_;
        hdFileStat          stat_;
        hdMemoryMappedFile* mmap_;
        hUint64             mmapPos_;
        hUint64             size_;
	};
}

#endif // DRIVEFILE_H__