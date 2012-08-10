/********************************************************************

	filename: 	hDriveFileSystem.h	
	
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

#ifndef DRIVEFILESYSTEM_H__
#define DRIVEFILESYSTEM_H__

namespace Heart
{
	class hDriveFileSystem : public hIFileSystem
	{
	public:
		hDriveFileSystem() 
		{
		}
		~hDriveFileSystem()
		{
		}

		/**
		* OpenFile 
		*
		* @param 	const hChar * filename
		* @param 	FileMode mode
		* @return   hIFile*
		*/
		hIFile*	OpenFile( const hChar* filename, hFileMode mode ) const;
        hIFile* OpenFileRoot( const hChar* filename, hFileMode mode ) const;
		
		/**
		* CloseFile 
		*
		* @return   void
		*/
		void	CloseFile( hIFile* pFile ) const;

		/**
		* EnumerateFiles 
		*
		* @param 	const hChar * path
		* @param 	EnumerateFilesCallback fn
		* @return   void
		*/
		void	EnumerateFiles( const hChar* path, hEnumerateFilesCallback fn ) const;
        void    CreateDirectory(const hChar* path);
	};
}

#endif // DRIVEFILESYSTEM_H__