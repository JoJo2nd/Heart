/********************************************************************

    filename:   dbserver.h  
    
    Copyright (c) 16:12:2013 James Moran
    
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

#ifndef DBSERVER_H__
#define DBSERVER_H__

#include "types.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};
#include "enet/enet.h"
#include <vector>
#include <unordered_map>
#include <thread>

namespace google
{
namespace protobuf
{
    class MessageLite;
}
}

namespace luadb
{
namespace proto
{
    class RPCVar;
}
}

class ldbDebugServer
{
public:

    static ldbDebugServer* get();
    ldb_bool attach(lua_State* L, ldb_uint port);
    void     networkTick(ldb_int timelimit, lua_State* L, lua_Debug* ar);
    void     debugOutput(const char* str, ldb_int len);
    void     debugOutputf(const char* str, ...);
    void     addSearchPath(const ldb_char* str);
    ldb_bool dettach();

private:
    enum DebugState {
        eDebugState_Running,
        eDebugState_Trapped,
        eDebugState_StepOver,
        eDebugState_StepIn,
        eDebugState_StepOut,
    };

    struct Packet 
    {
        Packet(ENetPeer* peer, ENetPacket* packet) 
            : peer_(peer), packet_(packet) {}
        ENetPeer*     peer_;
        ENetPacket*   packet_;
    };

    typedef std::vector< ldb_bool >                          ldb_bitArray;
    typedef std::vector< ENetPeer* >                         ldb_peerArray;
    typedef std::vector< ENetEvent >                         ldb_postedEvents;
    typedef std::vector< Packet >                            ldb_packetArray;
    typedef std::vector< ldb_string >                        ldb_stringArray;
    typedef std::vector< ldb_int >                           ldb_intArray;
    typedef std::unordered_map< ldb_string, ldb_bitArray >   ldb_filenameMap;
    typedef std::unordered_map< ldb_string, ldb_string >     ldb_filenameAliasMap;

    ldbDebugServer() 
        : lstate_(nullptr)
        , host_(nullptr)
    {
        // -- we make the assumption that enet is ready to use
    }
    ~ldbDebugServer() {}
    ldbDebugServer(const ldbDebugServer& rhs);
    ldbDebugServer& operator = (const ldbDebugServer& rhs);

    static void luaHook(lua_State*, lua_Debug*);
    ldb_bool    debuggerTick(lua_State*, ldb_int, lua_Debug*);

    void sendDebuggerState();

    void        handlePacket(ENetPeer*, ENetPacket*, lua_State*, lua_Debug*);
    ldb_bool    isTrapped() const { return debuggerState_.state_ == eDebugState_Trapped; }
    static void networkService(ENetHost* host, ldb_mutex* receivemtx, ldb_mutex* sendmtx, ldb_postedEvents* postedEvents, ldb_packetArray* packetstosend, volatile ldb_bool* killSignal);
    void        searchForPath(const ldb_char* name);
    const ldb_string& getPossibleFullPath(const ldb_string& sourename);
    void        clearLocalsRegistery(lua_State* L);
    void        broadcastPacket(const google::protobuf::MessageLite& msg);
    void        sendPacket(ENetPeer* peer, const google::protobuf::MessageLite& msg);
    void        initialiseRPCVar(lua_State* L, luadb::proto::RPCVar* rpcvar, const ldb_char* varname, ldb_intArray* referenceArray);

    lua_State*              lstate_;
    ENetAddress             address_;
    ENetAddress             thisaddress_;
    ENetHost*               host_;
    ldb_peerArray           peerArray_;
    ldb_char                ipstring_[64];
    ldb_thread*             serviceThread_;
    ldb_mutex               eventMtx_;
    ldb_postedEvents        postedEvents_;
    ldb_mutex               packetMtx_;
    ldb_packetArray         packetsToSend_;
    volatile ldb_bool       killThread_;
    ldb_filenameMap         breakpointFilenames_;
    ldb_stringArray         fileSearchPaths_;
    ldb_filenameAliasMap    filenameAlias_;
    ldb_intArray            localsRegFields_;

    struct DebuggerState {
        DebugState state_;
        ldb_string lastFile_;
        ldb_int64  lastLine_;
        ldb_uint64 stackDepth_;
        ldb_bool   doneFReturn_;

        void run() {
            doneFReturn_ = false;
            state_ = eDebugState_Running;
        }
        void trap() {
            doneFReturn_ = false;
            state_ = eDebugState_Trapped;
        }
        void step(ldb_uint64 stackdepth) {
            doneFReturn_ = false;
            stackDepth_ = stackdepth;
            state_ = eDebugState_StepOver;
        }
        void stepIn() {
            doneFReturn_ = false;
            state_ = eDebugState_StepIn;
        }
        void stepOut(ldb_uint64 stackdepth) {
            doneFReturn_ = false;
            stackDepth_ = stackdepth;
            state_ = eDebugState_StepOut;
        }
    } debuggerState_;
};

#endif // DBSERVER_H__