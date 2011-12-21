/********************************************************************

	filename: 	DLLInterface.cpp	
	
	Copyright (c) 30:10:2011 James Moran
	
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

#include "TestResourceBuilder.h"
#include "GameDatabaseLib.h"

extern "C" __declspec(dllexport) void GetPlugInInformation( GameData::gdPlugInInformation* info )
{
    using namespace GameData;

    info->SetConfigName( "PC" );
    info->SetDefaultExtensions( ".txt;.placeholder" );
    info->SetResourceTypeName( "placeholder" );

    //Default parameters
    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_String );
        val.Set( "DefaultValue" );
        param.SetName( "ParamString" );
        param.SetDefaultValue( val );        

        info->AppendDefaultParameter( param );
    }

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_Int );
        val.Set( 0 );
        param.SetName( "ParamInt" );
        param.SetDefaultValue( val );        

        info->AppendDefaultParameter( param );
    }

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_Float );
        val.Set( 1.0f );
        param.SetName( "ParamFloat" );
        param.SetDefaultValue( val );        

        info->AppendDefaultParameter( param );
    }

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_Bool );
        val.Set( false );
        param.SetName( "ParamBool" );
        param.SetDefaultValue( val );        

        info->AppendDefaultParameter( param );
    }

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_Filename );
        param.SetName( "ParamFile" );    
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }


    {
        gdEnumValue values[] =
        {
            { "DXT5", 5 },    
            { "DXT4", 4 },
            { "DXT3", 3 },
            { "DXT2", 2 },
            { "DXT1", 1 },
            { "None", 0 },
        };
        gdParameterDefinition param;
        gdParameterValue val( gdEnumValueArray( values, values + (sizeof(values)/sizeof(values[0])) ) );
        val.SetEnumByValue( 0 );
        param.SetName( "ParamEnum" );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }
}

extern "C" __declspec(dllexport) GameData::gdResourceBuilderBase* CreateResourceBuilder( const GameData::gdResourceBuilderConstructionInfo& resInfo )
{
    return new TestResourceBuilder( resInfo );
}

extern "C" __declspec(dllexport) void DestroyResourceBuilder( GameData::gdResourceBuilderBase* builder )
{
    delete builder;
}