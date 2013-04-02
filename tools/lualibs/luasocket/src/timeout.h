#ifndef TM_H
#define TM_H
/*=========================================================================*\
* Timeout management functions
* LuaSocket toolkit
*
* RCS ID: $Id: timeout.h,v 1.11 2004/07/15 06:11:53 diego Exp $
\*=========================================================================*/
#include <lua.h>

/* timeout control structure */
typedef struct t_tm_ {
    double total;          /* total number of miliseconds for operation */
    double block;          /* maximum time for blocking calls */
    double start;          /* time of start of operation */
} t_tm;
typedef t_tm *p_tm;

int tm_open(lua_State *L);
void tm_init(p_tm tm, double block, double total);
double tm_get(p_tm tm);
double tm_getretry(p_tm tm);
p_tm tm_markstart(p_tm tm);
double tm_getstart(p_tm tm);
double tm_gettime(void);
int tm_meth_settimeout(lua_State *L, p_tm tm);

#define tm_iszero(tm)   ((tm)->block == 0.0)

#endif /* TM_H */
