/********************************************************************
	created:	2008/08/01
	created:	1:8:2008   13:48
	filename: 	ResourceFileSystem.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef RESOURCEFILESYSTEM_H__
#define RESOURCEFILESYSTEM_H__

#include "hcTypes.h"
#include "Mutex.h"
#include <list>

enum FileMode
{
	FILEMODE_READ,
	FILEMODE_WRITE,

	FILEMODE_MAX
};

class DLLEXPORT ResourceFile
{
public:

	enum SeekOffset
	{
		SO_SEEK_CUR,//Seek from current file position
		SO_SEEK_SET,//Seek form beginning of file
		SO_SEEK_END,//Seek from end of file
	};

	virtual hUint32			Read( void* pDst, hUint32 bytesToRead ) = 0;
	virtual hUint32			Tell() = 0;
	virtual hUint32			Seek( hInt32 toSeek, SeekOffset seekOff ) = 0;
	virtual hUint32			Size() = 0;
	virtual hUint32			Write( void* pSrc, hUint32 bytesToWrite ) = 0;
	virtual const hChar*	GetFileName() const = 0;

protected:
	friend class ResourceFileSystem;

	virtual void			SetMode( FileMode mode ) = 0;
	virtual	hBool			Open( const hChar* filename, FileMode mode ) = 0;
	virtual void			Close() = 0;

	ResourceFile() {};
	ResourceFile( const ResourceFile& c ) {}
	virtual ~ResourceFile() {}
};

class DLLEXPORT ResourceFileSystem
{
public:

	ResourceFileSystem();
	~ResourceFileSystem();

	void			Initialise();
	void			CacheData();

	ResourceFile*	OpenFile( const hChar* filename, FileMode mode );
	hBool			CloseFile( ResourceFile* file );

private:	
#pragma warning ( push )
#pragma warning ( disable:4251 )

	typedef std::list< ResourceFile* >	MemFileList;

	Heart::Threading::Mutex				mutex_;
	MemFileList							memoryFiles_;

#pragma warning ( pop )
};

#endif // RESOURCEFILESYSTEM_H___H__