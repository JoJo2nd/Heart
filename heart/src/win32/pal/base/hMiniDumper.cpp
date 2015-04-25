/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hMiniDumper.h"
#include "base/hTypes.h"
#include <tchar.h>

typedef BOOL ( WINAPI *MINIDUMPWRITEDUMP)( HANDLE process, 
										   DWORD pid, 
										   HANDLE file, 
										   MINIDUMP_TYPE DumpType, 
										   CONST PMINIDUMP_EXCEPTION_INFORMATION exceptionparam,
										   CONST PMINIDUMP_USER_STREAM_INFORMATION userstreamparam,
										   CONST PMINIDUMP_CALLBACK_INFORMATION callbackparam );

huMiniDumper*					huMiniDumper::pDumper_ = NULL;
CONST TCHAR						huMiniDumper::DllFilename_[] = "DBGHELP.DLL";

/*/////////////////////////////////////////////////////////////////////////

function: huMiniDumper - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
huMiniDumper::huMiniDumper()
{
	hcAssert( !pDumper_ );

	if ( !pDumper_ )
	{
		SetUnhandledExceptionFilter( Handler );
		pDumper_ = this;
	}
}

/*/////////////////////////////////////////////////////////////////////////

function: WriteMiniDump - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
LONG huMiniDumper::WriteMiniDump( _EXCEPTION_POINTERS* pexceptioninfo )
{
	LONG retval = EXCEPTION_CONTINUE_SEARCH;
	pExceptionPointers_ = pexceptioninfo;

	// find the dbghelp DLL [7/6/2008 James]

	HMODULE dll = NULL;
	TCHAR dbghelppath[ MAX_PATH ];

	if ( GetModuleFileName( NULL, AppPath_, MAX_PATH ) )
	{
		TCHAR* pslash = _tcsrchr( AppPath_, '\\' );
		if ( pslash )
		{
			_tcscpy( AppBaseName_, pslash + 1 );
			*( pslash + 1 ) = 0;
		}

		_tcscpy( dbghelppath, AppPath_ );
		_tcscat( dbghelppath, DllFilename_ );
		dll = LoadLibrary( dbghelppath );
	}

	if ( dll == NULL )
	{
		dll = LoadLibrary( DllFilename_ );
	}

	LPCTSTR res = NULL;

	if ( dll )
	{
		MINIDUMPWRITEDUMP minidumpwritedump = reinterpret_cast< MINIDUMPWRITEDUMP >( GetProcAddress( dll, "MiniDumpWriteDump" ) );

		if ( minidumpwritedump )
		{
			SetDumpFilename();

			HANDLE file = CreateFile( DumpPath_, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

			if ( file != INVALID_HANDLE_VALUE )
			{
				_MINIDUMP_EXCEPTION_INFORMATION exinfo;

				exinfo.ThreadId = GetCurrentThreadId();
				exinfo.ExceptionPointers = pExceptionPointers_;
				exinfo.ClientPointers = NULL;

				// begin the dump [7/6/2008 James]
				BOOL v = minidumpwritedump( GetCurrentProcess(),
											GetCurrentProcessId(), 
											file,
											MiniDumpNormal,
											&exinfo,
											GetUserStreamArray(),
											NULL );

				if ( v )
				{
					res = NULL;
					retval = EXCEPTION_EXECUTE_HANDLER;
				}

				CloseHandle( file );
			}
		}
	}

	TCHAR tmp[ MAX_PATH ];
	sprintf_s( tmp, MAX_PATH, "Something went horribly wrong, causing a crash dump to be made at %s", DumpPath_ );

	MessageBox( NULL, tmp, NULL, MB_OK );

	TerminateProcess( GetCurrentProcess(), 0 );

	return retval;
}

/*/////////////////////////////////////////////////////////////////////////

function: Handler - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
LONG WINAPI huMiniDumper::Handler( struct _EXCEPTION_POINTERS* pexcoptioninfo )
{
	LONG retval = EXCEPTION_CONTINUE_SEARCH;

	if ( !pDumper_ )
	{
		return retval;
	}

	return pDumper_->WriteMiniDump( pexcoptioninfo );
}

/*/////////////////////////////////////////////////////////////////////////

function: SetDumpFilename - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
void huMiniDumper::SetDumpFilename()
{
	TCHAR scratch[ MAX_PATH ];

	for ( hUint32 i = 0; i < UINT_MAX; ++i )
	{
		sprintf_s( scratch, MAX_PATH, "%s%s.dump-%d.dmp", AppPath_, AppBaseName_, i );
	
		HANDLE file = CreateFile( scratch, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

		if ( file == INVALID_HANDLE_VALUE )
		{
			strcpy_s( DumpPath_, MAX_PATH, scratch );
			return;
		}
	}

	sprintf_s( scratch, MAX_PATH, "%s%slastest.dmp" );
	strcpy_s( DumpPath_, MAX_PATH, scratch );

}