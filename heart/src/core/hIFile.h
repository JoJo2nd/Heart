/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef hcFile_h__
#define hcFile_h__

#include "base/hTypes.h"
#include "base/hDeviceFileSystem.h"

namespace Heart
{
	class hIFile
	{
	public:
		hIFile() {}
		virtual ~hIFile() {}

        virtual hUint32				Read( void* pBuffer, hUint32 size ) = 0;
        virtual hUint32				Write( const void* pBuffer, hUint32 size ) = 0;
        virtual hUint32				Seek( hInt64 offset, hdSeekOffset from ) = 0;
		virtual hUint64				Tell() = 0;
		virtual hUint64				Length() = 0;
        virtual hTime               GetTimestamp() = 0;
        virtual hBool               getIsMemMapped() const = 0;
        virtual void*               getMemoryMappedBase() const = 0;
	};
}

#endif // hcFile_h__