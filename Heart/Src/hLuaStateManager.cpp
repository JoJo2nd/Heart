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

static Heart::hIFileSystem* gLuaFileSystems[MAX_LUA_FILESYSTEMS] = {NULL};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace Heart
{

#define HLUA_NEWTHREAD		(LUA_ERRERR+1)
#define HLUA_WAKEUP			(LUA_ERRERR+2)

	hLuaStateManager* hLuaStateManager::gLuaStateManagerInstance = NULL;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hLuaStateManager::hLuaStateManager() :
		mainLuaState_( NULL )
	{
		gLuaStateManagerInstance = this;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hLuaStateManager::~hLuaStateManager()
	{
		gLuaStateManagerInstance = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hLuaStateManager::Initialise( hIFileSystem** filesystems )
	{
		for ( hUint32 i = 0; i < MAX_LUA_FILESYSTEMS && filesystems[i] != NULL; ++i )
		{
			gLuaFileSystems[i] = filesystems[i];
		}

		mainLuaState_ = NewLuaState( NULL );
		OpenHeartLib( mainLuaState_ );
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
		for ( hLuaThreadState* i = luaThreads_.GetHead(); i != NULL; )
		{
            hLuaThreadState* next = i->GetNext();
			if ( RunLuaThread( i ) )
			{
				hLuaThreadState* removed = luaThreads_.Remove( i );
                hDELETE(hGeneralHeap, removed);
			}

            i = next;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hLuaStateManager::ExecuteBuffer( const hChar* buff, hUint32 size )
	{
		hLuaThreadState newthread;
		newthread.lua_ = NewLuaState( mainLuaState_ );
		newthread.status_ = HLUA_NEWTHREAD;
		newthread.yieldRet_ = 0;

		luaL_loadstring( newthread.lua_, buff );
		newthread.status_ = lua_resume( newthread.lua_, 0 );
		//status of 0 is completed OK, so we leave that be and 
		//let lua GC collect the thread object.
		if ( newthread.status_ == LUA_YIELD )
		{
			newthread.yieldRet_ = lua_gettop( newthread.lua_ );
            hLuaThreadState* listState = hNEW(hGeneralHeap, hLuaThreadState);
            *listState = newthread;
			luaThreads_.PushBack( listState );
		}
		else if ( newthread.status_ > LUA_YIELD )
		{
			//Error: printf the error from the top of the stack
			if ( lua_isstring( newthread.lua_, -1 ) )
			{
				hcPrintf( "Lua Error: %s", lua_tostring( newthread.lua_, -1 ) );
			}
			else
			{
				hcPrintf( "Lua Error: Couldn't Get Error Message" );
			}
		}
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
			hVMHeap.release( ptr );
			return NULL;
		}
		else if ( !ptr )
		{
			return hHeapMalloc(hVMHeap, nsize);
		}
		else 
		{
			return hHeapRealloc(hVMHeap, ptr, nsize);
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
				status = lua_resume( ts->lua_, ts->yieldRet_ );
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
		luaL_register( mainLuaState_, "Heart", libfunc );
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hComponent* hLuaStateManager::LuaScriptComponentCreate( hEntity* owner )
    {
        hLuaThreadState* newthread = hNEW(hGeneralHeap, hLuaThreadState);
        newthread->lua_ = NewLuaState( mainLuaState_ );
        newthread->status_ = HLUA_NEWTHREAD;
        newthread->yieldRet_ = 0;

        return hNEW(hVMHeap, hLuaScriptComponent(owner, newthread));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLuaStateManager::LuaScriptComponentDestroy( hComponent* luaComp )
    {
        hDELETE(hGeneralHeap, luaComp);
    }

}

extern "C"
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	int hlCheckReadable( const char* filename )
	{
		for ( hUint32 i = 0; i < MAX_LUA_FILESYSTEMS && gLuaFileSystems[i]; ++i )
		{
			Heart::hIFile* file = gLuaFileSystems[i]->OpenFile( filename, Heart::FILEMODE_READ );
			if ( file )
			{
				gLuaFileSystems[i]->CloseFile( file );
				return 1;
			}
		}
		return 0;
	}

	struct File
	{
		Heart::hIFile* file_;
		Heart::hIFileSystem* fileSystem_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void* hLuafopen( const char* filename, const char* mode )
	{
		(void)mode;//always read
		for ( hUint32 i = 0; i < MAX_LUA_FILESYSTEMS && gLuaFileSystems[i]; ++i )
		{
			Heart::hIFile* file = gLuaFileSystems[i]->OpenFile( filename, Heart::FILEMODE_READ );
			if ( file )
			{
				File* ff = hNEW(hVMHeap, File);
				ff->file_ = file;
				ff->fileSystem_ = gLuaFileSystems[i];
				return ff;
			}
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	int hLuagetc( void* f )
	{
		Heart::hIFile* file = (Heart::hIFile*)((File*)f)->file_;
		char c;
		hUint32 bytes;

		bytes = file->Read( &c, 1 );

		return bytes != 1 ? EOF : c;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	int hLuaungetc( int c, void* f )
	{
		Heart::hIFile* file = (Heart::hIFile*)((File*)f)->file_;
		file->Seek( -1, Heart::SEEKOFFSET_CURRENT );

		return c;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	size_t hLuafread( void* dst, size_t size, size_t count, void* f )
	{
		Heart::hIFile* file = (Heart::hIFile*)((File*)f)->file_;
		hUint32 bytes;

		bytes = file->Read( dst, size*count );

		return bytes;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	int hLuafclose( void* f )
	{
		Heart::hIFile* file = (Heart::hIFile*)((File*)f)->file_;
		((File*)f)->fileSystem_->CloseFile( file );
		delete f;
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	int hLuafeof( void* f )
	{
		Heart::hIFile* file = (Heart::hIFile*)((File*)f)->file_;
		if ( file->Tell() >= file->Length() )
			return 1;
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	size_t hLuaflen( void* f ) 
	{
		Heart::hIFile* file = (Heart::hIFile*)((File*)f)->file_;
		return (size_t)file->Length();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	static int errfile (lua_State *L, const char *what, int fnameindex) 
	{
		const char *serr = strerror(errno);
		const char *filename = lua_tostring(L, fnameindex) + 1;
		lua_pushfstring(L, "cannot %s %s: %s", what, filename, serr);
		lua_remove(L, fnameindex);
		return LUA_ERRFILE;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//Duplicate of lauxlib.c struct
	typedef struct LoadF {
		int extraline;
		void* f;
		char* buff;
	} LoadF;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	static const char *getF (lua_State *L, void *ud, size_t *size) 
	{
		LoadF *lf = (LoadF *)ud;
		(void)L;
		if (lf->extraline) 
		{
			lf->extraline = 0;
			*size = 0;
			delete lf->buff;
			lf->buff = NULL;
			return NULL;
		}
		//Can't test for eof so assume all ok
		//Instead we read teh entire file
// 		if (hLuafeof(lf->f)) 
// 		{	
// 			return NULL;
// 		}
		lf->buff = hNEW_ARRAY(hVMHeap, char, hLuaflen(lf->f));
		lf->extraline = 1;
		*size = hLuafread(lf->buff, 1, hLuaflen(lf->f), lf->f);
		return (*size > 0) ? lf->buff : NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	LUALIB_API int luaL_loadfile (lua_State *L, const char *filename) 
	{
		LoadF lf;
		int status;//, readstatus;
		int c;
		int fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
		lf.extraline = 0;
		//We don't support stdin
		if (filename == NULL) 
		{
			return errfile( L, "no filename", fnameindex );
		}
		else 
		{
			lua_pushfstring(L, "@%s", filename);
			lf.f = hLuafopen(filename, "r");
			if (lf.f == NULL) 
			{
				return errfile(L, "open", fnameindex);
			}
		}
		c = hLuagetc(lf.f);
		//Don't worry about unix exec. files
// 		if (c == '#') 
// 		{  /* Unix exec. file? */
// 			lf.extraline = 1;
// 			while ((c = getc(lf.f)) != EOF && c != '\n') ;  /* skip first line */
// 			if (c == '\n') c = getc(lf.f);
// 		}
		if (c == LUA_SIGNATURE[0] && filename) 
		{  /* binary file? */
			//Don't need to reopen 
			//lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
			//if (lf.f == NULL) return errfile(L, "reopen", fnameindex);
			/* skip eventual `#!...' */
			while ((c = hLuagetc(lf.f)) != EOF && c != LUA_SIGNATURE[0]) ;
			lf.extraline = 0;
		}
		hLuaungetc(c, lf.f);
		status = lua_load(L, getF, &lf, lua_tostring(L, -1));
		//Can't test for eof so assume all ok
		//TODO: fix this?
		//readstatus = ferror(lf.f);
		if (filename) 
		{
			hLuafclose(lf.f);  /* close file (even in case of errors) */
		}
		if ( lf.buff )
		{
			delete lf.buff;
			lf.buff = NULL;
		}
// 		if (readstatus) 
// 		{
// 			lua_settop(L, fnameindex);  /* ignore results from `lua_load' */
// 			return errfile(L, "read", fnameindex);
// 		}
		lua_remove(L, fnameindex);
		return status;
	}

//TODO:
// Define our own version of require that allow the library to be loaded async while blocking the script.
#if 0
	static const int sentinel_ = 0;
#define sentinel	((void *)&sentinel_)


	static int ll_require (lua_State *L) 
	{
		const char *name = luaL_checkstring(L, 1);
		int i;
		lua_settop(L, 1);  /* _LOADED table will be at index 2 */
		lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
		lua_getfield(L, 2, name);
		if (lua_toboolean(L, -1)) {  /* is it there? */
			if (lua_touserdata(L, -1) == sentinel)  /* check loops */
				luaL_error(L, "loop or previous error loading module " LUA_QS, name);
			return 1;  /* package is already loaded */
		}
		/* else must load it; iterate over available loaders */
		lua_getfield(L, LUA_ENVIRONINDEX, "loaders");
		if (!lua_istable(L, -1))
			luaL_error(L, LUA_QL("package.loaders") " must be a table");
		lua_pushliteral(L, "");  /* error message accumulator */
		for (i=1; ; i++) {
			lua_rawgeti(L, -2, i);  /* get a loader */
			if (lua_isnil(L, -1))
				luaL_error(L, "module " LUA_QS " not found:%s",
				name, lua_tostring(L, -2));
			lua_pushstring(L, name);
			lua_call(L, 1, 1);  /* call it */
			if (lua_isfunction(L, -1))  /* did it find module? */
				break;  /* module loaded successfully */
			else if (lua_isstring(L, -1))  /* loader returned error message? */
				lua_concat(L, 2);  /* accumulate it */
			else
				lua_pop(L, 1);
		}
		lua_pushlightuserdata(L, sentinel);
		lua_setfield(L, 2, name);  /* _LOADED[name] = sentinel */
		lua_pushstring(L, name);  /* pass name as argument to module */
		lua_call(L, 1, 1);  /* run loaded module */
		if (!lua_isnil(L, -1))  /* non-nil return? */
			lua_setfield(L, 2, name);  /* _LOADED[name] = returned value */
		lua_getfield(L, 2, name);
		if (lua_touserdata(L, -1) == sentinel) {   /* module did not set a value? */
			lua_pushboolean(L, 1);  /* use true as result */
			lua_pushvalue(L, -1);  /* extra copy to be returned */
			lua_setfield(L, 2, name);  /* _LOADED[name] = true */
		}
		return 1;
	}
#endif // 0

}
