/********************************************************************
	created:	2009/01/24

	filename: 	hlLuaString.cpp

	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "Heart.h"
#include "hlLuaString.h"
#include "hlLuaState.h"

extern "C"{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

/*/////////////////////////////////////////////////////////////////////////

function: operator= - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaString& hlLuaString::operator=( const hlLuaString& p )
{
	START_STACK_CHECK( pLuaState_ );

	// decrease our ref count [1/26/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, RefIdx_ );
	hcAssert( lua_isnumber( pLuaState_ , -1 ) );
	hInt32 refc = lua_tointeger( pLuaState_, -1 );
	lua_pop( pLuaState_, -1 );
	--refc;

	// if we have become zero then release this string [1/26/2009 James]
	if ( refc == 0 )
	{
		lua_unref( pLuaState_, RefIdx_ );
		lua_unref( pLuaState_, ValIdx_ );
	}
	else
	{
		// else push the new ref count [1/26/2009 James]
		lua_pushinteger( pLuaState_, refc );
		lua_rawseti( pLuaState_, LUA_REGISTRYINDEX, RefIdx_ );
	}

	// increase the ref count of p [1/26/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, p.RefIdx_ );
	hcAssert( lua_isnumber( pLuaState_, -1 ) );
	refc = lua_tointeger( pLuaState_, -1 );
	lua_pop( pLuaState_, -1 );
	++refc;
	
	// push it back and re=store it [1/26/2009 James]
	lua_pushinteger( pLuaState_, refc );
	lua_rawseti( pLuaState_, LUA_REGISTRYINDEX, p.RefIdx_ );

	// store the values to registry from p [1/26/2009 James]
	RefIdx_ = p.RefIdx_;
	ValIdx_ = p.ValIdx_;

	// get the new string [1/26/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, ValIdx_ );
	pString_ = lua_tostring( pLuaState_, -1 );
	lua_pop( pLuaState_, -1 );

	END_STACK_CHECK( pLuaState_ );
	
	return *this;
}

/*/////////////////////////////////////////////////////////////////////////

function: hlLuaString - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaString::hlLuaString( const hInt32 stackidx )
{
	pLuaState_ = pLuaState_;

	//START_STACK_CHECK( pLuaState_ );

	lua_pushinteger( pLuaState_, 1 );
	RefIdx_ = lua_ref( pLuaState_, -1 );

	hcAssert( lua_isstring( pLuaState_, stackidx ) );
	ValIdx_ = lua_ref( pLuaState_, stackidx );

	// get the new string [1/26/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, ValIdx_ );
	pString_ = lua_tostring( pLuaState_, -1 );
	
	lua_pop( pLuaState_, -1 );// remove the pString [1/26/2009 James]
	//lua_remove( pLuaState_, stackidx );// remove the string that we took ref of [1/26/2009 James]

	//END_STACK_CHECK( pLuaState_ );
}

/*/////////////////////////////////////////////////////////////////////////

function: hlLuaString - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaString::hlLuaString( const hlLuaString& p )
{
	// don't decrease our ref count because we don't have one yet [1/26/2009 James]
	pLuaState_ = pLuaState_;

	START_STACK_CHECK( pLuaState_ );

	// increase the ref count of p [1/26/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, p.RefIdx_ );
	hcAssert( lua_isnumber( pLuaState_, -1 ) );
	hInt32 refc = lua_tointeger( pLuaState_, -1 );
	lua_pop( pLuaState_, -1 );
	++refc;

	// push it back and re=store it [1/26/2009 James]
	lua_pushinteger( pLuaState_, refc );
	lua_rawseti( pLuaState_, LUA_REGISTRYINDEX, p.RefIdx_ );

	// store the values to registry from p [1/26/2009 James]
	RefIdx_ = p.RefIdx_;
	ValIdx_ = p.ValIdx_;

	// get the new string [1/26/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, ValIdx_ );
	pString_ = lua_tostring( pLuaState_, -1 );
	lua_pop( pLuaState_, -1 );

	END_STACK_CHECK( pLuaState_ );
}

/*/////////////////////////////////////////////////////////////////////////

function: ~hlLuaString - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaString::~hlLuaString()
{
	START_STACK_CHECK( pLuaState_ );

	// decrease our ref count [1/26/2009 James]  
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, RefIdx_ );
	hcAssert( lua_isnumber( pLuaState_ , -1 ) );
	hInt32 refc = lua_tointeger( pLuaState_, -1 );
	lua_pop( pLuaState_, -1 );
	--refc;

	// if we have become zero then release this string for Lua to collect [1/26/2009 James]
	if ( refc == 0 )
	{
		lua_unref( pLuaState_, RefIdx_ );
		lua_unref( pLuaState_, ValIdx_ );
	}
	else
	{
		// else push the new ref count [1/26/2009 James]
		lua_pushinteger( pLuaState_, refc );
		lua_rawseti( pLuaState_, LUA_REGISTRYINDEX, RefIdx_ );
	}

	END_STACK_CHECK( pLuaState_ );
}
