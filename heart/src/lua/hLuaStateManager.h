/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/


#ifndef LUASTATEMANAGER_H__
#define LUASTATEMANAGER_H__

#include "base/hTypes.h"    
extern "C" {
#   include "lua.h"
#   include "lualib.h"
#   include "lauxlib.h"
}
#include "base/hLinkedList.h"
#include "base/hFunctor.h"

namespace Heart
{
#define HEART_LUA_GET_ENGINE(L) \
    hHeartEngine* engine = (hHeartEngine*)lua_topointer(L, lua_upvalueindex(1)); \
    if (!engine) luaL_error(L, "Unable to grab engine pointer" ); \

	class hIFileSystem;
    class hEntity;
    class hComponent;
    class hLuaScriptComponent;

	typedef hFunctor< hBool (*)( lua_State* ) >::type VMYieldCallback;
	typedef hFunctor< void (*)( lua_State* ) >::type VMResumeCallback;

    struct hLuaThreadState : hLinkedListElement< hLuaThreadState >
    {
        hLuaThreadState() 
            : lua_(NULL)
            , status_(0)
            , yieldRet_(0)
        {}
        lua_State*			lua_;
        hUint32				status_;				
        hInt32				yieldRet_;
    };

	class hLuaStateManager
	{
	public:
		hLuaStateManager();
		virtual ~hLuaStateManager();

		void			Initialise();
		void			Destroy();
		void			ExecuteBuffer( const hChar* buff, hUint32 size );
		void			Update();
		void			RegisterLuaFunctions( luaL_Reg* );
        lua_State*      GetMainState() { return mainLuaState_; }

	private:

		typedef hLinkedList< hLuaThreadState > ThreadList;

		hBool			            RunLuaThread( hLuaThreadState* i );
		lua_State*		            NewLuaState( lua_State* parent );
		static void*	            LuaAlloc( void *ud, void *ptr, size_t osize, size_t nsize );
		static int		            LuaPanic (lua_State* L);
		static void		            LuaHook( lua_State* L, lua_Debug* LD );

		lua_State*		            mainLuaState_;
		ThreadList		            luaThreads_;
	};
}


#endif // SQUIRRELWRAPPER_H__