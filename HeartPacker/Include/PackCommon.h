/********************************************************************
	created:	2008/08/04
	created:	4:8:2008   14:19
	filename: 	PackCommon.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef PACKCOMMON_H__
#define PACKCOMMON_H__

//#include "hcMemory.h"
#include <map>
#include <string>
#include <stdio.h>
#include <ctype.h>

//#include "hmMatrix.h"
#include <windows.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

inline hUint32 ReadPastWhiteSpace( const char* line )
{
	hUint32 i;
	for ( i = 0; isspace( (*line) ) != 0; ++line, ++i );
	return i;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

inline void TrimTrailingWhiteSpace( char* line )
{
	for ( line += strlen( line ) - 1; isspace( (*line) ); --line )
	{
		(*line) = 0;
	}
}

extern int BoneIDSorter( const void*, const void* );
extern hUint32 getBoneID( hUint32, const hUint32*, hUint32 );

#define DATA_CACHE_DIR			".PackerCache"

struct ResourceTimeStamp
{
	__int64 modifiedTime_;
};

struct GameResource
{
	const struct DllInfo*						buildDLLInfo_;		//!< pointer to the dll info to build this resource with (creates the resource builder)
	std::string									dllName_;			//!< String ID of the DllInfo class
	std::string									inputResourcePath_; //!< path of the input file for this dll
	std::map< std::string, std::string >		paramMap_;			//!< map of extra parameter to pass to the resource builder class
	std::string									name_;				//!< name of the resource without extention
	std::string									outputPath_;		//!< where in the final zip the resource will live
	std::string									depPath_;			//!< path where any dependencies of this resource will live
	std::string									intermidatePath_;	//!< the path where the resource builder must place the successfully built resource for caching
	class ResourceBuilder*						pBuilder_;			//!< class crated by the dll to build our resource
	/*
		TODO: cache data inc..
			* dll modified date
			* input file modified data
			* parameter map
	*/
};

#endif // PACKCOMMON_H__
