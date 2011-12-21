/********************************************************************

	filename: 	hSerialiserFileStream.h	
	
	Copyright (c) 24:11:2011 James Moran
	
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

#ifndef HSERIALISERFILESTREAM_H__
#define HSERIALISERFILESTREAM_H__

#include "hSerialiser.h"
#include "hIFileSystem.h"
#include "hIFile.h"

namespace Heart
{
    class hSerialiserFileStream : public hISerialiseStream
    {
    public:

        hSerialiserFileStream() 
            : fileSystem_(NULL)
            , fileHandle_(NULL)
        {

        }

        hBool               Open( const hChar* filepath, hBool write, hIFileSystem* fileSystem )
        {
            fileSystem_ = fileSystem;
            fileHandle_ = fileSystem_->OpenFile( filepath, write ? FILEMODE_WRITE : FILEMODE_READ );
            return fileHandle_ != NULL;
        }
        void                Close()
        {
            hcAssert( fileHandle_ );
            fileSystem_->CloseFile( fileHandle_ );
            fileHandle_ = NULL;
            fileSystem_ = NULL;
        }
        hUint32				Read( void* pBuffer, hUint32 size )
        {
            return fileHandle_->Read( pBuffer, size );
        }
        hUint32				Write( const void* pBuffer, hUint32 size )
        {
            return fileHandle_->Write( pBuffer, size );
        }
        hUint32				Seek( hUint64 offset )
        {
            return fileHandle_->Seek( offset, SEEKOFFSET_BEGIN );
        }
        hUint64				Tell()
        {
            return fileHandle_->Tell();
        }

    private:

        hIFileSystem*       fileSystem_;
        hIFile*             fileHandle_;
        
    };
}

#endif // HSERIALISERFILESTREAM_H__