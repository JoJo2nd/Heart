/********************************************************************
	created:	2010/10/03
	created:	3:10:2010   11:35
	filename: 	LuaStateManager.h	
	author:		James
	
	purpose:	
*********************************************************************/

#ifndef LUASTATEMANAGER_H__
#define LUASTATEMANAGER_H__

#include "hTypes.h"
#include "hMemory.h"
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};

namespace Heart
{
	class hIFileSystem;

	typedef huFunctor< hBool (*)( lua_State* ) >::type VMYieldCallback;
	typedef huFunctor< void (*)( lua_State* ) >::type VMResumeCallback;

	class hLuaStateManager
	{
	public:
		hLuaStateManager();
		virtual ~hLuaStateManager();

		void			Initialise( hIFileSystem** filesystems );
		void			Destroy();
		void			ExecuteBuffer( const hChar* buff, hUint32 size );
		void			Update();
		void			RegisterLuaFunctions( luaL_Reg* );

		static hLuaStateManager*	GetInstance()
		{
			return gLuaStateManagerInstance;
		}

	private:

        struct LuaThreadState : hLinkedListElement< LuaThreadState >
		{
			LuaThreadState() 
                : lua_(NULL)
				, status_(0)
				, yieldRet_(0)
			{}
			lua_State*			lua_;
			hUint32				status_;				
			hInt32				yieldRet_;
		};

		typedef hLinkedList< LuaThreadState > ThreadList;

		hBool			RunLuaThread( LuaThreadState* i );
		lua_State*		NewLuaState( lua_State* parent );
		static void*	LuaAlloc( void *ud, void *ptr, size_t osize, size_t nsize );
		static int		LuaPanic (lua_State* L);
		static void		LuaHook( lua_State* L, lua_Debug* LD );

		static hLuaStateManager*    gLuaStateManagerInstance;
		lua_State*		            mainLuaState_;
		ThreadList		            luaThreads_;
	};
}


#endif // SQUIRRELWRAPPER_H__