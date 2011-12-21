/********************************************************************
	created:	2010/06/13
	created:	13:6:2010   17:55
	filename: 	Main.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "TextureBuilder.h"
#include "squish.h"

extern "C" __declspec(dllexport) void GetPlugInInformation( GameData::gdPlugInInformation* info )
{
    using namespace GameData;

    info->SetConfigName( "PC" );
    info->SetDefaultExtensions( ".png" );
    info->SetResourceTypeName( "texture" );
    info->SetBuiltDataExtension( 'T', 'E', 'X' );

    //Default parameters
    {
        gdEnumValue values[] =
        {
            { "DXT5 Fast",      squish::kDxt5 | squish::kColourRangeFit },
            { "DXT5 Slow",      squish::kDxt5 | squish::kColourClusterFit },
            { "DXT5 Very Slow", squish::kDxt5 | squish::kColourIterativeClusterFit},
            { "DXT3 Fast",      squish::kDxt3 | squish::kColourRangeFit },
            { "DXT3 Slow",      squish::kDxt3 | squish::kColourClusterFit },
            { "DXT3 Very Slow", squish::kDxt3 | squish::kColourIterativeClusterFit},
            { "DXT1 Fast",      squish::kDxt1 | squish::kColourRangeFit },
            { "DXT1 Slow",      squish::kDxt1 | squish::kColourClusterFit },
            { "DXT1 Very Slow", squish::kDxt1 | squish::kColourIterativeClusterFit},
            { "None", 0 },
        };
        gdParameterDefinition param;
        gdParameterValue val( gdEnumValueArray( values, values + (sizeof(values)/sizeof(values[0])) ) );
        val.SetEnumByValue( squish::kDxt3 | squish::kColourRangeFit );
        param.SetName( TextureBuilder::ParameterName_Compression );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_Bool );
        val.Set( true );
        param.SetName( TextureBuilder::ParameterName_GenerateMips );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }
}

//define the entry points for the resource plug in loader
extern "C" __declspec(dllexport) GameData::gdResourceBuilderBase* CreateResourceBuilder( const GameData::gdResourceBuilderConstructionInfo& builderInfo )
{
	return new TextureBuilder( builderInfo );
}

extern "C" __declspec(dllexport) void DestroyResourceBuilder( GameData::gdResourceBuilderBase* builder )
{
	delete builder;
}