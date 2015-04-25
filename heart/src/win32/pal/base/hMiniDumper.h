/********************************************************************
	created:	2008/07/06
	created:	6:7:2008   20:15
	filename: 	huMiniDumper.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef huMiniDumper_h__
#define huMiniDumper_h__

#include <winsock2.h>
#include <windows.h>
#include <dbghelp.h>

//-----------------------------------------------------------------------
///
/// @class huMiniDumper
/// @brief The Mini Dumper class creates crash dumps for a program, to use it, simply 
///	declare one at the of you're main file. It will create crash dump files using a {exe_name}_{crashnumber}.dmp format
/// @author James Moran
/// @date [13/7/2008]
/// @version 1.0
///
//-----------------------------------------------------------------------
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