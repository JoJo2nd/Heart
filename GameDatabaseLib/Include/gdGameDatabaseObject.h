/********************************************************************

	filename: 	gdGameDatabaseObject.h	
	
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

#ifndef GDGAMEDATABASEOBJECT_H__
#define GDGAMEDATABASEOBJECT_H__

namespace GameData
{
    typedef gdBool (*gdBuildProgressCallback)( const gdUniqueResourceID&, gdUint progress, void* );
    typedef std::vector< gdUniqueResourceID > gdResourceIDArray;
    typedef std::vector< gdString > gdStringArray;

    class gdDLL_EXPORT gdGameDatabaseObject
    {
    public:
        gdGameDatabaseObject();
        ~gdGameDatabaseObject();
        gdError Create( const gdWchar* destPath );
        gdError Load( const gdWchar* path );
        gdError Save() const;
        const gdChar* GetOutputPath();
        gdError SetOutputPath( const gdChar* path );
        gdError AddResource( 
            const gdChar* inputFile, 
            const gdChar* path, 
            const gdChar* name, 
            gdUniqueResourceID* resourceID );
        gdError RemoveResource( const gdUniqueResourceID& resourceID );
        gdError IsMoveResourceValid( const gdUniqueResourceID& resID, const gdChar* newPath );
        gdError MoveResource( const gdUniqueResourceID& resID, const gdChar* newPath, gdUint32* outCRC  );     
        gdError IsRenameResourceValid( const gdUniqueResourceID& resID, const gdChar* newName );
        gdError RenameResource( const gdUniqueResourceID& resID, const gdChar* newName, gdUint32* outCRC );
        gdError GetResourceIDByCRC( gdUint32 crc, gdUniqueResourceID* outResID );
        gdError GetResourceByCRC( gdUint32 crc, gdResourceInfo** outResID );
        gdUint  GetResourceIDArray( gdResourceIDArray* outArray );
        gdUint  GetAvailableResoruceTypes( gdStringArray* outArray ) const;
        const gdChar* GetResourceTypeNameByIndex( gdUint idx ) const;
        gdError IsResourceTypeNameValid( const gdChar* typeName ) const;
        gdError GetPlugInInfo( const gdChar* typeName, const gdPlugInInformation** outInfo ) const;
        gdError BuildSingleResource( const gdUniqueResourceID& resID );
        gdError BuildSingleResourceFromResInfo( const gdUniqueResourceID &resID, gdResourceInfo* resInfo );
        gdError BuildAllResources( gdBuildProgressCallback callback, void* user );
        gdError CleanData();
        void    ClearBuildMessages();
        const hChar* GetWarningMessages() const;
        const hChar* GetErrorMessages() const;
        const hChar* GetBuiltResourcesMessages() const;

        gdDEFINE_SAVE_VERSION( 2 )

    private:

        friend class boost::serialization::access;

        typedef std::list< gdResourceInfo* >                ResourceListType;
        typedef std::map< gdUint32, gdResourceInfo* >       ResourceMapType;
        typedef std::map< gdString, ResourceListType >      DuplicateResourceMapType;
        typedef std::list< gdString >                       StringList;

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version )
        {
            switch ( version )
            {
            case 2:
                arc & BOOST_SERIALIZATION_NVP(dataOutputPath_);
            case 1:
                arc & BOOST_SERIALIZATION_NVP(usedResourceTypes_);
            case 0:
                arc & BOOST_SERIALIZATION_NVP(resourceMap_);
                arc & BOOST_SERIALIZATION_NVP(databaseFilepath_);
                arc & BOOST_SERIALIZATION_NVP(dataSourcePath_);
            default:
                break;
            }
        }
        gdError LoadPlugInsDirectory();
        gdError ValidateAfterLoad();
        void    CleanUpMemoryResources();
        gdError AddAndValidateDllInfo( gdPlugInInformation* newPlugIn );
        gdBool  IsResourceCacheValid( 
            const gdResourceInfo* resInfo,
            const boost::filesystem::path& localCacheFolder,
            const boost::filesystem::path& localCacheFile,
            const boost::filesystem::path& infoCache,
            const boost::filesystem::path& infoMD5,
            const boost::filesystem::path& infoDeps,
            const gdPlugInInformation& plugInInfo );
        void    AppendWarningMessages( const gdUniqueResourceID& res, const hChar* warning );
        void    AppendErrorMessages( const gdUniqueResourceID& res, const hChar* warning );
        void    AppendBuiltResource( const gdUniqueResourceID& res );
        void    AppendCacheResource( const gdUniqueResourceID& res );
        void    CleanDuplicatesList();
        void    ResolveDuplicates( gdResourceInfo* res, const gdByte* md5Digest );
        void    WriteOutputAndDuplicateRemapTable( const DuplicateResourceMapType& duplicateMap );

//Hide stupid ms warning C4251
#pragma warning( push )
#pragma warning( disable: 4251 )

        gdString                 databaseFilepath_;
        gdString                 dataSourcePath_;
        gdString                 dataOutputPath_;
        ResourceMapType          resourceMap_;
        StringList               usedResourceTypes_;
        DuplicateResourceMapType duplicateResources_;

        //Non-serialized members
        gdResorucePlugInMap     loadedPlugIns_;  
        StringList              configNames_;
        boost::filesystem::path databaseFolderPath_;
        boost::filesystem::path plugInsPath_;
        boost::filesystem::path cachePath_;
        boost::filesystem::path outputPath_;
        boost::filesystem::path databasePath_;
        gdString                errorMessages_;
        gdString                warningMessages_;
        gdString                builtMessages_;

//Stop Hiding warning C4251
#pragma warning ( pop )
    };
}

gdDECLARE_SAVE_VERSION( GameData::gdGameDatabaseObject )

#endif // GDGAMEDATABASEOBJECT_H__