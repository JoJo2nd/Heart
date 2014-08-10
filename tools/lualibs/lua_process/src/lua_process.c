/********************************************************************

    filename:   lua_process.c  
    
    Copyright (c) 27:2:2014 James Moran
    
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
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lua_process.h"

#if defined (PLATFORM_LINUX)

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct lua_process_def {
    pid_t               pid_;
    size_t              cmdlineLen_;
    char*               cmdline_;
} lua_process_def_t;

int lua_process_api lua_process_exec(lua_State* L) {
    static const int args_limit = 1024;
    size_t cmdlinelen;
    const char* cmdline = luaL_checklstring(L, 1, &cmdlinelen);
    lua_process_def_t* udata= NULL;
    char* args[args_limit];
    char* cmdline_ptr;
    int i=0, prev_space=1, quote_count=0;

    udata = lua_newuserdata(L, sizeof(lua_process_def_t));
    luaL_getmetatable(L, "_lua_process_lib.proc");
    lua_setmetatable(L, -2);
    udata->cmdline_ = malloc(cmdlinelen+1);
    memcpy(udata->cmdline_, cmdline, cmdlinelen);
    udata->cmdline_[cmdlinelen] = 0;
    udata->cmdlineLen_ = cmdlinelen;

    cmdline_ptr = udata->cmdline_;
    prev_space = 1;
    do {
        if (!isspace(*cmdline_ptr)) {
            if (prev_space) {
                if (*cmdline_ptr == '"') {
                    ++cmdline_ptr;
                }
                args[i++] = cmdline_ptr;
                prev_space = 0;
            }else if (*cmdline_ptr == '"') {
                *cmdline_ptr = 0x0;
                prev_space = 1;
            }
        } else {
          prev_space = 1;
          *cmdline_ptr = 0x0;
        }
        ++cmdline_ptr;
    } while (i < (args_limit-1) && *cmdline_ptr);
    args[i] = 0;

    udata->pid_ = fork();

    if (udata->pid_ == 0) {
        /** child runs the process and exits */
        execv(args[0], args);
        exit(0);
    }

    /** parent returns */
    return 1;
}

int lua_process_api lua_process_sleep(lua_State* L) {
    int ms = luaL_checkint(L, 1);
    sleep(ms/1000);
    return 0;
}

int lua_process_api lua_process_gc(lua_State* L) {
    lua_process_def_t* udata = luaL_checkudata(L, 1, "_lua_process_lib.proc");

    if (udata->cmdline_) {
        free(udata->cmdline_);
        udata->cmdline_ = NULL;
    }

    return 0;
}

int lua_process_api lua_process_wait(lua_State* L) {
    lua_process_def_t* udata = luaL_checkudata(L, 1, "_lua_process_lib.proc");
    int wait = luaL_checkint(L, 2);
    int status;
    int ret = waitpid(udata->pid_, &status, wait == 0 ? WNOHANG : 0);

    if (ret > 0) {
        lua_pushinteger(L, status);
    } else {
        lua_pushnil(L);
    }

    return 1;
}

#elif defined (PLATFORM_WINDOWS)

#include <windows.h>

typedef struct lua_process_def {
    STARTUPINFO         startupInfo_;
    PROCESS_INFORMATION procInfo_;
    char*               cmdline_;
} lua_process_def_t;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int lua_process_api lua_process_exec(lua_State* L) {
    size_t cmdlinelen;
    const char* cmdline = luaL_checklstring(L, 1, &cmdlinelen);
    lua_process_def_t* udata= NULL;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    udata = lua_newuserdata(L, sizeof(lua_process_def_t));
    luaL_getmetatable(L, "_lua_process_lib.proc");
    lua_setmetatable(L, -2);
    udata->cmdline_ = malloc(cmdlinelen+1);
    memcpy(udata->cmdline_, cmdline, cmdlinelen);
    udata->cmdline_[cmdlinelen] = 0;

    // Start the child process. 
    if( !CreateProcess( NULL,   // No module name (use command line)
        udata->cmdline_,        // Command line
        NULL,               // Process handle not inheritable
        NULL,               // Thread handle not inheritable
        FALSE,              // Set handle inheritance to FALSE
        CREATE_NO_WINDOW,   // No creation flags
        NULL,               // Use parent's environment block
        NULL,               // Use parent's starting directory 
        &si,                // Pointer to STARTUPINFO structure
        &pi )               // Pointer to PROCESS_INFORMATION structure
        )
    {
        free(udata->cmdline_);
        udata->cmdline_ = NULL;
        return luaL_error(L, "CreateProcess failed (%d)\n", GetLastError() );
    }

    udata->procInfo_ = pi;
    udata->startupInfo_ = si;

    return 1;
}

int lua_process_api lua_process_sleep(lua_State* L) {
    int ms = luaL_checkint(L, 1);
    Sleep(ms);
    return 0;
}

int lua_process_api lua_process_gc(lua_State* L) {
    lua_process_def_t* udata = luaL_checkudata(L, 1, "_lua_process_lib.proc");

    if (udata->cmdline_) {
        // Close process and thread handles. 
        CloseHandle( udata->procInfo_.hProcess );
        CloseHandle( udata->procInfo_.hThread );
        free(udata->cmdline_);
        udata->cmdline_ = NULL;
    }

    return 0;
}

int lua_process_api lua_process_wait(lua_State* L) {
    lua_process_def_t* udata = luaL_checkudata(L, 1, "_lua_process_lib.proc");
    int wait = luaL_checkint(L, 2);
    DWORD exit_code = -1;

    // Wait until child process exits.
    if (WaitForSingleObject( udata->procInfo_.hProcess, wait != 0 ? INFINITE : 0) == WAIT_OBJECT_0) {
        // Get exit code
        GetExitCodeProcess( udata->procInfo_.hProcess, &exit_code );
        lua_pushinteger(L, exit_code);
        return 1;
    }

    lua_pushnil(L);
    return 1;
}
#endif

static const luaL_Reg lua_process_methods[] = {
    {"__gc", lua_process_gc},
    {"wait", lua_process_wait},
    {NULL, NULL}
};


static const luaL_Reg lua_process_lib[] = {
    {"exec", lua_process_exec},
    {"sleep", lua_process_sleep},
    {NULL, NULL}
};

lua_process_dll_export int lua_process_api luaopen_lua_process(lua_State* L) {
    luaL_newmetatable(L, "_lua_process_lib.proc");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lua_process_methods, 0);

    luaL_newlib(L, lua_process_lib);
    return 1;
}
