/********************************************************************

	filename: 	gdGameDatabaseObject.cpp	
	
	Copyright (c) 12:10:2011 James Moran
	
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

namespace GameData
{
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    /*
        VAssist suggests using __declspec(selectany) here?
    */
    static const gdWchar plugInsFolderName_[]  = L"plug_ins";
    static const gdWchar cacheFolderName_[]    = L"cache";
    static const gdWchar outputFolderName_[]   = L"GAMEDATA";
    static const gdWchar remapTableName_[]     = L"RRT";
    static const gdWchar databaseFilename_[]   = L"gamedata.db";

// #define plugInsFolderName_ (L"plug_ins")
// #define cacheFolderName_ (L"cache")
// #define outputFolderName_ (L"GAMEDATA")
// #define remapTableName_ (L"RRT")
// #define databaseFilename_ (L"gamedata.db")

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdGameDatabaseObject::gdGameDatabaseObject()
    {

    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdGameDatabaseObject::~gdGameDatabaseObject()
    {
        CleanUpMemoryResources();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::Create( const gdWchar* destPath )
    {
        using namespace boost::filesystem;

        databaseFolderPath_ = path( destPath );
        plugInsPath_ = databaseFolderPath_ / plugInsFolderName_;
        cachePath_ = databaseFolderPath_ / cacheFolderName_;
        databasePath_ = databaseFolderPath_ / databaseFilename_;
        SetOutputPath( databaseFolderPath_.generic_string().c_str() );

        if ( !exists( databaseFolderPath_ ) )
        {
            create_directory( databaseFolderPath_ );
        }

        if ( !exists( plugInsPath_ ) )
        {
            create_directory( plugInsPath_ );
        }

        if ( !exists( cachePath_ ) )
        {
            create_directory( cachePath_ );
        }

        if ( !exists( outputPath_ ) )
        {
            create_directory( outputPath_ );
        }

        dataSourcePath_ = databaseFolderPath_.generic_string();
        databaseFilepath_ = databasePath_.generic_string();

        LoadPlugInsDirectory();

        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void gdGameDatabaseObject::CleanUpMemoryResources()
    {
        for ( ResourceMapType::iterator i = resourceMap_.begin(), iend = resourceMap_.end(); 
            i != iend; ++i )
        {
            delete i->second;
        }

        for ( gdResorucePlugInMap::iterator i = loadedPlugIns_.begin(), iend = loadedPlugIns_.end();
            i != iend; ++i )
        {
            delete i->second;
        }

        loadedPlugIns_.clear();
        resourceMap_.clear();
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::Save() const
    {
        std::ofstream ofs( databaseFilepath_.c_str() );
        try
        {
            // if *this is passed its an invalid xml tag, hence dbroot is
            // created.
            const gdGameDatabaseObject& dbroot = *this;
            // save data to archive
            boost::archive::xml_oarchive oa(ofs);
            // write class instance to archive
            oa << BOOST_SERIALIZATION_NVP( dbroot/**this*/ );
            // archive and stream closed when destructors are called
        }
        catch( boost::archive::xml_archive_exception e )
        {
            return gdERROR_UNHANDLED_SAVE_EXPECTION;
        }

        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::Load( const gdWchar* databaserootfolder )
    {
        using namespace boost::filesystem;
        gdError err = gdERROR_OK;

        databaseFolderPath_ = path( databaserootfolder );
        databasePath_ = databaseFolderPath_ / databaseFilename_;

        std::ifstream ofs( databasePath_.c_str() );
        try
        {
            // if *this is passed its an invalid xml tag, hence dbroot is
            // created.
            gdGameDatabaseObject& dbroot = *this;
            // save data to archive
            boost::archive::xml_iarchive oa(ofs);
            // write class instance to archive
            oa >> BOOST_SERIALIZATION_NVP( dbroot/**this*/ );
            // archive and stream closed when destructors are called
        }
        catch( boost::archive::xml_archive_exception e )
        {
            return gdERROR_UNHANDLED_SAVE_EXPECTION;
        }

        plugInsPath_ = databaseFolderPath_ / plugInsFolderName_;
        cachePath_ = databaseFolderPath_ / cacheFolderName_;
        if ( dataOutputPath_.empty() )
        {
            SetOutputPath( databaseFolderPath_.generic_string().c_str() );
        }
        else 
        {
            SetOutputPath( dataOutputPath_.c_str() );
        }

        if ( !exists( databaseFolderPath_ ) )
        {
            create_directory( databaseFolderPath_ );
        }

        if ( !exists( plugInsPath_ ) )
        {
            create_directory( plugInsPath_ );
        }

        if ( !exists( cachePath_ ) )
        {
            create_directory( cachePath_ );
        }

        if ( !exists( outputPath_ ) )
        {
            create_directory( outputPath_ );
        }

        err = LoadPlugInsDirectory();
        if ( err != gdERROR_OK )
            return err;

        // ValidateAfterLoad() won't fail, its just makes
        // sure data (e.g. resource parameters) are up-to-date
        // with any external changes.
        err = ValidateAfterLoad();
        if ( err != gdERROR_OK )
            return err;

        return err;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const gdChar* gdGameDatabaseObject::GetOutputPath()
    {
        return dataOutputPath_.c_str();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::SetOutputPath( const gdChar* path )
    {
        outputPath_ = path;
        outputPath_ = outputPath_ / outputFolderName_;
        dataOutputPath_ = path;

        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::ValidateAfterLoad()
    {
        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::AddResource( 
        const gdChar* inputFile, 
        const gdChar* path, 
        const gdChar* name, 
        gdUniqueResourceID* resourceID )
    {
        gdError err = gdERROR_OK;

        if ( !resourceID )
            return gdERROR_INVALID_PARAM;

        err = resourceID->Initialise( path, name );
        if ( err != gdERROR_OK )
            return err;

        if ( resourceMap_.find( resourceID->GetResourceCRCID() ) != resourceMap_.end() )
            return gdERROR_RESOURCE_NAME_CRC_CONFLICT;

        gdResourceInfo* resInfo = new gdResourceInfo();

        resInfo->SetResourceID( *resourceID );
        resInfo->SetInputFilePath( inputFile );

        resourceMap_[resourceID->GetResourceCRCID()] = resInfo;
        //resourceMap_.push_back( resInfo );

        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::RemoveResource( const gdUniqueResourceID& resourceID )
    {
        ResourceMapType::iterator res = resourceMap_.find( resourceID.GetResourceCRCID() );
        if ( res == resourceMap_.end() )
        {
            return gdERROR_RESOURCE_NOT_FOUND;
        }

        delete res->second;
        resourceMap_.erase( res );

        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::GetResourceIDByCRC( gdUint32 crc, gdUniqueResourceID* outResID )
    {
        if ( !outResID )
            return gdERROR_INVALID_PARAM;

        ResourceMapType::iterator res = resourceMap_.find( crc );
        if ( res == resourceMap_.end() )
        {
            return gdERROR_RESOURCE_NOT_FOUND;
        }

        *outResID = *res->second->GetResourceID();

        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::GetResourceByCRC( gdUint32 crc, gdResourceInfo** outResID )
    {
        if ( !outResID )
            return gdERROR_INVALID_PARAM;

        ResourceMapType::iterator res = resourceMap_.find( crc );
        if ( res == resourceMap_.end() )
        {
            return gdERROR_RESOURCE_NOT_FOUND;
        }

        *outResID = res->second;

        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdUint gdGameDatabaseObject::GetResourceIDArray( gdResourceIDArray* outArray )
    {
        gdUint size = resourceMap_.size();
        outArray->reserve( resourceMap_.size() );
        
        for ( ResourceMapType::iterator i = resourceMap_.begin(), iend = resourceMap_.end(); 
            i != iend; ++i )
        {
            outArray->push_back( *i->second->GetResourceID() );
        }

        return size;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdUint gdGameDatabaseObject::GetAvailableResoruceTypes( gdStringArray* outArray ) const
    {
        gdUint size = loadedPlugIns_.size();
        outArray->reserve( size );

        for ( gdResorucePlugInMap::const_iterator i = loadedPlugIns_.begin(), iend = loadedPlugIns_.end();
                i != iend; ++i )
        {
            outArray->push_back( i->first );
        }

        return size;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const gdChar* gdGameDatabaseObject::GetResourceTypeNameByIndex( gdUint idx ) const
    {
        gdUint size = loadedPlugIns_.size();
        gdUint ret = 0;

        for ( gdResorucePlugInMap::const_iterator i = loadedPlugIns_.begin(), iend = loadedPlugIns_.end();
            i != iend; ++i, ++ret )
        {
            if ( ret == idx )
            {
                return i->first.c_str();
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::IsResourceTypeNameValid( const gdChar* typeName ) const
    {
        if ( !typeName )
            return gdERROR_INVALID_PARAM;

        for ( gdResorucePlugInMap::const_iterator i = loadedPlugIns_.begin(), iend = loadedPlugIns_.end();
            i != iend; ++i )
        {
            if ( strcmp( i->first.c_str(), typeName ) == 0 )
            {
                return gdERROR_OK;
            }
        }

        return gdERROR_INVALID_PARAM;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::IsMoveResourceValid( const gdUniqueResourceID& resID, const gdChar* newPath )
    {
        ResourceMapType::iterator res = resourceMap_.find( resID.GetResourceCRCID() );

        if ( res == resourceMap_.end() )
            return gdERROR_RESOURCE_NOT_FOUND;

        gdUniqueResourceID newResID = resID;
        newResID.SetResourcePath( newPath );

        res = resourceMap_.find( newResID.GetResourceCRCID() );

        if ( res != resourceMap_.end() )
            return gdERROR_RESOURCE_NAME_CRC_CONFLICT;
    
        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::MoveResource( const gdUniqueResourceID& resID, const gdChar* newPath, gdUint32* outCRC )
    {
        gdError err = IsMoveResourceValid( resID, newPath );

        if ( err != gdERROR_OK )
            return err;

        ResourceMapType::iterator res = resourceMap_.find( resID.GetResourceCRCID() );
        gdResourceInfo* resPtr = res->second;
        gdUniqueResourceID newResID = resID;
        newResID.SetResourcePath( newPath );

        //remove old entry
        resourceMap_.erase( res );

        //add new entry
        resPtr->SetResourceID( newResID );
        resourceMap_[newResID.GetResourceCRCID()] = resPtr;

        *outCRC = newResID.GetResourceCRCID();

        return err;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::IsRenameResourceValid( const gdUniqueResourceID& resID, const gdChar* newName )
    {
        ResourceMapType::iterator res = resourceMap_.find( resID.GetResourceCRCID() );

        if ( res == resourceMap_.end() )
            return gdERROR_RESOURCE_NOT_FOUND;

        gdUniqueResourceID newResID = resID;
        newResID.SetResourceName( newName );

        res = resourceMap_.find( newResID.GetResourceCRCID() );

        if ( res != resourceMap_.end() )
            return gdERROR_RESOURCE_NAME_CRC_CONFLICT;

        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::RenameResource( const gdUniqueResourceID& resID, const gdChar* newName, gdUint32* outCRC )
    {
        if ( !outCRC )
            return gdERROR_INVALID_PARAM;

        gdError err = IsRenameResourceValid( resID, newName );

        if ( err != gdERROR_OK )
            return err;

        ResourceMapType::iterator res = resourceMap_.find( resID.GetResourceCRCID() );
        gdResourceInfo* resPtr = res->second;
        gdUniqueResourceID newResID = resID;
        newResID.SetResourceName( newName );

        //remove old entry
        resourceMap_.erase( res );

        //add new entry
        resPtr->SetResourceID( newResID );
        resourceMap_[newResID.GetResourceCRCID()] = resPtr;

        *outCRC = newResID.GetResourceCRCID();

        return err;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::LoadPlugInsDirectory()
    {
        boost::filesystem::recursive_directory_iterator i( plugInsPath_, boost::filesystem::symlink_option::recurse ), dirend; 
        gdString buffer;
        gdPlugInInformation* newPlugIn = new gdPlugInInformation();
        for ( ; i != dirend; ++i )
        {
            if ( !boost::filesystem::is_regular_file( i->path() ) )
                continue;

            DWORD err;
            boost::filesystem::path dllPath = i->path();
            gdTime_t lastModify = boost::filesystem::last_write_time( dllPath );
            HMODULE dll = LoadLibraryEx( dllPath.generic_wstring().c_str(), NULL, /*DONT_RESOLVE_DLL_REFERENCES | LOAD_WITH_ALTERED_SEARCH_PATH*/0 );
            if ( !dll )
            {
                err = GetLastError();
            }
            if ( newPlugIn->Initialise( dll, lastModify ) )
            {
                //Found a dll that can build resources, add it
                gdError err = AddAndValidateDllInfo( newPlugIn );
                if ( err != gdERROR_OK )
                {
                    delete newPlugIn;
                    return err;
                }
                newPlugIn = new gdPlugInInformation();
            }
            else if ( dll )
            {
                //Unload Library??
            }

        }

        delete newPlugIn;
        return gdERROR_OK;
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::AddAndValidateDllInfo( gdPlugInInformation* newPlugIn )
    {
        if ( std::find( configNames_.begin(), configNames_.end(), newPlugIn->GetConfigName() ) == configNames_.end() )    
        {
            configNames_.push_back( newPlugIn->GetConfigName() );
        }

        if ( std::find( usedResourceTypes_.begin(), usedResourceTypes_.end(), newPlugIn->GetResourceTypeName() ) == usedResourceTypes_.end() )
        {
            //Validate all parameters against the (possible) new ones
            for ( ResourceMapType::iterator i = resourceMap_.begin(), iend = resourceMap_.end(); 
                i != iend; ++i )
            {
                if ( i->second->GetResourceTypeName() && 
                     strcmp( i->second->GetResourceTypeName(), newPlugIn->GetResourceTypeName() ) == 0 )
                {
                    i->second->ApplyResourceParametersFromResourceType( *newPlugIn );
                }
            }
        }

        if ( loadedPlugIns_.find( newPlugIn->GetResourceTypeName() ) != loadedPlugIns_.end() )
            return gdERROR_DUPLICATE_PLUGIN;

        loadedPlugIns_[newPlugIn->GetResourceTypeName()] = newPlugIn;

        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::GetPlugInInfo( const gdChar* typeName, const gdPlugInInformation** outInfo ) const
    {
        gdAssert( outInfo );
        gdAssert( typeName );
        
        gdResorucePlugInMap::const_iterator f = loadedPlugIns_.find( typeName );

        if ( f == loadedPlugIns_.end() )
            return gdERROR_INVALID_PARAM;

        *outInfo = f->second;

        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdBool gdGameDatabaseObject::IsResourceCacheValid( 
        const gdResourceInfo* resInfo,
        const boost::filesystem::path& localCacheFolder,
        const boost::filesystem::path& localCacheFile,
        const boost::filesystem::path& infoCache,
        const boost::filesystem::path& infoMD5,
        const boost::filesystem::path& infoDeps,
        const gdPlugInInformation& plugInInfo )
    {
        gdTimestampInfo cacheRoot;

        if ( !boost::filesystem::exists( infoCache ) )
        {
            return false;
        }

        std::ifstream ofs( infoCache.c_str() );
        try
        {
            // save data to archive
            boost::archive::xml_iarchive oa(ofs);
            // write class instance to archive
            oa >> BOOST_SERIALIZATION_NVP( cacheRoot );
            // archive and stream closed when destructors are called
        }
        catch( boost::archive::xml_archive_exception e )
        {
            return false;
        }
        if ( !boost::filesystem::exists( localCacheFolder ) )
        {
            return false;
        }
        if ( !boost::filesystem::exists( localCacheFile ) )
        {
            return false;
        }
        if ( !boost::filesystem::exists( infoDeps ) )
        {
            return false;
        }
        if ( !boost::filesystem::exists( infoMD5 ) )
        {
            return false;
        }
        if ( cacheRoot.GetParamCRC() != resInfo->GetResourceParameterCRC() )
        {
            return false;
        }
        if ( cacheRoot.GetDllTimestamp() != plugInInfo.GetLastModificationTime() )
        {
            return false;
        }

        return cacheRoot.FileTimestampsValid();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::BuildSingleResource( const gdUniqueResourceID& resID )
    {
        ResourceMapType::iterator res = resourceMap_.find( resID.GetResourceCRCID() );
        if ( res == resourceMap_.end() )
            return gdERROR_RESOURCE_NOT_FOUND;

        gdResourceInfo* resInfo = res->second;
        return BuildSingleResourceFromResInfo(resID, resInfo);

    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::BuildSingleResourceFromResInfo( const gdUniqueResourceID &resID, gdResourceInfo* resInfo )
    {
        gdChar CRCString[64];
        gdFileHandle inputFile;
        gdFileHandle outputFile;
        gdError retCode = gdERROR_OK;

        sprintf_s( CRCString, 64, "0x%08X", resID.GetResourceCRCID() );

        if ( !resInfo->GetResourceTypeName() )
        {
            //Not got valid resource type info, skip this
            return gdERROR_NOT_RESOURCE_TYPE_INFO;
        }

        const gdPlugInInformation* plugin = NULL;
        GetPlugInInfo( resInfo->GetResourceTypeName(), &plugin );

        if ( !plugin )
        {
            //No Plugins?
            return gdERROR_PLUGIN_NOT_FOUND;
        }

        boost::filesystem::path localCachePath = cachePath_ / CRCString;
        gdString resourceOutputName = resID.GetResourceName();
        resourceOutputName += plugin->GetBuiltDataExtension();
        boost::filesystem::path outputPath = localCachePath / resourceOutputName;
        boost::filesystem::create_directories( localCachePath );
        boost::filesystem::path infoCache = localCachePath / "info.cache";
        boost::filesystem::path infoMD5 = localCachePath / "info.md5";
        boost::filesystem::path infoDeps = localCachePath / "info.deps";

        //set current directory to build path
        boost::filesystem::path workingDir(resInfo->GetInputFilePath());
        workingDir = workingDir.parent_path();
        boost::filesystem::current_path( workingDir );

        // Check the cache!
        gdBool cacheValid = IsResourceCacheValid( 
            resInfo,
            localCachePath,
            outputPath,
            infoCache,
            infoMD5,
            infoDeps, 
            *plugin );

        gdBool buildSuccess = false;
        if ( !cacheValid )
        {
            //In case the build doesn't work, delete the md5 file so cache checks fail next time
            if ( boost::filesystem::exists( infoMD5 ) )
            {
                boost::filesystem::remove( infoMD5 );
            }
            inputFile.Open( resInfo->GetInputFilePath() );
            outputFile.Open( outputPath.generic_string().c_str(), true );

            //BUILD!
            gdResourceBuilderConstructionInfo  builderInfo( 
                *plugin, 
                loadedPlugIns_,
                *resInfo->GetResourceParameterArray(),
                resID,
                &inputFile,
                &outputFile,
                resInfo->GetResourceParameterCRC() );

            gdResourceBuilderBase* builder = plugin->CreateResourceBuilder( builderInfo );
            try
            {
                builder->BuildResource();
                builder->CleanUpFromBuild();
                buildSuccess = true;
            }
            catch(gdBuildError be)
            {
                AppendErrorMessages( resID, builder->GetErrorMessage() );
                builder->CleanUpFromBuild();
                retCode = gdERROR_GENERIC;
            }
            catch(...)
            {
                AppendErrorMessages( resID, "Unknown Exception Thrown" );
                builder->CleanUpFromBuild();
                retCode = gdERROR_GENERIC;
            }

            if ( strlen( builder->GetWarningMessages() ) )
            {
                AppendWarningMessages( resID, builder->GetWarningMessages() );
            }

            inputFile.Close();
            outputFile.Close();

            if ( buildSuccess )
            {
                //write builder cache info
                std::ofstream ofs( infoCache.c_str() );
                try
                {
                    // if *this is passed its an invalid xml tag, hence dbroot is
                    // created.
                    const gdTimestampInfo& cacheRoot = builder->GetTimestampInfo();
                    // save data to archive
                    boost::archive::xml_oarchive oa(ofs);
                    // write class instance to archive
                    oa << BOOST_SERIALIZATION_NVP( cacheRoot );
                    // archive and stream closed when destructors are called
                }
                catch( ... )
                {
                    return gdERROR_INVALID_PARAM;
                }

                gdFileHandle md5;
                md5.Open( infoMD5.generic_string().c_str(), true );
                md5.Write( builder->GetMD5Digest(), CY_MD5_LEN );
                md5.Close();

                //Write out the deps
                std::ofstream dfs( infoDeps.c_str() );
                try
                {
                    // if *this is passed its an invalid xml tag, hence dbroot is
                    // created.
                    const gdDependencyArrayType& depsRoot = *builder->GetDependencyMap();
                    // save data to archive
                    boost::archive::xml_oarchive oa(dfs);
                    // write class instance to archive
                    oa << BOOST_SERIALIZATION_NVP( depsRoot );
                    // archive and stream closed when destructors are called
                }
                catch( ... )
                {
                    return gdERROR_INVALID_PARAM;
                }

                ResolveDuplicates( resInfo, builder->GetMD5Digest() );
                AppendBuiltResource( resID );

                //Attempt to build the deps
                for ( gdDependencyArrayType::const_iterator i = builder->GetDependencyMap()->begin(); i != builder->GetDependencyMap()->end(); ++i )
                {
                    retCode |= BuildSingleResourceFromResInfo( *i->second->GetResourceID(), i->second );
                }
            }

            //Done. Clean Up
            plugin->DestroyResourceBuiler( builder );
        }
        else
        {
            //cache is valid
            buildSuccess = true;

            //Loaded the deps from last time and attempt to build them
            gdDependencyArrayType depsRoot;
            std::ifstream ofs( infoDeps.c_str() );
            try
            {
                // save data to archive
                boost::archive::xml_iarchive oa(ofs);
                // write class instance to archive
                oa >> BOOST_SERIALIZATION_NVP( depsRoot );
                // archive and stream closed when destructors are called
            }
            catch( boost::archive::xml_archive_exception e )
            {
                return gdERROR_UNHANDLED_SAVE_EXPECTION;
            }

            gdFileHandle md5;
            gdByte md5Str[CY_MD5_LEN];
            md5.Open( infoMD5.generic_string().c_str(), false );
            md5.Read( md5Str, CY_MD5_LEN );
            md5.Close();

            ResolveDuplicates( resInfo, md5Str );
            AppendCacheResource( resID );

            //Attempt to build the deps
            for ( gdDependencyArrayType::const_iterator i = depsRoot.begin(); i != depsRoot.end(); ++i )
            {
                retCode |= BuildSingleResourceFromResInfo( *i->second->GetResourceID(), i->second );
            }
        }

        return retCode;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::BuildAllResources( gdBuildProgressCallback callback, void* user )
    {
        CleanDuplicatesList();
        ClearBuildMessages();

        gdError retCode = gdERROR_OK;
        try
        {
            hUint32 currentRes = 0;
            for ( ResourceMapType::iterator i = resourceMap_.begin(), iend = resourceMap_.end(); 
                i != iend; ++i )
            {
                gdUint p = (gdUint)((++currentRes/(gdFloat)resourceMap_.size())*100);
                if ( !(callback)(*i->second->GetResourceID(), p, user ) )
                {
                    return gdERROR_BUILD_CANCELLED;
                }
                retCode |= BuildSingleResource( *i->second->GetResourceID() );
            }
        }
        catch( ... )
        {
            gdAssert( false );
            return gdERROR_GENERIC;
        }

        if ( retCode == gdERROR_OK )
        {
            WriteOutputAndDuplicateRemapTable( duplicateResources_ );
        }

        return retCode;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdGameDatabaseObject::CleanData()
    {
        if ( exists( cachePath_ ) )
        {
            boost::filesystem::remove_all( cachePath_ );
        }

        if ( exists( outputPath_ ) )
        {
            boost::filesystem::remove_all( outputPath_ );
        }

        if ( !exists( cachePath_ ) )
        {
            create_directory( cachePath_ );
        }

        if ( !exists( outputPath_ ) )
        {
            create_directory( outputPath_ );
        }

        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void gdGameDatabaseObject::ClearBuildMessages()
    {
        warningMessages_ = "";
        errorMessages_ = "";
        builtMessages_ = "";
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void gdGameDatabaseObject::AppendWarningMessages( const gdUniqueResourceID& res, const hChar* warning )
    {
        warningMessages_ += "WARNING IN ";
        warningMessages_ += res.GetResourcePath();
        warningMessages_ += res.GetResourceName();
        warningMessages_ += " :: ";
        warningMessages_ += warning;
        warningMessages_ += "\n";
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void gdGameDatabaseObject::AppendErrorMessages( const gdUniqueResourceID& res, const hChar* errorMsg )
    {
        errorMessages_ += "ERROR IN ";
        errorMessages_ += res.GetResourcePath();
        errorMessages_ += res.GetResourceName();
        errorMessages_ += " :: ";
        errorMessages_ += errorMsg;
        errorMessages_ += "\n";
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void gdGameDatabaseObject::AppendBuiltResource( const gdUniqueResourceID& res )
    {
        builtMessages_ += "BUILT ";
        builtMessages_ += res.GetResourcePath();
        builtMessages_ += res.GetResourceName();
        builtMessages_ += "\n";
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void gdGameDatabaseObject::AppendCacheResource( const gdUniqueResourceID& res )
    {
        builtMessages_ += "CACHED ";
        builtMessages_ += res.GetResourcePath();
        builtMessages_ += res.GetResourceName();
        builtMessages_ += "\n";
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hChar* gdGameDatabaseObject::GetWarningMessages() const
    {
        return warningMessages_.c_str();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hChar* gdGameDatabaseObject::GetErrorMessages() const
    {
        return errorMessages_.c_str();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    const hChar* gdGameDatabaseObject::GetBuiltResourcesMessages() const
    {
        return builtMessages_.c_str();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void gdGameDatabaseObject::CleanDuplicatesList()
    {
        duplicateResources_.clear();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void gdGameDatabaseObject::ResolveDuplicates( gdResourceInfo* res, const gdByte* md5Digest )
    {
        gdChar md5Str[64];
        cyMD5DigestToString( md5Digest, md5Str );
        gdString md5(md5Str);
        DuplicateResourceMapType::iterator i = duplicateResources_.find( md5 );
        if ( i == duplicateResources_.end() )
        {
            //Add new
            ResourceListType newlist;
            newlist.push_back( res );
            duplicateResources_[md5] = newlist;
        }
        else
        {
            i->second.push_back( res );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void gdGameDatabaseObject::WriteOutputAndDuplicateRemapTable( const DuplicateResourceMapType& duplicateMap )
    {
        boost::filesystem::path rrtpath = outputPath_ / remapTableName_;
        gdFileHandle rrt;
        rrt.Open( rrtpath.generic_string().c_str(), true );

        for ( DuplicateResourceMapType::const_iterator i = duplicateMap.begin(); i != duplicateMap.end(); ++i )
        {
            if ( i->second.size() > 1 )
            {
                //Write the first, duplicates will remap to this file
                gdUint32 nullterm = 0;
                for ( ResourceListType::const_iterator li = i->second.begin(); li != i->second.end(); ++li )
                {

                    gdChar CRCString[64];
                    gdResourceInfo* resInfo = (*li);
                    const gdUniqueResourceID& resID = *resInfo->GetResourceID();

                    sprintf_s( CRCString, 64, "0x%08X", resID.GetResourceCRCID() );
                    const gdPlugInInformation* plugin;
                    GetPlugInInfo( resInfo->GetResourceTypeName(), &plugin );
                    boost::filesystem::path localCachePath = cachePath_ / CRCString;
                    gdString resourceOutputName = resID.GetResourceName();
                    resourceOutputName += plugin->GetBuiltDataExtension();
                    localCachePath = localCachePath / resourceOutputName;

                    if ( li == i->second.begin() )
                    {
                        // Copy the output file to the dest directory
                        boost::filesystem::path finalPath = outputPath_ / resID.GetResourcePath();
                        if ( !exists( finalPath ) )
                        {
                            boost::filesystem::create_directories( finalPath );
                        }
                        finalPath /= resourceOutputName;//+ plugininfo.fourCC
                        if ( boost::filesystem::exists( finalPath ) )
                        {
                            boost::filesystem::remove( finalPath );
                        }
                        boost::filesystem::copy_file( localCachePath, finalPath );

                        //Write the Path that other resources will go to
                        finalPath = resID.GetResourcePath();
                        finalPath /= resourceOutputName;

                        hUint32 len = finalPath.generic_string().size();
                        rrt.Write( &len, sizeof(len) );
                        rrt.Write( finalPath.generic_string().c_str(), len );
                    }
                    else
                    {
                        // Make a placeholder file to the dest directory
                        boost::filesystem::path finalPath = outputPath_ / resID.GetResourcePath();
                        if ( !exists( finalPath ) )
                        {
                            boost::filesystem::create_directories( finalPath );
                        }
                        finalPath /= resourceOutputName;//+ plugininfo.fourCC
                        if ( boost::filesystem::exists( finalPath ) )
                        {
                            boost::filesystem::remove( finalPath );
                        }
                        fclose( fopen( finalPath.generic_string().c_str(), "wb" ) );

                        //Write the CRC 
                        gdUint32 id = (*li)->GetResourceID()->GetResourceCRCID();
                        rrt.Write( &id, sizeof(gdUint32) );
                    }
                }
                rrt.Write( &nullterm, sizeof(gdUint32) );
            }
            else
            {
                gdChar CRCString[64];
                gdResourceInfo* resInfo = *(i->second.begin());
                const gdUniqueResourceID& resID = *resInfo->GetResourceID();

                sprintf_s( CRCString, 64, "0x%08X", resID.GetResourceCRCID() );
                const gdPlugInInformation* plugin;
                GetPlugInInfo( resInfo->GetResourceTypeName(), &plugin );
                boost::filesystem::path localCachePath = cachePath_ / CRCString;
                gdString resourceOutputName = resID.GetResourceName();
                resourceOutputName += plugin->GetBuiltDataExtension();
                localCachePath = localCachePath / resourceOutputName;

                // Copy the output file to the dest directory
                boost::filesystem::path finalPath = outputPath_ / resID.GetResourcePath();
                if ( !exists( finalPath ) )
                {
                    boost::filesystem::create_directories( finalPath );
                }
                finalPath /= resourceOutputName;//+ plugininfo.fourCC
                if ( boost::filesystem::exists( finalPath ) )
                {
                    boost::filesystem::remove( finalPath );
                }
                boost::filesystem::copy_file( localCachePath, finalPath );
            }
        }

        rrt.Close();
    }

}