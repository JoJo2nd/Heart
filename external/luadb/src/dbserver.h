/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#ifndef DBSERVER_H__
#define DBSERVER_H__

#include "ldb/types.h"

#include "luadb_rpc.pb.h"
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};
#include "enet/enet.h"

class ldbDebugServer
{
public:
    ldbDebugServer(){}
    ~ldbDebugServer() {}

    bool attach(lua_State* L, uint32);

private:
    ldbDebugServer(const ldbDebugServer& rhs);
    ldbDebugServer& operator = (const ldbDebugServer& rhs);

};

#endif // DBSERVER_H__