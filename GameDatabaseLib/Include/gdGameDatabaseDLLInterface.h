/********************************************************************

	filename: 	gdGameDatabaseDLLInterface.h	
	
	Copyright (c) 29:10:2011 James Moran
	
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

#ifndef GDGAMEDATABASEDLLINTERFACE_H__
#define GDGAMEDATABASEDLLINTERFACE_H__

namespace GameData
{
    class gdPlugInInformation;
    class gdResourceBuilderBase;
    struct gdResourceBuilderConstructionInfo;
    typedef std::map< gdString, gdPlugInInformation* >  gdResorucePlugInMap;
}

//////////////////////////////////////////////////////////////////////////
// Prototypes/structures for DLL functions and interfaces ////////////////
//////////////////////////////////////////////////////////////////////////
gdDEFINE_DLL_FUNCTION_PROTOTYPE( void, GetPlugInInformation ) (GameData::gdPlugInInformation*);
gdDEFINE_DLL_FUNCTION_PROTOTYPE( GameData::gdResourceBuilderBase*, CreateResourceBuilder ) ( const GameData::gdResourceBuilderConstructionInfo& );
gdDEFINE_DLL_FUNCTION_PROTOTYPE( void, DestroyResourceBuilder ) ( GameData::gdResourceBuilderBase* );

namespace GameData
{
    typedef std::vector< gdParameterDefinition > gdParameterDefinitionArray;

    class gdDLL_EXPORT gdPlugInInformation
    {
    public:
        gdPlugInInformation();
        ~gdPlugInInformation();

        gdBool                  Initialise( HMODULE dllHandle, gdTime_t lastModTime );
        gdBool                  GetIsValid() const;
        void                    SetConfigName( const gdChar* configName );
        void                    SetResourceTypeName( const gdChar* typeName );
        void                    SetDefaultExtensions( const gdChar* exts );
        void                    SetBuiltDataExtension( gdChar a, gdChar b, gdChar c );
        void                    AppendDefaultParameter( const gdParameterDefinition& paramDef );
        const gdChar*           GetConfigName() const;
        const gdChar*           GetResourceTypeName() const;
        const gdChar*           GetDefaultExtensions() const;
        const gdChar*           GetBuiltDataExtension() const { return builtDataExt_; }
        void                    GetDefaultParameterArray( gdParameterDefinitionArray& out ) const;
        gdTime_t                GetLastModificationTime() const;
        gdResourceBuilderBase*  CreateResourceBuilder( const gdResourceBuilderConstructionInfo& info ) const;
        void                    DestroyResourceBuiler( gdResourceBuilderBase* builder ) const;

    private:
// Hide stupid ms warning C4251
// (XXX has no DLL interface warning. Things that are private 
// in the class are not inline to avoid exposing them to outside libs.
// All work with the private members is hidden in the dll implementation.)
#pragma warning( push )
#pragma warning( disable: 4251 )
        gdPREVENT_COPY( gdPlugInInformation );

        // resource configs supported. Seperated by ; (read platform e.g. PC;MAC;)
        gdString configName_;
        // resource type name
        gdString resourceTypeName_;
        // default extensions used. Seperated by ; with the dot e.g. ".xml;.tga;.jpg"
        gdString extensions_;
        // default parameter definitions
        gdParameterDefinitionArray parameterDefinitions_;
        //Last modified date of dll
        gdTime_t lastModTime_;
        //.extension to identify data build by this plug in (e.g. .EXT)
        gdChar builtDataExt_[5];
        //DLL information
        // handle to the loaded dll 
        HMODULE dllHandle_;
        //All the process addresses needed by the database library
        gdDLLPT_GetPlugInInformation  infoProc_; 
        gdDLLPT_CreateResourceBuilder createProc_;
        gdDLLPT_DestroyResourceBuilder destroyProc_;
//Stop Hiding warning C4251
#pragma warning ( pop )
    };
}


#endif // GDGAMEDATABASEDLLINTERFACE_H__