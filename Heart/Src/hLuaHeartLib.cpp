/********************************************************************

	filename: 	LuaHeartLib.cpp	
	
	Copyright (c) 8:5:2011 James Moran
	
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

namespace Heart
{
/*
typedef int (*lua_CFunction) (lua_State *L);
Type for C functions.

In order to communicate properly with Lua, a C function must use the following protocol, 
which defines the way parameters and results are passed: a C function receives its arguments 
from Lua in its stack in direct order (the first argument is pushed first). So, when the 
function starts, lua_gettop(L) returns the number of arguments received by the function. The 
first argument (if any) is at index 1 and its last argument is at index lua_gettop(L). To return 
values to Lua, a C function just pushes them onto the stack, in direct order (the first result 
is pushed first), and returns the number of results. Any other value in the stack 
below the results will be properly discarded by Lua. Like a Lua function, a C function called 
by Lua can also return many results.

*/

	int WaitCheck( lua_State* L )
	{
		float startTime = luaL_checknumber( L, 1 );
		float waitTime = luaL_checknumber( L, 2 );

		if ( (startTime+waitTime) < hClock::elapsed() )
		{
			lua_pushboolean( L, hTrue );
			return 1;
		}

		lua_pushboolean( L, hFalse );
		return 1;
	}

	int Wait( lua_State* L )
	{
		float waitTime = luaL_checknumber( L, 1 );

		lua_pushcfunction( L, WaitCheck );
		lua_pushnumber( L, hClock::elapsed() );
		lua_pushnumber( L, waitTime );
		return lua_yield( L, 3 );
	}

	int Elasped( lua_State* L )
	{
		lua_pushnumber( L, hClock::elapsed() );
		return 1;
	}

	int ElaspedHoursMinSecs( lua_State* L )
	{
		lua_pushinteger( L, hClock::hours() );
		lua_pushinteger( L, hClock::mins() );
		lua_pushinteger( L, hClock::secs() );
		return 3;
	}

	int EnableDebugDraw( lua_State* L )
	{
		int enable = luaL_checkinteger( L, -1 );
		
		Heart::DebugRenderer::EnableDebugDrawing( enable > 0 );

		return 0;
	}

	static const luaL_Reg heartlib[] = {
		{"Elasped",			Elasped},
		{"ElaspedHMS",		ElaspedHoursMinSecs},
		{"Wait",			Wait},
		{"EnableDebugDraw", EnableDebugDraw },
		{NULL, NULL}
	};

	void OpenHeartLib( lua_State* L )
	{
		luaL_register( L, "Heart", heartlib );
	}
}