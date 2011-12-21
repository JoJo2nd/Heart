/********************************************************************
	created:	2008/08/01
	created:	1:8:2008   13:49
	filename: 	ArchiveBuilder.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef ArchiveBuilder_h__
#define ArchiveBuilder_h__


#include "Heart.h"
#include <list>
#include <string>
#include <map>
#include "hcArchive.h"
#include "PackCommon.h"

class ResourceBuilder;

struct ResourceInfo
{
	typedef std::map< std::string, std::string >	ParamMap;

	hcResourceType		Type_;
	std::string			Name_;
	ParamMap			Params_;
	
};

class ArchiveBuilder
{
public:

	typedef std::list< ResourceInfo >						ResInfoList;
	typedef std::list< ResourceInfo >::iterator				ResInfoListItr;
	typedef std::list< ResourceBuilder* >					ResBuilderList;
	typedef std::list< ResourceBuilder* >::iterator			ResBuilderListItr;
	typedef std::list< ResourceInfo >::const_iterator		ConstResInfoListItr;
	typedef std::list< ResourceBuilder* >::const_iterator	ConstResBuilderListItr;
	typedef std::map< std::string, hcResourceType >			ResTypeLookUp;
	typedef std::map< std::string, ResourceTimeStamp >		ResTimeStamps;

															ArchiveBuilder();
															~ArchiveBuilder();

	hBool													OpenArchiveDataList( const char* filename );
	hBool													BuildResources();
	hBool													ExportBuildResources();
	hBool													HadError() const;
	void													GetErrorStrings( std::list< std::string >* errorstrings ) const;
	void													SetErrorString( std::string error );
	void													AddDependency( ResourceBuilder* buildingRes, ResourceBuilder* newresource );
	std::string												GetArchiveTempFileNamePrefix();

private:

	hBool													CheckValidArchiveDataList();
	ResourceBuilder*										CreateResourceBuilder( const char* type );
	void													GetResourceParams( const char* paramline, ResourceBuilder* res );
	hBool													GetOneParam( const char* paramline, char* name, char* value, hUint32& read );
	void													GetResourceTimestamps();
	hBool 													CheckResourceTimestamp( const std::string& resName, ResourceTimeStamp toCheck, ResBuilderListItr& resItr );
	void													SetResourceTimestamp( const std::string& resName, ResourceTimeStamp stamp );
	void													WriteTimestamps();
	void													WriteCacheData();
	hByte*													GetCacheData( const std::string& resName, hUint32& size );
	void													AddCachedDependencies( ResBuilderListItr& i );

	ResInfoList												ResourceList_;
	ResBuilderList											ResourceBuilders_;
	ResBuilderList											AddedDependencies_;// dependencies dependency [8/29/2008 James]
	ResTypeLookUp											ResTypes_;
	FILE*													ArchiveFile_;
	hByte*													ArchiveData_;
	hcArchiveHeader											Header_;
	hByte*													CompressedArchiveData_;
	hUint32													ArchiveSize_;
	hUint32													CompressedSize_;
	std::string												ArchiveName_;
	std::string												ErrorString_;
	hBool													HadError_;
	ResTimeStamps											resourceTimeStamps_;

};

inline void ArchiveBuilder::SetErrorString( std::string error )
{
	ErrorString_ = error;
	HadError_ = hTrue;
}

inline std::string ArchiveBuilder::GetArchiveTempFileNamePrefix()
{
	std::string r = DATA_CACHE_DIR;
	r += "\\";
	r += ArchiveName_ + "\\.tmp\\";
	return r;
}

#endif // ArchiveBuilder_h__
