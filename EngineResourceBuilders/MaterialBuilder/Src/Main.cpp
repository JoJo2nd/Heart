/********************************************************************
	created:	2010/06/13
	created:	13:6:2010   17:55
	filename: 	Main.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "MaterialBuilder.h"

extern "C" __declspec(dllexport) void GetPlugInInformation( GameData::gdPlugInInformation* info )
{
    info->SetConfigName( "PC" );
    info->SetDefaultExtensions( ".cgfx" );
    info->SetResourceTypeName( "Cg Effect" );
    info->SetBuiltDataExtension( 'C', 'F', 'X' );
}

//define the entry points for the resource plug in loader
extern "C" __declspec(dllexport) GameData::gdResourceBuilderBase* CreateResourceBuilder( const GameData::gdResourceBuilderConstructionInfo& builderInfo )
{
	return new MaterialEffectBuilder( builderInfo );
}

extern "C" __declspec(dllexport) void DestroyResourceBuilder( GameData::gdResourceBuilderBase* pBuilder )
{
	delete pBuilder;
}