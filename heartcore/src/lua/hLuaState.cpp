/********************************************************************
	created:	2009/01/24

	filename: 	hlLuaState.cpp

	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "Common.h"

extern "C"{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include "hlLuaState.h"
#include "hCore.h"

/*/////////////////////////////////////////////////////////////////////////

function: initLua - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
void hlLuaState::initLua()
{
	pLuaState_ = lua_open();

	hcAssert( pLuaState_ != NULL );

	if ( pLuaState_ )
	{
		luaL_openlibs( pLuaState_ );
		// DONT WAIT TO COLLECT GARBAGE [2/1/2009 James]
		lua_gc( pLuaState_, LUA_GCSETPAUSE, 100 );
	}
}

/*/////////////////////////////////////////////////////////////////////////

function: destroyLua - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
void hlLuaState::destroyLua()
{
	if ( pLuaState_ )
	{
		lua_close( pLuaState_ );
	}
}

/*/////////////////////////////////////////////////////////////////////////

function: hlLuaState - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaState::hlLuaState() : 
	pLuaState_( NULL )
{

}

/*/////////////////////////////////////////////////////////////////////////

function: ~hlLuaState - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaState::~hlLuaState()
{

}
