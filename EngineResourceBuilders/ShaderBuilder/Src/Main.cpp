/********************************************************************
	created:	2010/06/13
	created:	13:6:2010   17:55
	filename: 	Main.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "ShaderBuilder.h"

extern "C" __declspec(dllexport) void GetPlugInInformation( GameData::gdPlugInInformation* info )
{
    using namespace GameData;

    info->SetConfigName( "PC" );
    info->SetDefaultExtensions( ".cgfx" );
    info->SetResourceTypeName( "Shader Program" );
    info->SetBuiltDataExtension( 'G', 'P', 'U' );

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_String );
        val.Set( "main" );
        param.SetName( ShaderProgramBuilder::ParameterName_Entry );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_String );
        val.Set( "" );
        param.SetName( ShaderProgramBuilder::ParameterName_Profile );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }

    {
        gdEnumValue values[] =
        {
            { "Vertex",      Heart::ShaderType_VERTEXPROG },
            { "Fragment",    Heart::ShaderType_FRAGMENTPROG },
        };
        gdParameterDefinition param;
        gdParameterValue val( gdEnumValueArray( values, values + (sizeof(values)/sizeof(values[0])) ) );
        val.SetEnumByValue( Heart::ShaderType_VERTEXPROG );
        param.SetName( ShaderProgramBuilder::ParameterName_Type );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }
}

//define the entry points for the resource plug in loader
extern "C" __declspec(dllexport) GameData::gdResourceBuilderBase* CreateResourceBuilder( const GameData::gdResourceBuilderConstructionInfo& builderInfo )
{
	return new ShaderProgramBuilder( builderInfo );
}

extern "C" __declspec(dllexport) void DestroyResourceBuilder( GameData::gdResourceBuilderBase* pBuilder )
{
	delete pBuilder;
}