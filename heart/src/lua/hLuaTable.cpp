/********************************************************************
	created:	2009/01/26

	filename: 	hlLuaTable.cpp

	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "Heart.h"
#include "hlLuaState.h"
#include "hlLuaTable.h"

extern "C"{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

/*/////////////////////////////////////////////////////////////////////////

function: hlLuaTable - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaTable::hlLuaTable( const hInt32 stackIdx )
{
	//START_STACK_CHECK( pLuaState_ );

	pLuaState_ = pLuaState_;

	lua_pushinteger( pLuaState_, 1 );
	RefIdx_ = lua_ref( pLuaState_, -1 );

	hcAssert( lua_istable( pLuaState_, stackIdx ) == hTrue );
	ValIdx_ = lua_ref( pLuaState_, stackIdx );

	//lua_remove( pLuaState_, stackIdx );// remove the table that we took ref of [1/26/2009 James]

	//END_STACK_CHECK( pLuaState_ );
}

/*/////////////////////////////////////////////////////////////////////////

function: hlLuaTable - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaTable::hlLuaTable( const hlLuaTable& c )
{
	pLuaState_ = pLuaState_;

	START_STACK_CHECK( pLuaState_ );

	// increase the ref count of c [1/26/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, c.RefIdx_ );
	hcAssert( lua_isnumber( pLuaState_, -1 ) );
	hInt32 refc = lua_tointeger( pLuaState_, -1 );

	lua_pop( pLuaState_, -1 );

	++refc;

	// push it back and re=store it [1/26/2009 James]
	lua_pushinteger( pLuaState_, refc );
	lua_rawseti( pLuaState_, LUA_REGISTRYINDEX, c.RefIdx_ );

	// store the values to registry from c [1/26/2009 James]
	RefIdx_ = c.RefIdx_;
	ValIdx_ = c.ValIdx_;

	END_STACK_CHECK( pLuaState_ );
}

/*/////////////////////////////////////////////////////////////////////////

function: ~hlLuaTable - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaTable::~hlLuaTable()
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

/*/////////////////////////////////////////////////////////////////////////

function: operator= - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaTable& hlLuaTable::operator=( const hlLuaTable& p )
{
	START_STACK_CHECK( pLuaState_ );

	// decrease our ref count [1/26/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, RefIdx_ );
	hcAssert( lua_isnumber( pLuaState_ , -1 ) );
	hInt32 refc = lua_tointeger( pLuaState_, -1 );
	lua_pop( pLuaState_, -1 );// remove the pushed int of the stack [1/26/2009 James]
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
	lua_pop( pLuaState_, -1 );// remove the pushed int of the stack [1/26/2009 James]
	++refc;

	// push it back and re=store it [1/26/2009 James]
	lua_pushinteger( pLuaState_, refc );
	lua_rawseti( pLuaState_, LUA_REGISTRYINDEX, p.RefIdx_ );

	// store the values to registry from p [1/26/2009 James]
	RefIdx_ = p.RefIdx_;
	ValIdx_ = p.ValIdx_;

	END_STACK_CHECK( pLuaState_ );

	return *this;
}

/*/////////////////////////////////////////////////////////////////////////

function: getVarAsInt - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hInt32 hlLuaTable::getVarAsInt( const hChar* var )
{
	START_STACK_CHECK( pLuaState_ );
	// push our table on to the stack [1/31/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, ValIdx_ );

	// get the field [1/31/2009 James]
	lua_pushstring( pLuaState_, var );
	lua_rawget( pLuaState_, -2 );

	// check its an int [1/31/2009 James]
	hcAssert( lua_isnumber( pLuaState_, -1 ) );

	hInt32 r = lua_tointeger( pLuaState_, -1 );

	// pop table and int [1/31/2009 James]
	lua_pop( pLuaState_, 2 );

	END_STACK_CHECK( pLuaState_ );
	return r;
}

/*/////////////////////////////////////////////////////////////////////////

function: getVarAsString - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaString hlLuaTable::getVarAsString( const hChar* var )
{
	START_STACK_CHECK( pLuaState_ );

	// push our table on to the stack [1/31/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, ValIdx_ );

	hcAssert( lua_istable( pLuaState_, -1 ) == hTrue );

	// get the field [1/31/2009 James]
	lua_pushstring( pLuaState_, var );
	lua_rawget( pLuaState_, -2 );

	// check its an int [1/31/2009 James]
	hcAssert( lua_isstring( pLuaState_, -1 ) );

	// pop table[1/31/2009 James]
	lua_remove( pLuaState_, -2 );

	hlLuaString	r( -1 );

	END_STACK_CHECK( pLuaState_ );
	return r;
}

/*/////////////////////////////////////////////////////////////////////////

function: getVarAsFloat - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hFloat hlLuaTable::getVarAsFloat( const hChar* var )
{
	START_STACK_CHECK( pLuaState_ );
	// push our table on to the stack [1/31/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, ValIdx_ );

	// get the field [1/31/2009 James]
	lua_pushstring( pLuaState_, var );
	lua_rawget( pLuaState_, -2 );

	// check its an int [1/31/2009 James]
	hcAssert( lua_isnumber( pLuaState_, -1 ) );

	hFloat r = static_cast< hFloat >( lua_tonumber( pLuaState_, -1 ) );

	// pop table and int [1/31/2009 James]
	lua_pop( pLuaState_, 2 );

	END_STACK_CHECK( pLuaState_ );
	return r;
}

/*/////////////////////////////////////////////////////////////////////////

function: isVarNumber - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hBool hlLuaTable::isVarNumber( const hChar* var )
{
	START_STACK_CHECK( pLuaState_ );
	// push our table on to the stack [1/31/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, ValIdx_ );

	// get the field [1/31/2009 James]
	lua_pushstring( pLuaState_, var );
	lua_rawget( pLuaState_, -2 );

	// check its an int [1/31/2009 James]
	hBool r = lua_isnumber( pLuaState_, -1 ) > 0;

	// pop table and int [1/31/2009 James]
	lua_pop( pLuaState_, 2 );

	END_STACK_CHECK( pLuaState_ );
	return r;
}

/*/////////////////////////////////////////////////////////////////////////

function: isVarString - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hBool hlLuaTable::isVarString( const hChar* var )
{
	START_STACK_CHECK( pLuaState_ );
	// push our table on to the stack [1/31/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, ValIdx_ );

	// get the field [1/31/2009 James]
	lua_pushstring( pLuaState_, var );
	lua_rawget( pLuaState_, -2 );

	// check its an int [1/31/2009 James]
	hBool r = lua_isstring( pLuaState_, -1 ) >= 0;

	// pop table and string [1/31/2009 James]
	lua_pop( pLuaState_, 2 );

	END_STACK_CHECK( pLuaState_ );
	return r;
}

/*/////////////////////////////////////////////////////////////////////////

function: isVarTable - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hBool hlLuaTable::isVarTable( const hChar* var )
{
	START_STACK_CHECK( pLuaState_ );
	// push our table on to the stack [1/31/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, ValIdx_ );

	// get the field [1/31/2009 James]
	lua_pushstring( pLuaState_, var );
	lua_rawget( pLuaState_, -2 );

	// check its an int [1/31/2009 James]
	hBool r = lua_istable( pLuaState_, -1 );

	// pop table and table [1/31/2009 James]
	lua_pop( pLuaState_, 2 );

	END_STACK_CHECK( pLuaState_ );
	return r;
}

/*/////////////////////////////////////////////////////////////////////////

function: isVarFunction - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hBool hlLuaTable::isVarFunction( const hChar* var )
{
	START_STACK_CHECK( pLuaState_ );
	// push our table on to the stack [1/31/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, ValIdx_ );

	// get the field [1/31/2009 James]
	lua_pushstring( pLuaState_, var );
	lua_rawget( pLuaState_, -2 );

	// check its an int [1/31/2009 James]
	hBool r = lua_isfunction( pLuaState_, -1 );

	// pop table and function [1/31/2009 James]
	lua_pop( pLuaState_, 2 );

	END_STACK_CHECK( pLuaState_ );
	return r;
}

/*/////////////////////////////////////////////////////////////////////////

function: getVarAsTable - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaTable hlLuaTable::getVarAsTable( const hChar* var )
{
	START_STACK_CHECK( pLuaState_ );

	// push our table on to the stack [1/31/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, ValIdx_ );

	// get the field [1/31/2009 James]
	lua_pushstring( pLuaState_, var );
	lua_rawget( pLuaState_, -2 );

	// check its an int [1/31/2009 James]
	hcAssert( lua_istable( pLuaState_, -1 ) );

	// pop table[1/31/2009 James]
	lua_remove( pLuaState_, -2 );

	hlLuaTable	r( -1 );

	END_STACK_CHECK( pLuaState_ );
	return r;
}

/*/////////////////////////////////////////////////////////////////////////

function: getVarAsFunction - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hBool hlLuaTable::getVarAsFunction( const hChar* var ) const
{
	START_STACK_CHECK( pLuaState_ );
	// push our table on to the stack [1/31/2009 James]
	lua_rawgeti( pLuaState_, LUA_REGISTRYINDEX, ValIdx_ );

	// get the field [1/31/2009 James]
	lua_pushstring( pLuaState_, var );
	lua_rawget( pLuaState_, -2 );

	// check its an int [1/31/2009 James]
	hBool r = lua_isfunction( pLuaState_, -1 );

	if ( r == hTrue )
	{
		// cant stack check cos this leaves a function on the stack [1/31/2009 James]
		// pop table, leave function for caller on stack [1/31/2009 James]
		lua_remove( pLuaState_, -2 );
	}
	else
	{
		// pop table and function [1/31/2009 James]
		lua_pop( pLuaState_, 2 );

		END_STACK_CHECK( pLuaState_ );
	}

	return r;
}