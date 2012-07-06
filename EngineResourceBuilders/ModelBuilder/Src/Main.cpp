/********************************************************************

	filename: 	Main.cpp	
	
	Copyright (c) 19:5:2012 James Moran
	
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


#include "ModelBuilder.h"

extern "C" __declspec(dllexport) void __cdecl GetPlugInInformation( GameData::gdPlugInInformation* info )
{
    using namespace GameData;

    info->SetConfigName( "PC" );
    info->SetDefaultExtensions( ".dae" );
    info->SetResourceTypeName( "3D Model" );
    info->SetBuiltDataExtension( 'M', 'D', 'F' );

    //Default parameters
    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_Bool );
        val.Set( false );
        param.SetName( ModelBuilder::ParameterName_GenerateOctTree );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_Filename );
        val.SetFile( "" );
        param.SetName( ModelBuilder::ParameterName_PropsXML );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_Bool );
        val.Set( true );
        param.SetName( ModelBuilder::ParameterName_ExportLights );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_Bool );
        val.Set( true );
        param.SetName( ModelBuilder::ParameterName_ExportCameras );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }

    {
        gdParameterDefinition param;
        gdParameterValue val( gdParameterValue::Type_Bool );
        val.Set( false );
        param.SetName( ModelBuilder::ParameterName_SwapYZ );
        param.SetDefaultValue( val );

        info->AppendDefaultParameter( param );
    }
}

//define the entry points for the resource plug in loader
extern "C" __declspec(dllexport) GameData::gdResourceBuilderBase* __cdecl CreateResourceBuilder( const GameData::gdResourceBuilderConstructionInfo& builderInfo )
{
	return new ModelBuilder( builderInfo );
}

extern "C" __declspec(dllexport) void __cdecl DestroyResourceBuilder( GameData::gdResourceBuilderBase* builder )
{
	delete builder;
}