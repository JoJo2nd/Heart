/********************************************************************

	filename: 	hZipFile.h	
	
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

#ifndef ZIPFILE_H__
#define ZIPFILE_H__

namespace Heart
{
	class hZipFile : public hIFile
	{
	public:

		hBool				ReadAsync( void* pBuffer, hUint32 size );
		hBool				WriteAsync( const void* pBuffer, hUint32 size );
		hBool				SeekAsync( hUint64 offset, hdSeekOffset from );
        hUint32				Read( void* pBuffer, hUint32 size );
        hUint32				Write( const void* pBuffer, hUint32 size );
        hUint32     		Seek( hUint64 offset, hdSeekOffset from );
		hUint64				Tell();
		hUint64				Length();
		
		hBool				IsDone( hUint32* bytes ) const;

	private:

		friend class				hZipFileSystem;

		hZipFile();
		~hZipFile();

		void						ReadInternal( unzFile zip );
		void						WriteInternal( unzFile zip );
		void						SeekInternal( unzFile zip );
		
		union 
		{
			struct
			{
				void*				pReadBuffer_;
				hUint32				readSize_;
			};
			struct
			{
				const void*		    pWriteBuffer_;
				hUint32				writeSize_;
			};
			struct 
			{
				hUint64				seek_;
				hdSeekOffset			where_;
			};
		}							opData_;
		const hZipFileSystem*		pFileSystem_;
		unz64_file_pos				zipFilePos_;
		hUint64						filePos_;
		hUint64						size_;
		volatile hUint32			prevOpRes_;
		volatile hUint32			nextOP_;
	};
}

#endif // ZIPFILE_H__