/********************************************************************
	created:	2008/08/01
	created:	1:8:2008   13:48
	filename: 	GameDataBuilder.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef GameDataBuilder_h__
#define GameDataBuilder_h__

#include "Heart.h"
#include "Thread.h"
#include "Mutex.h"
#include "tinyxml/tinyxml.h"
#include "zip.h"
#include "ResourceBuilder.h"
#include "ResourceFileSystem.h"
#include <list>
#include <string>
#include <map>
#include <vector>
#include <stack>

class ResourceBuilder;

struct DllInfo
{
	std::string									name_;
	std::string									path_;
	std::string									extension_;
	HMODULE										builderModule_;	
	ResourceCreator								creator_;//proc from Dll
	ResourceDestroyer							destroyer_;//proc from Dll

	bool operator == ( const std::string& name )
	{
		return name_ == name;
	}
};

class GameDataBuilder : public TiXmlVisitor
{
public:

													GameDataBuilder();
													~GameDataBuilder();

	//version 2.0
	hBool											BuildGamedata( const char* gamedataxml, const char* configName );

private:
	
	typedef	std::list< std::string >				StringList;
	typedef std::list< std::string >::iterator		StringListItr;

	// version 2.0d
	void											QuearyCPUInfo();
	void											MapToGamedataDirectory( std::string xmlPath );
	void											CreateBuilderThreads();
	hUint32											ResourceBuilderTask( void* param );
	hBool											ParseGamedataXml();
	hBool											AddDllInfo( const std::string& name, const std::string& path, const std::string& ext );
	const DllInfo*									GetDllInfo( const std::string& name );

	//XML parser functions
	virtual bool									VisitEnter( const TiXmlDocument& doc );
	virtual bool									VisitExit( const TiXmlDocument& doc );
	virtual bool									VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute );
	virtual bool									VisitExit( const TiXmlElement& element );
	virtual bool									Visit( const TiXmlDeclaration& declaration );
	virtual bool									Visit( const TiXmlText& text );
	// don't parse comments & unknown elements

	static const std::string						OUTPUT_DIRECTORY;

	SYSTEM_INFO										system_info;
	hUint32											nCores_;			//!< so we can push jobs onto different cores
	std::list< DllInfo >							loadedPlugins_;
	std::vector< Heart::Threading::Thread >			builderThreads_;
	Heart::Threading::Mutex							accessMutex_;
	std::string										gamedataXML_;		//!< XML we parse for all our initial resources (does not include dependencies)
	std::string										workingDir_;
	std::string										configName_;
	hBool											configFound_;
	TiXmlDocument									gamedataXmlDoc_;
	std::string										currentZipPath_;
	std::stack< std::string >						pathStack_;
	const TiXmlElement*								pGamedataElement_;
	std::list< GameResource >						resourcesToBuild_;	//!< list of resources to build, dependencies are appended to this list
	Heart::Threading::Mutex							gamedataZipMutex_;
	zipFile											gamedataZip_;
	volatile bool									stoppedPushResources_;
	ResourceFileSystem								resourceFileSystem_;
	//TODO: Dependency Map
};

#endif // GameDataBuilder_h__