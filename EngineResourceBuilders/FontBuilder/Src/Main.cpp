/********************************************************************
	created:	2010/09/05
	created:	5:9:2010   9:38
	filename: 	Main.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "FontBuilder.h"

extern "C" __declspec(dllexport) void __cdecl GetPlugInInformation( GameData::gdPlugInInformation* info )
{
    using namespace GameData;

    info->SetConfigName( "PC" );
    info->SetDefaultExtensions( ".ttf" );
    info->SetResourceTypeName( "Font Resource" );
    info->SetBuiltDataExtension( 'F', 'N', 'T' );

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_Int );
        val.Set( 10 );
        param.SetName( FontBuilder::FONT_SIZE_PARAM );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_String );
        val.Set( "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM[](){}=+-?!%^*\\/.,:;@'~#_" );
        param.SetName( FontBuilder::FONT_CHAR_ARRAY );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_Filename );
        val.SetFile( "" );
        param.SetName( FontBuilder::FONT_MATERIAL_PARAM );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }
}

//define the entry points for the resource plug in loader
extern "C" __declspec(dllexport) GameData::gdResourceBuilderBase* __cdecl CreateResourceBuilder( const GameData::gdResourceBuilderConstructionInfo& builderInfo )
{
    return new FontBuilder( builderInfo );
}

extern "C" __declspec(dllexport) void __cdecl DestroyResourceBuilder( GameData::gdResourceBuilderBase* pBuilder )
{
    delete pBuilder;
}