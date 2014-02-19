/********************************************************************

    filename:   dbclient.h  
    
    Copyright (c) 22:12:2013 James Moran
    
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

#pragma once

#ifndef DBCLIENT_H__
#define DBCLIENT_H__

#include "types.h"
#include "enet/enet.h"
#include <vector>

namespace luadb {
namespace proto {
    class RPCCall;
}
}

enum NameType {
    eNameType_Global,
    eNameType_Local,
    eNameType_Method,
    eNameType_Field,
    eNameType_UpValue,
    eNameType_None,
};

enum FunctionType {
    eFunctionType_C,
    eFunctionType_Lua,
    eFunctionType_main,
};

enum LuaType {
    eLuaType_None           = -1,   //LUA_TNONE		(-1)
    eLuaType_Nil            = 0,    //LUA_TNIL		0
    eLuaType_Boolean        = 1,    //LUA_TBOOLEAN		1
    eLuaType_LightUserData  = 2,    //LUA_TLIGHTUSERDATA	2
    eLuaType_Number         = 3,    //LUA_TNUMBER		3
    eLuaType_String         = 4,    //LUA_TSTRING		4
    eLuaType_Table          = 5,    //LUA_TTABLE		5
    eLuaType_Function       = 6,    //LUA_TFUNCTION		6
    eLuaType_UserData       = 7,    //LUA_TUSERDATA		7
    eLuaType_Thread         = 8,    //LUA_TTHREAD		8
};


struct ldbStackLevel {
    ldb_string      source_;
    ldb_string      shortSrc_;
    ldb_string      name_;
    NameType        nameType_;
    ldb_int         currentLine_;
    FunctionType    funcType_;
};

struct ldbVarField {
    LuaType     type_;
    ldb_uint    entryIndex_;
    ldb_string  printableKey_;
    ldb_string  printableValue_;
};

struct ldbVarDesc {
    LuaType                     type_;
    ldb_string                  name_;
    ldb_string                  printableValue_;
    std::vector< ldbVarField >  fields_;
};

typedef std::function<void(const ldb_char*)> ldb_debugMsgDelegate;

class ldbClient 
{
public:
    ldbClient()
        : client_(nullptr)
        , peer_(nullptr)
        , currentLine_(0)
        , rpcOpNumber_(0)
        , rpcOpComplete_(false)
        , debuggerRunning_(false)
        , debuggerConnected_(false)
        , debuggerExit_(false)
    {}
    ~ldbClient()
    {
        disconnect();
    }

    struct ldbTickResults { // stuck to pass in/out result from tick
        ldbVarDesc*     outResult;
    };

    ldb_bool                connect(ldb_string& address,ldb_int port);
    void                    tickSingleEvent(ldb_uint timeout, ldbTickResults* outresults);
    void                    tickMultipleEvents(ldb_uint timeout, ldbTickResults* outresults);
    ldb_bool                isRunning() const { return debuggerRunning_; }
    ldb_bool                isConnected() const { return debuggerConnected_; }
    ldb_bool                debuggerExit() const { return debuggerExit_; }
    const ldb_string&       getCurrentFile() const { return currentFile_; }
    ldb_uint32              getCurrentLine() const { return currentLine_; }
    ldb_bool                run();
    ldb_bool                step();
    ldb_bool                stepIn();
    ldb_bool                stepOut();
    ldb_bool                trap();
    ldb_debugMsgDelegate    setMessageDelegate(const ldb_debugMsgDelegate& val);
    ldb_bool                setBreakpoint(ldb_string& filepath, ldb_uint line, ldb_bool set);
    ldb_bool                getBacktrace();
    ldb_uint                getBacktraceSize() const { return stack_.size(); }
    const ldbStackLevel*    getBacktraceLevel(ldb_uint lvl) const { return &stack_[lvl]; }
    void                    addSearchPath(const ldb_char* path);
    ldb_bool                getLocals(ldb_int stacklevel, std::vector< ldbVarDesc >* outlocals);
    ldb_bool                getMoreInfo(ldb_int varEntryID, ldbVarDesc* outvar);
    ldb_bool                getWatch(const ldb_char* varname, ldb_int stacklevel, ldbVarDesc* outvar);
    void                    disconnect();

private:

    typedef std::vector< ldbStackLevel > ldb_stackInfo;

    ldbClient(const ldbClient& rhs) {}
    ldbClient& operator = (const ldbClient& rhs) {}
    void        handlePacket(ENetHost* client, ENetPeer * peer, ENetPacket * packet, ldbTickResults* outresults);
    void        sendPacket(ENetPeer* peer, const luadb::proto::RPCCall& packetdata);

    ldb_debugMsgDelegate        debugMsgDelegate_;
    ENetHost*                   client_;
    ENetPeer*                   peer_;
    ldb_stackInfo               stack_;
    ldb_int                     localsStackLevel_;
    std::vector< ldbVarDesc >   locals_;
    ldb_string                  currentFile_;
    ldb_uint                    currentLine_;
    ldb_uint32                  rpcOpNumber_;
    ldb_bool                    rpcOpComplete_;
    ldb_bool                    debuggerRunning_;
    ldb_bool                    debuggerConnected_;
    ldb_bool                    debuggerExit_;
};

#endif // DBCLIENT_H__