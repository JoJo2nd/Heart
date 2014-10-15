/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "WorldObjectBuilder.h"


extern "C" __declspec(dllexport) void __cdecl GetPlugInInformation( GameData::gdPlugInInformation* info )
{
    using namespace GameData;

    info->SetConfigName( "PC" );
    info->SetDefaultExtensions( ".wod" );
    info->SetResourceTypeName( "World Object Definitions" );
    info->SetBuiltDataExtension( 'W', 'O', 'D' );

    //Default parameters

}

//define the entry points for the resource plug in loader
extern "C" __declspec(dllexport) GameData::gdResourceBuilderBase* __cdecl CreateResourceBuilder( const GameData::gdResourceBuilderConstructionInfo& builderInfo )
{
    return new WorldObjectBuilder( builderInfo );
}

extern "C" __declspec(dllexport) void __cdecl DestroyResourceBuilder( GameData::gdResourceBuilderBase* builder )
{
    delete builder;
}
