#ifndef LUASOCKET_H
#define LUASOCKET_H
/*=========================================================================*\
* LuaSocket toolkit
* Networking support for the Lua language
* Diego Nehab
* 9/11/1999
*
* RCS ID: $Id: luasocket.h,v 1.17 2004/07/26 04:03:55 diego Exp $
\*=========================================================================*/
#include <lua.h>

/*-------------------------------------------------------------------------*\
* Current luasocket version
\*-------------------------------------------------------------------------*/
#define LUASOCKET_VERSION "LuaSocket 2.0 (beta2)"

/*-------------------------------------------------------------------------*\
* This macro prefixes all exported API functions
\*-------------------------------------------------------------------------*/
#ifndef LUASOCKET_API
#define LUASOCKET_API extern
#endif

/*-------------------------------------------------------------------------*\
* Initializes the library.
\*-------------------------------------------------------------------------*/
LUASOCKET_API int luaopen_socket(lua_State *L);

#endif /* LUASOCKET_H */
