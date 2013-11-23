/********************************************************************

	filename: 	hZipFileSystem.h	
	
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

#ifndef ZIPFILESYSTEM_H__
#define ZIPFILESYSTEM_H__

namespace Heart
{
	
	class HEART_DLLEXPORT hZipFileSystem : public hIFileSystem
	{
	public:

		hZipFileSystem(const hChar* zipFile) 
            : zipFileHandle_( NULL )
            , openHandles_(0)
            , basePathLen_(0)
		{
            Initialise(zipFile);
		}

		~hZipFileSystem()
		{
            Destory();
		}

        hBool IsOpen() const { 
            return zipFileHandle_ != 0;
        }

		/**
		* OpenFile 
		*
		* @param 	const hChar * filename
		* @param 	FileMode mode
		* @return   hIFile*
		*/
		virtual hIFile*	OpenFile( const hChar* filename, hFileMode mode ) const;
		
		/**
		* CloseFile 
		*
		* @return   void
		*/
		virtual void	CloseFile( hIFile* pFile ) const;

		/**
		* EnumerateFiles 
		*
		* @param 	const hChar * path
		* @param 	EnumerateFilesCallback fn
		* @return   void
		*/
		virtual void	EnumerateFiles( const hChar* path, hEnumerateFilesCallback fn ) const;
        void            CreateDirectory(const hChar* /*path*/) {}
        virtual hBool   WriteableSystem() const { return hFalse; }

	private:

        void			Initialise( const hChar* zipFile );
        void			Destory();

		zlib_filefunc64_def					zipFileIODefs_;
		unzFile								zipFileHandle_;
        hUint                               basePathLen_;
        mutable hUint32                     openHandles_;
	};

}

#endif // ZIPFILESYSTEM_H__