/********************************************************************
	created:	2008/06/08
	created:	8:6:2008   18:06
	filename: 	hcFile.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef hcFile_h__
#define hcFile_h__

#include "hTypes.h"
#include "hIFileSystem.h"

class HeartEngine;

namespace Heart
{
	enum SeekOffset
	{
		SEEKOFFSET_BEGIN,
		SEEKOFFSET_CURRENT,
		SEEKOFFSET_END
	};

	class hIFile
	{
	public:
		hIFile() {}
		virtual ~hIFile() {}

        virtual hUint32				Read( void* pBuffer, hUint32 size ) = 0;
        virtual hUint32				Write( const void* pBuffer, hUint32 size ) = 0;
        virtual hUint32				Seek( hUint64 offset, SeekOffset from ) = 0;
		virtual hUint64				Tell() = 0;
		virtual hUint64				Length() = 0;
	};
}

#endif // hcFile_h__