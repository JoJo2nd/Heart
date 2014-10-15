/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef LUNA_H
#define LUNA_H

/*****************************************************************************
 *     .:: Luna ::.                                                          *
 *                                                                *   , *    *
 *  C++ library for binding classes into Lua.     By nornagon.       ((   *  *
 *                                                               *    `      *
 *  Example:                                                                 *
 *****************************************************************************

    class Foo {
      public:
        Foo(lua_State *L) {
          printf("in constructor\n");
        }

        int foo(lua_State *L) {
          printf("in foo\n");
        }

        ~Foo() {
          printf("in destructor\n");
        }

        static const char className[];
        static const Luna<Foo>::RegType Register[];
    };

    const char Foo::className[] = "Foo";
    const Luna<Foo>::RegType Foo::Register[] = {
      { "foo", &Foo::foo },
      { 0 }
    };

 *****************************************************************************
 * Then:                                                                     *
 *****************************************************************************

    Luna<Foo>::Register(L);

 *****************************************************************************
 * From Lua:                                                                 *
 *****************************************************************************

    local foo = Foo()
    foo:foo()

 *****************************************************************************
 * Clean up:                                                                 *
 *****************************************************************************

    lua_close(L);

 *****************************************************************************
 * Output:                                                                   *
 *****************************************************************************

    in constructor
    in foo
    in destructor

 *****************************************************************************
 * This program is free software. It comes without any warranty, to          *
 * the extent permitted by applicable law. You can redistribute it           *
 * and/or modify it under the terms of the Do What The Fuck You Want         *
 * To Public License, Version 2, as published by Sam Hocevar. See            *
 * http://sam.zoy.org/wtfpl/COPYING for more details.                        *
 ****************************************************************************/
 

// convenience macros
#define luna_register(L, klass) (Luna<klass>::Register((L)))
#define luna_registermetatable(L, klass) (Luna<klass>::RegisterMetatable((L)))
#define luna_inject(L, klass, t) (Luna<klass>::inject((L), (t)))

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

template< typename _Ty >
struct LuaBindStatckObject
{
	static _Ty pop( lua_State* L, int i )
	{
		return luaL_checkudata( L, i, Luna< _Ty >::classname );
	}

	static void push( lua_State* L, _Ty& val )
	{
		Luna< _Ty >::copyConstructor( L, val );
	}

	static void push( lua_State* L, _Ty* val )
	{
		Luna< _Ty >::inject( L, val );
	}
};

//specialise
template<>
struct LuaBindStackObject< int >
{
	static _Ty pop( lua_State* L, int i )
	{
		return luaL_checkinteger( L, i );
	}

	static void push( lua_State* L, _Ty& val )
	{
		lua_pushinteger( L, val );
	}

	static void push( lua_State* L, _Ty* val )
	{
		lua_pushinteger( L, *val );
	}
};

template<>
struct LuaBindStackObject< float >
{
	static _Ty pop( lua_State* L, int i )
	{
		return luaL_checknumber( L, i );
	}

	static void push( lua_State* L, _Ty& val )
	{
		lua_pushnumber( L, val );
	}

	static void push( lua_State* L, _Ty* val )
	{
		lua_pushnumber( L, *val );
	}
};

template<>
struct LuaBindStackObject< char* >
{
	static _Ty pop( lua_State* L, int i )
	{
		//const problem?
		return lua_tostring( L, i );
	}

	static void push( lua_State* L, const _Ty& val )
	{
		lua_pushinteger( L, val );
	}

	static void push( lua_State* L, const _Ty* val )
	{
		lua_pushinteger( L, *val );
	}
};


template< typename _R >
int staticThunk( lua_State* L ) 
{
	typedef _R (*func_sig_type)();
	//TODO: error check
	//get func pointer from upvalue
	func_sig_type fp = (func_sig_type)lua_touserdata(L, lua_upvalueindex(1));
	//pop each element from the stack (with error check) LuaBindStackObject<_Ty>::pop( L, idx )
	//call function
	_R r = (*fp)();
	//push return (specilaise for void) LuaBindStackObject<_Ty>::push( L )
	LuaBindStatckObject< _R >::push( L, r );
	//return 1 (or 0 for void)
	return 1
}

template< typename _R, typename _P1 >
int staticThunk( lua_State* L ) 
{
	typedef _R (*func_sig_type)(_P1);
	//TODO: error check
	//get func pointer from upvalue
	func_sig_type fp = (func_sig_type)lua_touserdata(L, lua_upvalueindex(1));
	//pop each element from the stack (with error check) LuaBindStackObject<_Ty>::pop( L, idx )
	_P1 p1 = LuaBindStatckObject< _R >::pop( L, 1 );
	//call function
	_R r = (*fp)(p1);
	//push return (specilaise for void) LuaBindStackObject<_Ty>::push( L )
	LuaBindStatckObject< _R >::push( L, r );
	//return 1 (or 0 for void)
	return 1
}

template<class T> 
class Luna 
{
public:
    static void Register(lua_State *L, const char* name, const char* fntablename ) 
	{
		classname = name;
		classfntable = fntablename;

		lua_pushcfunction(L, &Luna<T>::constructor);
		lua_setglobal(L, classname); // T() in lua will make a new instance.

		RegisterMetatable(L);
    }

    // register the metatable without registering the class constructor
    static void RegisterMetatable(lua_State *L) 
	{
      luaL_newmetatable( L, classname ); // create a metatable in the registry
      lua_pushstring(L, "__gc");
      lua_pushcfunction(L, &Luna<T>::gc_obj);
      lua_settable(L, -3); // metatable["__gc"] = Luna<T>::gc_obj
      lua_pop(L, 1);
    }

	static void RegisterFunctionTable(lua_State* L)
	{
		lua_getregistry(L);
		lua_pushstring(L,classfntable);
		lua_newtable(L);
		lua_settable( L, -3 );//registry["classfntable"] = new table

	}

	template< typename _R, typename _P1 >
	static void RegisterFunction( lua_State* L, _R(*fn)(_P1), const char* name )
	{
		lua_getregistry( L );
		lua_pushstring( L, classfntable );
		lua_gettable( L, -2 );//push registry["classfntable"] to top of stack
		lua_pushlightuserdata( L, (void*)fn );
		lua_pushcclosure( L, &staticThunk<fn>, 1 );//push thunk with 1 up value (upvalue is function pointer)
		lua_setfield( L, -2, name ); // registry["classfntable"].["name"] = staticThunk<fn>
	}

	static void RegisterFunctionRaw( lua_State* L, int(*fn)(lua_State*), const char* name )
	{

	}

	static int copyConstructor( lua_State* L, const _Ty& c )
	{
		return inject(L, new T(c));
	}

    static int constructor(lua_State *L) 
	{
      return inject(L, new T);
    }

    static int inject(lua_State *L, T* obj) 
	{
		lua_newtable(L); // create a new table for the class object ('self')

		lua_pushnumber(L, 0);

		T** a = static_cast<T**>(lua_newuserdata(L, sizeof(T*))); // store a ptr to the ptr
		*a = obj; // set the ptr to the ptr to point to the ptr... >.>
		luaL_newmetatable(L, classfntable); // get (or create) the unique metatable
		lua_setmetatable(L, -2); // self.metatable = uniqe_metatable

		lua_settable(L, -3); // self[0] = obj;

//       for (int i = 0; T::Register[i].name; i++) 
// 	  { // register the functions
//         lua_pushstring(L, T::Register[i].name);
//         lua_pushnumber(L, i); // let the thunk know which method we mean
//         lua_pushcclosure(L, &Luna<T>::thunk, 1);
//         lua_settable(L, -3); // self["function"] = thunk("function")
//       }
		//TODO: copy from table
		lua_getregistry( L );
		lua_pushstring( L, classfntable );
		lua_gettable( L, -2 );//push registry["classfntable"] to top of stack
		// table is in the stack at index 't' 
		lua_pushnil( L );  // first key
		while (lua_next( L, -2 ) != 0) 
		{
			// uses 'key' (at index -2) and 'value' (at index -1) 
			// copy 'key' & 'value' to set into object table
			lua_pushvalue( L, -2 );//key
			lua_pushvalue( L, -2 );//value
			lua_settable( L, -6 );//registry["classfntable"]."key" = "value"
			// removes 'value'; keeps 'key' for next iteration 
			lua_pop(L, 1);
		}
		return 1;
    }

//     static int thunk(lua_State *L) 
// 	{
//       // redirect method call to the real thing
//       int i = (int)lua_tonumber(L, lua_upvalueindex(1)); // which function?
//       lua_pushnumber(L, 0);
//       lua_gettable(L, 1); // get the class table (i.e, self)
// 
//       T** obj = static_cast<T**>(luaL_checkudata(L, -1, T::className));
//       lua_remove(L, -1); // remove the userdata from the stack
// 
//       return ((*obj)->*(T::Register[i].mfunc))(L); // execute the thunk
//     }

    static int gc_obj(lua_State *L) 
	{
      // clean up
      //printf("GC called: %s\n", T::className);
      T** obj = static_cast<T**>( luaL_checkudata(L, -1, classname ) );
      delete (*obj);
      return 0;
    }

	static const char* classname;
	static const char* classfntable;

    struct RegType 
	{
      const char *name;
      int(T::*mfunc)(lua_State*);
    };
};


#endif /* LUNA_H */
