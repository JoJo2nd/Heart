/*=========================================================================*\
* Timeout management functions
* LuaSocket toolkit
*
* RCS ID: $Id: timeout.c,v 1.25 2004/07/16 06:48:48 diego Exp $
\*=========================================================================*/
#include <stdio.h>

#include <lua.h>
#include <lauxlib.h>

#include "luasocket.h"
#include "auxiliar.h"
#include "timeout.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#include <sys/time.h>
#endif

/* min and max macros */
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? x : y)
#endif
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? x : y)
#endif

/*=========================================================================*\
* Internal function prototypes
\*=========================================================================*/
static int tm_lua_gettime(lua_State *L);
static int tm_lua_sleep(lua_State *L);

static luaL_Reg func[] = {
    { "gettime", tm_lua_gettime },
    { "sleep", tm_lua_sleep },
    { NULL, NULL }
};

/*=========================================================================*\
* Exported functions.
\*=========================================================================*/
/*-------------------------------------------------------------------------*\
* Initialize structure
\*-------------------------------------------------------------------------*/
void tm_init(p_tm tm, double block, double total) {
    tm->block = block;
    tm->total = total;
}

/*-------------------------------------------------------------------------*\
* Determines how much time we have left for the next system call,
* if the previous call was successful 
* Input
*   tm: timeout control structure
* Returns
*   the number of ms left or -1 if there is no time limit
\*-------------------------------------------------------------------------*/
double tm_get(p_tm tm) {
    if (tm->block < 0.0 && tm->total < 0.0) {
        return -1;
    } else if (tm->block < 0.0) {
        double t = tm->total - tm_gettime() + tm->start;
        return MAX(t, 0.0);
    } else if (tm->total < 0.0) {
        return tm->block;
    } else {
        double t = tm->total - tm_gettime() + tm->start;
        return MIN(tm->block, MAX(t, 0.0));
    }
}

/*-------------------------------------------------------------------------*\
* Returns time since start of operation
* Input
*   tm: timeout control structure
* Returns
*   start field of structure
\*-------------------------------------------------------------------------*/
double tm_getstart(p_tm tm) {
    return tm->start;
}

/*-------------------------------------------------------------------------*\
* Determines how much time we have left for the next system call,
* if the previous call was a failure
* Input
*   tm: timeout control structure
* Returns
*   the number of ms left or -1 if there is no time limit
\*-------------------------------------------------------------------------*/
double tm_getretry(p_tm tm) {
    if (tm->block < 0.0 && tm->total < 0.0) {
        return -1;
    } else if (tm->block < 0.0) {
        double t = tm->total - tm_gettime() + tm->start;
        return MAX(t, 0.0);
    } else if (tm->total < 0.0) {
        double t = tm->block - tm_gettime() + tm->start;
        return MAX(t, 0.0);
    } else {
        double t = tm->total - tm_gettime() + tm->start;
        return MIN(tm->block, MAX(t, 0.0));
    }
}

/*-------------------------------------------------------------------------*\
* Marks the operation start time in structure 
* Input
*   tm: timeout control structure
\*-------------------------------------------------------------------------*/
p_tm tm_markstart(p_tm tm) {
    tm->start = tm_gettime();
    return tm;
}

/*-------------------------------------------------------------------------*\
* Gets time in ms, relative to system startup.
* Returns
*   time in ms.
\*-------------------------------------------------------------------------*/
#ifdef _WIN32
double tm_gettime(void) {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    return ft.dwLowDateTime/1.0e7 + ft.dwHighDateTime*(4294967296.0/1.0e7);
}
#else
double tm_gettime(void) {
    struct timeval v;
    gettimeofday(&v, (struct timezone *) NULL);
    return v.tv_sec + v.tv_usec/1.0e6;
}
#endif

/*-------------------------------------------------------------------------*\
* Initializes module
\*-------------------------------------------------------------------------*/
int tm_open(lua_State *L) {
    luaL_openlib(L, NULL, func, 0);
    return 0;
}

/*-------------------------------------------------------------------------*\
* Sets timeout values for IO operations
* Lua Input: base, time [, mode]
*   time: time out value in seconds
*   mode: "b" for block timeout, "t" for total timeout. (default: b)
\*-------------------------------------------------------------------------*/
int tm_meth_settimeout(lua_State *L, p_tm tm) {
    double t = luaL_optnumber(L, 2, -1);
    const char *mode = luaL_optstring(L, 3, "b");
    switch (*mode) {
        case 'b':
            tm->block = t; 
            break;
        case 'r': case 't':
            tm->total = t;
            break;
        default:
            luaL_argcheck(L, 0, 3, "invalid timeout mode");
            break;
    }
    lua_pushnumber(L, 1);
    return 1;
}

/*=========================================================================*\
* Test support functions
\*=========================================================================*/
/*-------------------------------------------------------------------------*\
* Returns the time the system has been up, in secconds.
\*-------------------------------------------------------------------------*/
static int tm_lua_gettime(lua_State *L)
{
    lua_pushnumber(L, tm_gettime());
    return 1;
}

/*-------------------------------------------------------------------------*\
* Sleep for n seconds.
\*-------------------------------------------------------------------------*/
int tm_lua_sleep(lua_State *L)
{
    double n = luaL_checknumber(L, 1);
#ifdef _WIN32
    Sleep((int)(n*1000));
#else
    struct timespec t, r;
    t.tv_sec = (int) n;
    n -= t.tv_sec;
    t.tv_nsec = (int) (n * 1000000000);
    if (t.tv_nsec >= 1000000000) t.tv_nsec = 999999999;
    nanosleep(&t, &r);
#endif
    return 0;
}
