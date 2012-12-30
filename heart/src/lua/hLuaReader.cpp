/********************************************************************
	created:	2009/01/26

	filename: 	hlLuaReader.cpp

	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "Heart.h"
#include "hlLuaReader.h"
#include "hlLuaState.h"

extern "C"{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

hInt32 hlLuaReader::nLastReturn_ = 0;

/*/////////////////////////////////////////////////////////////////////////

function: readScript - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
void hlLuaReader::readScript( void* pScript, hUint32 size, const hChar* name /*= "Default Script" */ )
{
	if ( pScript )
	{
		lua_State* L = pLua_->pLuaState_;
		START_STACK_CHECK( L );

		luaL_loadbuffer( L, (hChar*)pScript, size, name );
		if ( lua_pcall( L, 0, 0, 0 ) )
		{
			hcPrintf( "LUA ERROR: %s\n", lua_tostring( L, -1 ) );
			lua_pop( L, 1 );  /* pop error message from the stack */
		}
		END_STACK_CHECK( L );
	}
	else
	{
		hcPrintf( "LUA ERROR: No Script to run\n" );
	}
}

/*/////////////////////////////////////////////////////////////////////////

function: getVarAsString - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaString hlLuaReader::getVarAsString( const hChar* var )
{
	lua_State* L = pLua_->pLuaState_;
	START_STACK_CHECK( L );

	lua_getglobal( L, var );

	hcAssert( lua_isstring( L, -1 ) );
	if ( !lua_isstring( L, -1 ) )
	{
		// variable didn't exist so push a default variable [1/26/2009 James]
		lua_pushstring(  L, "No String" );
	}
	
	hlLuaString r( -1 );

	lua_pop( L, -1 );// pop the global var [1/26/2009 James]

	END_STACK_CHECK( L );
	return r;
}

/*/////////////////////////////////////////////////////////////////////////

function: call - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hInt32 hlLuaReader::call( const hlLuaTable* env, const hChar* fname, const hChar* format, ... )
{
	if ( nLastReturn_ != 0 )
	{
		clearReturnResults();
	}

	lua_State* L = pLua_->pLuaState_;

	START_STACK_CHECK( L );

	hInt32 prevtop = lua_gettop( L );
	hInt32 nargs = 0;

	if ( env == NULL )
	{
		// push the function [1/26/2009 James]
		lua_getglobal( L, fname );
	}
	else
	{
		// push the function from the table[1/26/2009 James]
		env->getVarAsFunction( fname );
	}

	hcAssert( lua_isfunction( L, -1 ) == hTrue );
	if ( !lua_isfunction( L, - 1 ) )
	{
		lua_settop( L, prevtop );
		END_STACK_CHECK( L );
		return -1;
	}

	va_list args;
	va_start( args, format );

	for ( hUint32 i = 0; i < strlen( format ); i = i + 2 )
	{
		if ( format[ i ] == '%' )
		{
			++nargs;
			switch( format[ i + 1 ] )
			{
			case 'i':
			case 'I':
				{
					hInt32* i = va_arg( args, hInt32* );
					lua_pushinteger( L, *i );
				}
				break;
			case 'f':
			case 'F':
				{
					hFloat* f = va_arg( args, hFloat* );
					lua_pushnumber( L, *f );
				}
				break;
			case 's':
			case 'S':
				{
					hChar* c = va_arg( args, hChar* );
					lua_pushstring( L, c );
				}
				break;
			case 't':
			case 'T':
				{
					hlLuaTable* c = va_arg( args, hlLuaTable* );
					lua_rawgeti( L, LUA_REGISTRYINDEX, c->ValIdx() );
				}
				break;
			default:
				hcBreak;
				va_end( args );
				lua_settop( L, prevtop );
				END_STACK_CHECK( L );
				return 0;
				break;
			}
		}
	}
	

	if ( lua_pcall( L, nargs, LUA_MULTRET, 0 ) )
	{
		// had an error [1/26/2009 James]
		hcPrintf( "LUA ERROR: %s\n", lua_tostring( L, - 1 ) );
		lua_settop( L, prevtop );
		va_end( args );
		END_STACK_CHECK( L );
		return -1;
	}
	
	va_end( args );
	nLastReturn_ = prevtop - lua_gettop( L );
	return nLastReturn_;
}

/*/////////////////////////////////////////////////////////////////////////

function: getVarAsInt - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hInt32 hlLuaReader::getVarAsInt( const hChar* var )
{
	lua_State* L = pLua_->pLuaState_;
	START_STACK_CHECK( L );

	lua_getglobal( L, var );

	hcAssert( lua_isnumber( L, -1 ) );
	if ( !lua_isnumber( L, -1 ) )
	{
		// variable didn't exist so return a default variable [1/26/2009 James]
		return 0;
	}

	hInt32 r = lua_tointeger( L, -1 );

	lua_pop( L, 1 );

	END_STACK_CHECK( L );
	return r;	
}

/*/////////////////////////////////////////////////////////////////////////

function: getVarAsFloat - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hFloat hlLuaReader::getVarAsFloat( const hChar* var )
{
	lua_State* L = pLua_->pLuaState_;
	START_STACK_CHECK( L );

	lua_getglobal( L, var );

	hcAssert( lua_isnumber( L, -1 ) );
	if ( !lua_isnumber( L, -1 ) )
	{
		// variable didn't exist so return a default variable [1/26/2009 James]
		return 0.0f;
	}

	hFloat r = static_cast< hFloat>( lua_tonumber( L, -1 ) );

	lua_pop( L, 1 );

	END_STACK_CHECK( L );
	return r;	
}

/*/////////////////////////////////////////////////////////////////////////

function: clearReturnResults - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
void hlLuaReader::clearReturnResults()
{
	if ( nLastReturn_ != 0 )
	{
		lua_pop( pLua_->pLuaState_, nLastReturn_ );
	}
}

/*/////////////////////////////////////////////////////////////////////////

function: getVarAsTable - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hlLuaTable hlLuaReader::getVarAsTable( const hChar* var )
{
	lua_State* L = pLua_->pLuaState_;
	START_STACK_CHECK( L );

	lua_getglobal( L, var );

	hcAssert( lua_istable( L, -1 ) == hTrue );

	hlLuaTable r( -1 );

	lua_pop( L, -1 );// pop the global var [1/26/2009 James]

	END_STACK_CHECK( L );
	return r;
}

/*/////////////////////////////////////////////////////////////////////////

function: isTable - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
hBool hlLuaReader::isTable( const hChar* var )
{
	lua_State* L = pLua_->pLuaState_;
	START_STACK_CHECK( L );

	lua_getglobal( L, var );

	hBool r = lua_istable( L, -1 ) == hTrue;

	lua_pop( L, -1 );

	END_STACK_CHECK( L );

	return r;
}

/*/////////////////////////////////////////////////////////////////////////

function: report - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
void hlLuaReader::report()
{
	hcPrintf( "Lua Report: \n\tMemory Usage = %dKb\n", lua_gc( pLua_->pLuaState_, LUA_GCCOUNT, 0 ) );
}

/*/////////////////////////////////////////////////////////////////////////

function: fullGC - 

purpose: 

author: James Moran

/////////////////////////////////////////////////////////////////////////*/
void hlLuaReader::fullGC()
{
	lua_gc( pLua_->pLuaState_, LUA_GCCOLLECT, 0 );
}