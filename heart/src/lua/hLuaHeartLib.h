/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef LUAHEARTLIB_H__
#define LUAHEARTLIB_H__

struct lua_State;
class hHeartEngine;

namespace Heart
{
	void OpenHeartLuaLib( lua_State* L, hHeartEngine* engine );
}

#endif // LUAHEARTLIB_H__