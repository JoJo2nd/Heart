/********************************************************************

	filename: 	hIFile.h	
	
	Copyright (c) 1:4:2012 James Moran
	
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