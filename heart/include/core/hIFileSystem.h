/********************************************************************

	filename: 	hIFileSystem.h	
	
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

#ifndef hcFileManager_h__
#define hcFileManager_h__

namespace Heart
{
	class hIFile;

	enum hFileMode
	{
		FILEMODE_READ,
		FILEMODE_WRITE,

		FILEMODE_MAX
	};

	struct hFileInfo
	{
		const hChar*	path_;
		const hChar*	name_;
		hByte			directory_;
	};

	typedef huFunctor< hBool(*)(const hFileInfo*) >::type		hEnumerateFilesCallback;

	class HEART_DLLEXPORT hIFileSystem
	{
	public:
		virtual ~hIFileSystem() {}

		/**
		* OpenFile 
		*
		* @param 	const hChar * filename
		* @param 	FileMode mode
		* @return   hIFile*
		*/
		virtual hIFile*	OpenFile( const hChar* filename, hFileMode mode ) const = 0;
        virtual hIFile* OpenFileRoot( const hChar* filename, hFileMode mode ) const = 0;
		
		/**
		* CloseFile 
		*
		* @return   void
		*/
		virtual void	CloseFile( hIFile* pFile ) const = 0;

		/**
		* EnumerateFiles 
		*
		* @param 	const hChar * path
		* @param 	EnumerateFilesCallback fn
		* @return   void
		*/
		virtual void	EnumerateFiles( const hChar* path, hEnumerateFilesCallback fn ) const = 0;
        virtual void    CreateDirectory(const hChar* path) = 0;
        virtual hBool   WriteableSystem() const { return hTrue; }

	};
}

#endif // hcFileManager_h__