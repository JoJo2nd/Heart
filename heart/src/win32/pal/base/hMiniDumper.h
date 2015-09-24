/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef huMiniDumper_h__
#define huMiniDumper_h__

#include <winsock2.h>
#include <windows.h>
#include <dbghelp.h>

class huMiniDumper
{
public:

	/**
	* 
	*
	* @return   	
	*/
											huMiniDumper();

private:

	LONG									WriteMiniDump( _EXCEPTION_POINTERS* pexcoptioninfo );
	static LONG WINAPI						Handler( struct _EXCEPTION_POINTERS* pexcoptioninfo );
	void									SetDumpFilename();
	MINIDUMP_USER_STREAM_INFORMATION*		GetUserStreamArray();

	static huMiniDumper*					pDumper_;
	static const TCHAR						DllFilename_[];
	_EXCEPTION_POINTERS*					pExceptionPointers_;
	TCHAR									DumpPath_[ MAX_PATH ];
	TCHAR									AppPath_[ MAX_PATH ];
	TCHAR									AppBaseName_[ MAX_PATH ];

};

inline MINIDUMP_USER_STREAM_INFORMATION* huMiniDumper::GetUserStreamArray()
{
	return NULL;
}

#endif // huMiniDumper_h__