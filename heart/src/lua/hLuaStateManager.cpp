/********************************************************************

    filename: 	hLuaStateManager.cpp	
    
    Copyright (c) 1:4:2012 James Moran
    
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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace Heart
{

#define HLUA_NEWTHREAD		(LUA_ERRERR+1)
#define HLUA_WAKEUP			(LUA_ERRERR+2)

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hLuaStateManager::hLuaStateManager() 
        : mainLuaState_( NULL )
    {
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hLuaStateManager::~hLuaStateManager()
    {
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLuaStateManager::Initialise()
    {
        mainLuaState_ = NewLuaState( NULL );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLuaStateManager::Destroy()
    {
        lua_close( mainLuaState_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLuaStateManager::Update()
    {
        HEART_PROFILE_FUNC();
        for ( hLuaThreadState* i = luaThreads_.GetHead(); i != NULL; )
        {
            hLuaThreadState* next = i->GetNext();
            if ( RunLuaThread( i ) )
            {
                hLuaThreadState* removed = luaThreads_.Remove( i );
                hDELETE(GetGlobalHeap()/*!heap*/, removed);
            }

            i = next;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLuaStateManager::ExecuteBuffer( const hChar* buff, hUint32 size )
    {
        if (luaL_dostring(mainLuaState_, buff) != 0) {
            hcPrintf(__FUNCTION__" Failed to run, Error: %s", lua_tostring(mainLuaState_, -1));
            lua_pop(mainLuaState_, 1);
        }
//         hLuaThreadState newthread;
//         newthread.lua_ = NewLuaState( mainLuaState_ );
//         newthread.status_ = HLUA_NEWTHREAD;
//         newthread.yieldRet_ = 0;
// 
//         luaL_loadstring( newthread.lua_, buff );
//         newthread.status_ = lua_resume( newthread.lua_, NULL, 0 );
//         //status of 0 is completed OK, so we leave that be and 
//         //let lua GC collect the thread object.
//         if ( newthread.status_ == LUA_YIELD )
//         {
//             newthread.yieldRet_ = lua_gettop( newthread.lua_ );
//             hLuaThreadState* listState = hNEW(GetGlobalHeap()/*!heap*/, hLuaThreadState);
//             *listState = newthread;
//             luaThreads_.PushBack( listState );
//         }
//         else if ( newthread.status_ > LUA_YIELD )
//         {
//             //Error: printf the error from the top of the stack
//             if ( lua_isstring( newthread.lua_, -1 ) )
//             {
//                 hcPrintf( "Lua Error: %s", lua_tostring( newthread.lua_, -1 ) );
//             }
//             else
//             {
//                 hcPrintf( "Lua Error: Couldn't Get Error Message" );
//             }
//         }
    }

    //////////////////////////////////////////////////////////////////////////
    // 21:52:13 ////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* hLuaStateManager::LuaAlloc( void *ud, void *ptr, size_t osize, size_t nsize )
    {
        (void)ud;
        (void)osize;
        if (nsize == 0) 
        {
            hHeapFree(GetGlobalHeap()/*!heap*/, ptr );
            return NULL;
        }
        else
        {
            return hHeapRealloc(GetGlobalHeap()/*!heap*/, ptr, nsize);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // 21:57:53 ////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int hLuaStateManager::LuaPanic( lua_State *L )
    {
        (void)L;  /* to avoid warnings */
        hcPrintf( "Lua PANIC: unprotected error in call to Lua API (%s)\n", lua_tostring(L, -1) );
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    // 23:25:22 ////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hLuaStateManager::RunLuaThread( hLuaThreadState* luaTState )
    {
        hLuaThreadState* ts = luaTState;
        if ( ts->status_ == LUA_YIELD )
        {
            int status = HLUA_WAKEUP;
            int top = lua_gettop( ts->lua_ );
            int newtop = lua_gettop( ts->lua_ );

            //If Nothing on the stack to check this yield against so just resume
            if ( ts->yieldRet_ > 0 )
            {
                if ( lua_isfunction(ts->lua_, -ts->yieldRet_ ) )
                {
                    newtop = lua_gettop( ts->lua_ ) - ts->yieldRet_;
                    //attempt to call this
                    for ( int i = newtop+1; i <= newtop+ts->yieldRet_; ++i )
                    {
                        lua_pushvalue( ts->lua_, i );
                    }

                    newtop = lua_gettop( ts->lua_ );
                    if ( lua_pcall( ts->lua_, ts->yieldRet_-1, 1, 0 ) != 0 )
                    {
                        //Hit error in resume call, just report it and resume
                        hcPrintf( "Lua Error calling yield function : %s", lua_tostring( ts->lua_, -1 ) );
                        //remove the error string
                        lua_pop( ts->lua_, 1 );
                        status = HLUA_WAKEUP;
                    }
                    else
                    {
                        //check that there is a bool on top of stack to check
                        newtop = lua_gettop( ts->lua_ );
                        if ( lua_isboolean( ts->lua_, -1 ) )
                        {
                            int ret = lua_toboolean( ts->lua_, -1 );
                            //must return stack to prev state before calling 
                            //resume
                            lua_pop( ts->lua_, 1 );
                            if ( ret )
                            {
                                //ok to resume the thread
                                status = HLUA_WAKEUP;
                            }
                            else
                            {
                                //still waiting
                                status = LUA_YIELD;
                            }
                        }
                        else
                        {
                            //unknown result, pop results (whatever they were)
                            // warn in log and resume, 
                            hcWarningHigh( true, "WARNING: yield check function returned unexpected result. resuming thread anyway." );
                            lua_pop( ts->lua_, 1 );
                            status = HLUA_WAKEUP;
                        }
                    }
                }
                else
                {
                    //unknown at stack bottom, just resume
                    status = HLUA_WAKEUP;
                }
            }

            if ( status == HLUA_WAKEUP )
            {
                status = lua_resume( ts->lua_, NULL, ts->yieldRet_ );
                if ( status == LUA_YIELD )
                {
                    ts->yieldRet_ = lua_gettop( ts->lua_ );
                }
            }

            ts->status_ = status;

            //Warn about errors
            if ( status > LUA_YIELD )
            {
                //TODO: call debugger...

                if ( lua_isstring( ts->lua_, -1 ) )
                {
                    hcPrintf( "Lua Error: %s", lua_tostring( ts->lua_, -1 ) );
                }
                else
                {
                    hcPrintf( "Lua Error: Couldn't Get Error Message" );
                }

            }
        }

        return ts->status_ != LUA_YIELD;
    }

    //////////////////////////////////////////////////////////////////////////
    // 9:57:32 ////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLuaStateManager::LuaHook( lua_State* L, lua_Debug* LD )
    {
        (void)L;
        (void)LD;
        //seems that LD->currentline == -1 when source is unknown
        //name,source,what,namewhat are duff pointers
    }

    //////////////////////////////////////////////////////////////////////////
    // 10:00:52 ////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    lua_State* hLuaStateManager::NewLuaState( lua_State* parent )
    {
        lua_State* L = NULL;
        if ( parent )
        {
            L = lua_newthread( parent );
        }
        else
        {
            L = lua_newstate( LuaAlloc, NULL );
            luaL_openlibs( L );
            lua_atpanic( L, LuaPanic );
            lua_sethook( L, LuaHook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0 );
        }
        return L;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLuaStateManager::RegisterLuaFunctions( luaL_Reg* libfunc )
    {
        lua_getglobal(mainLuaState_, "heart");
        luaL_setfuncs(mainLuaState_,libfunc,0);
        lua_pop(mainLuaState_, 1);// pop heart module table
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hComponent* hLuaStateManager::LuaScriptComponentCreate( hEntity* owner )
    {
        hLuaThreadState* newthread = hNEW(GetGlobalHeap()/*!heap*/, hLuaThreadState);
        newthread->lua_ = NewLuaState( mainLuaState_ );
        newthread->status_ = HLUA_NEWTHREAD;
        newthread->yieldRet_ = 0;

        return hNEW(GetGlobalHeap()/*!heap*/, hLuaScriptComponent)(owner, newthread);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLuaStateManager::LuaScriptComponentDestroy( hComponent* luaComp )
    {
        hDELETE(GetGlobalHeap()/*!heap*/, luaComp);
    }

}

