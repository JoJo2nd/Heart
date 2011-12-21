/********************************************************************
	created:	2008/08/03
	created:	3:8:2008   11:22
	filename: 	GameDataBuilder.cpp
	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "GameDataBuilder.h"
#include "PackCommon.h"
#include "zip.h"
#include <algorithm>
#include <stdio.h>
#include <windows.h>


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
const std::string GameDataBuilder::OUTPUT_DIRECTORY = "gamedataPAK.zip";

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

GameDataBuilder::GameDataBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

GameDataBuilder::~GameDataBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GameDataBuilder::QuearyCPUInfo()
{
	GetSystemInfo( &system_info );
	nCores_ = system_info.dwNumberOfProcessors*2;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GameDataBuilder::CreateBuilderThreads()
{
	hcAssert( nCores_ > 0 );

//#ifdef _DEBUG
 	//nCores_ = 1;//easier to debug this for time being!
//#endif

	builderThreads_.resize( nCores_ );
	for ( hUint32 i = 0; i < nCores_; ++i )
	{
		builderThreads_[ i ].Begin( 
			"Builder Thread",
			Heart::Threading::Thread::PRIORITY_NORMAL,
			Heart::Device::Thread::ThreadFunc::bind< 
				GameDataBuilder, 
				&GameDataBuilder::ResourceBuilderTask >( this ), 
			NULL );
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool GameDataBuilder::ParseGamedataXml()
{
	if ( gamedataXmlDoc_.LoadFile( gamedataXML_.c_str(), TIXML_ENCODING_UTF8 ) )
	{
		//TODO: needs more error checking
		pGamedataElement_ = NULL;
		gamedataXmlDoc_.Accept( this );

		return hTrue;
	}
	else 
	{
		char buffer[2048];
		sprintf_s( buffer, 2048, "Error parsing %s:\n%s (line %d column %d)", 
			gamedataXML_.c_str(),
			gamedataXmlDoc_.ErrorDesc(), 
			gamedataXmlDoc_.ErrorRow(), 
			gamedataXmlDoc_.ErrorCol() );
		MessageBox( NULL, buffer, NULL, MB_OK );
	}

	return hFalse;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GameDataBuilder::MapToGamedataDirectory( std::string xmlPath )
{
	//replace all \ with /
	std::string::const_iterator iend = xmlPath.end();
	for ( std::string::iterator i = xmlPath.begin(); i != iend; ++i )
	{
		if ( *i == '\\' )
		{
			*i = '/';
		}
	}
	//find the last / in the path
	size_t slash = xmlPath.find_last_of( '/' );
	if ( slash != xmlPath.npos )
	{
		gamedataXML_ = xmlPath.substr( slash+1 );//+1 to exclude the /
		workingDir_ = xmlPath.erase( slash );
		SetCurrentDirectory( workingDir_.c_str() );
		char fullWorkingDir[MAX_PATH];
		GetCurrentDirectory( MAX_PATH, fullWorkingDir );
		workingDir_ = fullWorkingDir;
	}
	else
	{
		gamedataXML_ = xmlPath;
		workingDir_ = "";
	}

	//create the output zip file
	gamedataZipMutex_.Lock();

	gamedataZip_ = zipOpen( OUTPUT_DIRECTORY.c_str(), APPEND_STATUS_CREATE );

	gamedataZipMutex_.Unlock();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 GameDataBuilder::ResourceBuilderTask( void* /*param*/ )
{
	while ( true )
	{
		Sleep( 100 );

		GameResource resource;

		accessMutex_.Lock();

		if( resourcesToBuild_.size() == 0 )
		{
			accessMutex_.Unlock();
			if ( !stoppedPushResources_ )
			{
				continue;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			resource = *(resourcesToBuild_.begin());
			resourcesToBuild_.erase( resourcesToBuild_.begin() );
			accessMutex_.Unlock();
		}

		//TODO catch any errors
 		try
 		{
			//create resource builder
			ResourceBuilder* pRes = resource.buildDLLInfo_->creator_();

			//Set the Resource manager & other stuff
			pRes->SetResourceFileSystem( &resourceFileSystem_ );
			pRes->SetResourceName( resource.name_ );
			pRes->SetResourcePath( resource.outputPath_ );
			pRes->SetResourceDependenciesPath( resource.depPath_ );

			//set input file
			pRes->SetInputFilename( resource.inputResourcePath_ );

			//set params
			std::map< std::string, std::string >::iterator iend = resource.paramMap_.end();
			for( std::map< std::string, std::string >::iterator i = resource.paramMap_.begin(); i != iend; ++i )
			{
				pRes->SetParam( i->first, i->second );
			}
			
			//build
			hcPrintf( "Building Resource %s (param CRC 0x%08X)\n\tInput Path: %s\n\tOutput Path: %s", resource.name_.c_str(), pRes->GetParamHash(), resource.inputResourcePath_.c_str(), resource.outputPath_.c_str() );
			pRes->BuildResource();

			//add any dependencies 
			for ( ResourceBuilder::DependencyList::const_iterator i = pRes->GetDependencies().begin(); 
				  i != pRes->GetDependencies().end(); ++i )
			{
				accessMutex_.Lock();

				(*i)->buildDLLInfo_ = GetDllInfo( (*i)->dllName_ );
				if ( (*i)->buildDLLInfo_ == NULL )
				{
					accessMutex_.Unlock();
					throw std::exception( "Can't find dll Info for resource dependency" );
				}
				resourcesToBuild_.push_back( *(*i) );

				accessMutex_.Unlock();
			}

			//write data
			void* pOutputData = pRes->GetDataPtr();
			hUint32 outputSize = pRes->GetDataSize();

			gamedataZipMutex_.Lock();

			std::string path = resource.outputPath_;
			hUint32 pos;
			while( ( pos = path.find_first_of( '\\' ) ) < path.size() )
			{
				path[ pos ] = '/';
			}
			path += ".";
			path += resource.buildDLLInfo_->extension_;
			zipOpenNewFileInZip(
					gamedataZip_,
					path.c_str(),
					NULL,/*const zip_fileinfo* zipfi,*/
					NULL,/*const void* extrafield_local,*/
					0,/*uInt size_extrafield_local,*/
					NULL,/*const void* extrafield_global,*/
					0,/*uInt size_extrafield_global,*/
					NULL,/*const char* comment,*/
					Z_DEFLATED,
					Z_BEST_COMPRESSION
				);

			zipWriteInFileInZip( 
					gamedataZip_, 
					pOutputData, 
					outputSize 
				);

			zipCloseFileInZip( gamedataZip_ );

			gamedataZipMutex_.Unlock();

			//cache data

			//add to completed resources
			
			//delete resource builder
			resource.buildDLLInfo_->destroyer_( pRes );
 		}
		catch ( std::exception& e )
 		{
			MessageBox( NULL, e.what(), NULL, MB_OK );
 		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool GameDataBuilder::BuildGamedata( const char* gamedataxml, const char* configName )
{
	stoppedPushResources_ = false;

	huCRC32::Initialise();

	configName_ = configName;
	configFound_ = false;

	resourceFileSystem_.Initialise();
	MapToGamedataDirectory( gamedataxml );
	QuearyCPUInfo();
	CreateBuilderThreads();
	ParseGamedataXml();

	if ( !configFound_ )
	{
		hcPrintf( "Couldn't Find DLL plugin group for config \"%s\"", configName );
		return hFalse;
	}

	hBool working = hTrue;
	while ( working )
	{
		Sleep( 1000 );
		accessMutex_.Lock();
		if ( resourcesToBuild_.size() == 0 )
		{
			working = hFalse;
		}
		accessMutex_.Unlock();
	}

	stoppedPushResources_ = true;
	bool waitingOnWork = true;
	while( waitingOnWork )
	{
		waitingOnWork = false;
		for ( hUint32 i = 0; i < nCores_; ++i )
		{
			if ( builderThreads_[ i ].IsComplete() == false )
			{
				waitingOnWork = true;
			}
		}
	}

	gamedataZipMutex_.Lock();

	zipClose( gamedataZip_, NULL );

	gamedataZipMutex_.Unlock();

	return hFalse;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool GameDataBuilder::VisitEnter( const TiXmlDocument& doc )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool GameDataBuilder::VisitExit( const TiXmlDocument& doc )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool GameDataBuilder::VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute )
{
	//common elements first
	if ( strcmp( element.Value(), "resource" ) == 0 )
	{
		//check we have the 3 essential elements
		//for a resource
		std::string name,dll,input;
		if ( element.QueryStringAttribute( "name", &name ) != TIXML_SUCCESS ||
			 element.QueryStringAttribute( "dll", &dll ) != TIXML_SUCCESS ||
			 element.QueryStringAttribute( "input", &input ) != TIXML_SUCCESS )
		{
			throw std::exception( "unable to find name, dll & input attributes for resource" );
		}

		GameResource newResource;
		for ( const TiXmlAttribute* i = firstAttribute; i != NULL; i = i->Next() )
		{
			if ( strcmp( i->Name(), "name" ) == 0 )
			{
				newResource.name_ = i->Value();
				newResource.outputPath_ = currentZipPath_ + i->Value();
			}
			else if ( strcmp( i->Name(), "dll" ) == 0 )
			{
				newResource.dllName_ = i->Value();
				newResource.buildDLLInfo_ = GetDllInfo( i->Value() );
				if ( !newResource.buildDLLInfo_ )
				{
					throw std::exception( "cant find dll" );
				}
			}
			else if ( strcmp( i->Name(), "input" ) == 0 )
			{
				newResource.inputResourcePath_ = i->Value();
			}
			else
			{
				//extra param, stick it in the param map
				std::string lName = i->Name();
				std::string lValue = i->Value();
				std::transform( lName.begin(), lName.end(), lName.begin(), tolower );
				std::transform( lValue.begin(), lValue.end(), lValue.begin(), tolower );
				newResource.paramMap_[ i->Name() ] = i->Value();
			}
		}

		accessMutex_.Lock();

		//before push back sort last few parameters
		newResource.depPath_ = currentZipPath_ + newResource.name_ + "_dep";

		resourcesToBuild_.push_back( newResource );

		accessMutex_.Unlock();
	}
	else if ( strcmp( element.Value(), "folder" ) == 0 )
	{
		const char* folderName = element.Attribute( "name" );
		if ( !folderName )
		{
			throw std::exception( "folder name not found" );
		}

		pathStack_.push( std::string( folderName ) + std::string( "\\" ) );
		currentZipPath_ += pathStack_.top();
		
	}
	else if ( strcmp( element.Value(), "dll" ) == 0 )
	{
		if ( !AddDllInfo( element.Attribute( "name" ), element.Attribute( "path" ), element.Attribute( "ext" ) ) )
		{
			throw std::exception( "unable to add dll info" );
		}
	}
	else if ( strcmp( element.Value(), "plugins" ) == 0 )
	{
		if ( element.Attribute( "group" ) && strcmp( element.Attribute( "group" ), configName_.c_str() ) == 0 )
		{
			configFound_ = hTrue;
		}
		else
		{
			return false;
		}
	}
	else if ( strcmp( element.Value(), "gamedata" ) == 0 )
	{
		pGamedataElement_ = &element;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool GameDataBuilder::VisitExit( const TiXmlElement& element )
{
	if ( strcmp( element.Value(), "folder" ) == 0 )
	{
		//pop last element off
		size_t slash = currentZipPath_.find( pathStack_.top() );
		if ( slash != currentZipPath_.npos )
		{
			currentZipPath_.erase( slash );
		}
		else
		{
			currentZipPath_.clear();
		}
		pathStack_.pop();
	}
	else if ( strcmp( element.Value(), "plugins" ) == 0 )
	{
	}
	else if ( strcmp( element.Value(), "gamedata" ) == 0 )
	{
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool GameDataBuilder::Visit( const TiXmlDeclaration& declaration )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool GameDataBuilder::Visit( const TiXmlText& text )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool GameDataBuilder::AddDllInfo( const std::string& name, const std::string& path, const std::string& ext )
{
	DllInfo info;
	info.name_ = name;
	info.path_ = path;
	info.extension_ = ext;

	//create full directory path so extra dlls can be loaded
	char wd[ 2048 ];
	GetCurrentDirectory( 2048, wd );

	std::string fullpath( wd ); 
	fullpath += '\\';
	fullpath += info.path_;

	//check the file exists
	info.builderModule_ = LoadLibraryEx( fullpath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
	if ( info.builderModule_ == NULL )
	{
		std::string msg = "unable to load dll library : ";
		msg += fullpath;
		throw std::exception( msg.c_str() );
	}

	//get the proc addresses
	info.creator_ = (ResourceCreator)GetProcAddress( info.builderModule_, "CreateResourceBuilder" );
	info.destroyer_ = (ResourceDestroyer)GetProcAddress( info.builderModule_, "DestroyResourceBuilder" );
	if ( info.creator_ == NULL ||
		 info.destroyer_ == NULL )
	{
		throw std::exception( "unable to load Proc Addresses from dll" );
	}

	//testing 1,2,3
	ResourceBuilder* pPtr = info.creator_();
	//pPtr->BuildResource();
	info.destroyer_( pPtr );

	loadedPlugins_.push_back( info );
	return hTrue;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const DllInfo* GameDataBuilder::GetDllInfo( const std::string& name )
{
	std::list< DllInfo >::iterator found = std::find( loadedPlugins_.begin(), loadedPlugins_.end(), name );

	if ( found != loadedPlugins_.end() )
	{
		return &(*found);
	}

	return NULL;
}
