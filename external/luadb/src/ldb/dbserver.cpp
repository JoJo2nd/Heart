/********************************************************************

    filename:   dbserver.cpp  
    
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

#include "ldb/dbserver.h"
#include "luadb_rpc.pb.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "osfuncs.h"
#include <boost/filesystem.hpp>

static int ldb_api ldb_print(lua_State* L) {
    int n = lua_gettop(L);  /* number of arguments */
    int i;
    lua_getglobal(L, "tostring");
    for (i=1; i<=n; i++) {
        const char *s;
        size_t l;
        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, i);   /* value to print */
        lua_call(L, 1, 1);
        s = lua_tolstring(L, -1, &l);  /* get result */
        if (s == NULL)
            return luaL_error(L,
            LUA_QL("tostring") " must return a string to " LUA_QL("print"));
        if (i>1) {
            ldbDebugServer::get()->debugOutput(" ", 1);
        }
        ldbDebugServer::get()->debugOutput(s, l);
        lua_pop(L, 1);  /* pop result */
    }
    ldbDebugServer::get()->debugOutput("\n", 1);
    return 0;
}

// pushes on to the stack value "name" if found & returns 1, else returns 0
static int ldb_getVariableValue(lua_State* L, lua_Debug* ar, const ldb_char* name, ldb_int level) {
    if (!lua_getstack(L, level, ar)) {
        //invalid stack level
        return 0;
    }
    
    const ldb_char* varname = nullptr;
    for (ldb_int i = 1; varname = lua_getlocal(L, ar, i); ++i) {
        if (strcmp(name, varname) == 0) {
            //found, leave on the stack and return
            return 1;
        }
        //not what we wanted, so remove
        lua_pop(L, 1);
    }

    // check up-values
    lua_getinfo(L, "Sf", ar);
    for (ldb_int i=1; varname=lua_getupvalue(L, -1, i); ++i) {
        if (strcmp(name, varname) == 0) {
            //found, leave on the stack and return
            return 1;
        }
        //not what we wanted, so remove
        lua_pop(L, 1);
    }

    // check "_ENV"
    if ((*ar->what == 'C' && strcmp("_ENV", name) == 0) || !ldb_getVariableValue(L, ar, "_ENV", level)) {
        return 0; //unable to find "_ENV"
    }
    lua_getfield(L, -1, name);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return 0;
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldbDebugServer* ldbDebugServer::get() {
    static ldbDebugServer instance_;
    return &instance_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbDebugServer::attach(lua_State* L, ldb_uint port) {
    if (lstate_) {
        return false;
    }

    //create the server first
    address_.host=ENET_HOST_ANY;
    address_.port=port;
    host_=enet_host_create(&address_, 1/*peers*/, 1/*channels*/, 0/*in b/w*/, 0/*out b/w*/);
    if (!host_) {
        return false;
    }
    enet_address_get_host(&address_, ipstring_, sizeof(ipstring_));
    enet_address_set_host(&thisaddress_, ipstring_);
    enet_address_get_host_ip(&thisaddress_, ipstring_, sizeof(ipstring_));

    printf("Creating host 0x%p @ %s : %u\n", host_, ipstring_, address_.port);

    lstate_ = L;

    debuggerState_.doneFReturn_ = false;
    debuggerState_.lastFile_ = "";
    debuggerState_.lastLine_ = 0;
    debuggerState_.stackDepth_ = 0;
    debuggerState_.state_ = eDebugState_Trapped;

    lua_getglobal(L, "print");
    lua_pushstring(L, "ldbDebugServer- server created, port");
    lua_pushinteger(L, port);
    lua_pcall(L, 2, 0, 0);

    lua_pushcclosure(L, ldb_print, 0);
    lua_setglobal(L, "print");
    lua_sethook(lstate_, luaHook,  LUA_MASKCALL|LUA_MASKRET|LUA_MASKLINE, 0);

    killThread_ = false;
    serviceThread_ = new ldb_thread([=]{
        networkService(host_, &eventMtx_, &packetMtx_, &postedEvents_, &packetsToSend_, &killThread_);
    });

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbDebugServer::dettach() {
    if (!serviceThread_ || !lstate_) {
        return false;
    }

    debugOutputf("ldbDebugServer- server shutdown");
    lua_sethook(lstate_, nullptr, 0, 0);

    if (host_ && serviceThread_) {
        luadb::proto::RPCCall rpccall;
        rpccall.set_cmd(luadb::proto::eDebuggerCmd_DebuggerExit);
        broadcastPacket(rpccall);

        while (!peerArray_.empty()) {
            networkTick(5000, nullptr, nullptr);
        }

        if (serviceThread_->joinable()) {
            killThread_ = true;
            serviceThread_->join();
        }
        delete serviceThread_;
        serviceThread_=nullptr;
    }

    if (host_) {
        enet_host_destroy(host_);
        host_ = nullptr;
        peerArray_.clear();
    }

    lstate_=nullptr;

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbDebugServer::luaHook(lua_State* L, lua_Debug* ar) {
    ldb_bool run=false;
    ldb_int timelimit = 0;
    lua_getinfo(L, "nSltu", ar);
    ldb_int stackdepth=0;
    for (; lua_getstack(L, stackdepth, ar); ++stackdepth) {}
    do {
        // get if we should run, commands in handlePacket my run to change
        get()->debuggerTick(L, stackdepth, ar);
        get()->networkTick(timelimit, L, ar);
        timelimit=500;
    } while(get()->isTrapped());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbDebugServer::debuggerTick(lua_State* L, ldb_int stackdepth, lua_Debug* ar) {
    //check if we have hit a breakpoint
    DebugState prevstate = debuggerState_.state_;
    ldb_string sourcefile;
    if (*ar->source == '@') {
        sourcefile = ar->source+1;
        searchForPath(sourcefile.c_str());
    }
    auto bpitr = breakpointFilenames_.find(getPossibleFullPath(sourcefile));
    if (bpitr != breakpointFilenames_.end()) {
        if (ar->currentline < bpitr->second.size() && bpitr->second[ar->currentline]) {
            debuggerState_.state_ = eDebugState_Trapped;
            sendDebuggerState();
        }
    }

    if (debuggerState_.state_ == eDebugState_StepOver) {
        if (stackdepth <= debuggerState_.stackDepth_ && debuggerState_.lastLine_ != ar->currentline) {
            debuggerState_.state_ = eDebugState_Trapped;
            debuggerState_.stackDepth_ = stackdepth;
            sendDebuggerState();
        }
    } else if (debuggerState_.state_ == eDebugState_StepIn) {
        debuggerState_.stackDepth_ = stackdepth;
        debuggerState_.state_ = eDebugState_Trapped;
        sendDebuggerState();
    } else if (debuggerState_.state_ == eDebugState_StepOut) {
        if (stackdepth < debuggerState_.stackDepth_) {
            debuggerState_.stackDepth_ = stackdepth;
            debuggerState_.state_ = eDebugState_Trapped;
            sendDebuggerState();
        }
    }

    if (debuggerState_.lastFile_ != sourcefile || debuggerState_.lastLine_ != ar->currentline) {
        debuggerState_.lastFile_ = sourcefile;
        debuggerState_.lastLine_ = ar->currentline;
        if (debuggerState_.state_ == eDebugState_Trapped && !peerArray_.empty()) {
            luadb::proto::RPCCall rpccall;
            rpccall.set_cmd(luadb::proto::eDebuggerCmd_LineInfo);
            rpccall.mutable_linemessagedata()->set_absolutefilepath(debuggerState_.lastFile_);
            rpccall.mutable_linemessagedata()->set_linenumber(debuggerState_.lastLine_);
            broadcastPacket(rpccall);
        }
    }

    return debuggerState_.state_ != eDebugState_Trapped;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbDebugServer::handlePacket(ENetPeer* peer, ENetPacket* packet, lua_State* L, lua_Debug* ar) {
    using namespace luadb;

    ldb_int stackdepth=0;
    for (; lua_getstack(L, stackdepth, ar); ++stackdepth) {}
    proto::RPCCall rpccall;
    rpccall.ParseFromArray(packet->data, packet->dataLength);

    switch(rpccall.cmd()) {
    case proto::eDebuggerCmd_Run: {
        debuggerState_.run();
        sendDebuggerState();
        clearLocalsRegistery(L);
    } break;
    case proto::eDebuggerCmd_Trap: {
        debuggerState_.trap();
        sendDebuggerState();
        debuggerTick(L, stackdepth, ar);
        clearLocalsRegistery(L);
    } break;
    case proto::eDebuggerCmd_Step: {
        debuggerState_.step(stackdepth);
        sendDebuggerState();
        clearLocalsRegistery(L);
    } break;
    case proto::eDebuggerCmd_StepIn: {
        debuggerState_.stepIn();
        sendDebuggerState();
        clearLocalsRegistery(L);
    } break;
    case proto::eDebuggerCmd_StepOut: {
        debuggerState_.stepOut(stackdepth);
        sendDebuggerState();
        clearLocalsRegistery(L);
    } break;
    case proto::eDebuggerCmd_RemoveBreakpoint:
    case proto::eDebuggerCmd_SetBreakpoint: {
        if (rpccall.has_breakpointdata()) {
            ldb_bool apply = rpccall.cmd() == proto::eDebuggerCmd_SetBreakpoint;
            const ldb_string& filepath = rpccall.breakpointdata().absolutefilepath();
            if (breakpointFilenames_[filepath].size() <= rpccall.breakpointdata().linenumber()) {
                breakpointFilenames_[filepath].resize(rpccall.breakpointdata().linenumber()+1, false);
            }
            breakpointFilenames_[filepath][rpccall.breakpointdata().linenumber()] = apply;
            //report that it was set ok
            rpccall.mutable_breakpointdata()->set_breakpointset(true);
        }
        sendPacket(peer, rpccall);
    } break;
    case proto::eDebuggerCmd_Backtrace: {
        for (ldb_int sd=0; lua_getstack(L, sd, ar); ++sd) {
            lua_getinfo(L, "nSltu", ar);
            auto bt = rpccall.mutable_backtracedata()->add_stackbacktrace();
            if (ar->name) {
                bt->set_name(ar->name);
            }
            if (*ar->source == '@') {
                bt->set_filename(ar->source+1);
            }
            if (strcmp(ar->what, "C") == 0) {
                bt->set_iscfunction(true);
            }
            if (strcmp(ar->what, "main") == 0) {
                bt->set_ismain(true);
            }
            bt->set_namewhat(ar->namewhat);
            if (ar->short_src) {
                bt->set_printfilename(ar->short_src);
            }
            bt->set_linenumber(ar->currentline);
        }
        //send back
        sendPacket(peer, rpccall);
    } break;
    case proto::eDebuggerCmd_GetLocals: {
        if (rpccall.has_localsstacklevel()) {
            clearLocalsRegistery(L);
            if (lua_getstack(L, rpccall.localsstacklevel(), ar)) {
                // valid stack level, 
                const ldb_char* varname = nullptr;
                for (ldb_int i = 1; varname = lua_getlocal(L, ar, i); ++i) {
                    //values is at top of stack
                    proto::RPCVar* rpcvar = rpccall.add_locals();
                    initialiseRPCVar(L, rpcvar, varname, &localsRegFields_);
                }
            }
        }
        //send back
        sendPacket(peer, rpccall);
    } break;
    case proto::eDebuggerCmd_AddSearchPath: {
        if (rpccall.has_searchpath()) {
            addSearchPath(rpccall.searchpath().c_str());
        }
    } break;
    case proto::eDebuggerCmd_GetVarMoreInfo: {
        if (rpccall.has_varmoreinfoid()) {
            ldb_bool islocalvar=false;
            ldb_int expectedrefindex = rpccall.varmoreinfoid();
            ldb_int refindex = LUA_REFNIL;
            for (ldb_size_t i=0, n=localsRegFields_.size(); i<n; ++i) {
                if (localsRegFields_[i] == expectedrefindex) {
                    refindex = localsRegFields_[i];
                    islocalvar = true;
                }
            }

            if (refindex == LUA_REFNIL) {
                break;
            }

            ldb_intArray tmparray;
            const ldb_char* varname = "";
            //values is at top of stack
            proto::RPCVar* rpcvar = rpccall.mutable_moreinfovar();
            lua_rawgeti(L, LUA_REGISTRYINDEX, refindex);
            initialiseRPCVar(L, rpcvar, varname, islocalvar ? &localsRegFields_ : &tmparray);
        }
        //send back
        sendPacket(peer, rpccall);
    } break;
    case proto::eDebuggerCmd_GetWatch: {
        if (rpccall.has_watchvarstring() && rpccall.has_localsstacklevel()) {
            if (ldb_getVariableValue(L, ar, rpccall.watchvarstring().c_str(), rpccall.localsstacklevel())) {
                proto::RPCVar* rpcvar = rpccall.mutable_watchvar();
                initialiseRPCVar(L, rpcvar, rpccall.watchvarstring().c_str(), &localsRegFields_);
            }
        }
        sendPacket(peer, rpccall);
    } break;
    case proto::eDebuggerCmd_Noop:
    default: break;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbDebugServer::networkTick(ldb_int timelimit, lua_State* L, lua_Debug* ar) {
    if (!host_) {
        return;
    }

    ldb_mutexsentry sentry(eventMtx_);
    for (auto i=postedEvents_.begin(), n=postedEvents_.end(); i!=n; ++i) {
        auto& deferredevent = *i;
        switch(deferredevent.type) {
        case ENET_EVENT_TYPE_CONNECT: {
            peerArray_.push_back(deferredevent .peer);
        } break;
        case ENET_EVENT_TYPE_DISCONNECT: {
            for (auto i=peerArray_.begin(), n=peerArray_.end(); i!=n; ++i) {
                if (*i == deferredevent .peer) {
                    peerArray_.erase(i);
                    break;
                }
            }
        } break;
        default: break;
        }
    }

    for (auto i=postedEvents_.begin(), n=postedEvents_.end(); i!=n; ++i) {
        auto& deferredevent = *i;
        switch(deferredevent.type) {
        case ENET_EVENT_TYPE_RECEIVE: {
            ldb_bool inpeerarray=true;
            for (auto i=peerArray_.begin(), n=peerArray_.end(); i!=n; ++i) {
                if (deferredevent.peer == *i) inpeerarray=true;
            }
            if (deferredevent.packet && inpeerarray && L && ar) {
                handlePacket(deferredevent.peer, deferredevent.packet, L, ar);
            }
            enet_packet_destroy(deferredevent.packet);
        } break;
        default: break;
        }
    }

    postedEvents_.clear();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbDebugServer::debugOutput(const char* str, ldb_int len) {
    ldb_mutexsentry sentry(eventMtx_);
    if (!host_) {
        return;
    }
    luadb::proto::RPCCall rpccall;
    rpccall.set_cmd(luadb::proto::eDebuggerCmd_DebugConsole);
    rpccall.set_debugconsolemessage(str, len);
    broadcastPacket(rpccall);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbDebugServer::debugOutputf(const char* str, ...) {
    va_list marker;
    va_start(marker, str);

    ldb_char buffer[ 4*1024 ];
    ldb_uint len = vsprintf_s( buffer, sizeof(buffer), str, marker);
    debugOutput(buffer, strlen(buffer));
    va_end(marker);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbDebugServer::networkService(ENetHost* host, 
    ldb_mutex* receivemtx, 
    ldb_mutex* sendmtx, 
    ldb_postedEvents* postedEvents, 
    ldb_packetArray* packetstosend, 
    volatile ldb_bool* killSignal) {

    while (!(*killSignal)) {
        if (host) {
            ENetEvent enetevent;
            while (enet_host_service(host, &enetevent, 10) > 0) {
                switch(enetevent.type) {
                case ENET_EVENT_TYPE_NONE: break;
                case ENET_EVENT_TYPE_DISCONNECT:
                case ENET_EVENT_TYPE_RECEIVE:
                case ENET_EVENT_TYPE_CONNECT: {
                    ldb_mutexsentry sentry(*receivemtx);
                    postedEvents->push_back(enetevent);
                } break;
                default: break;
                }
            }
            {
                ldb_mutexsentry sentry(*sendmtx);
                for (auto i=packetstosend->cbegin(), n=packetstosend->cend(); i!=n; ++i) {
                    if (!i->peer_) {
                        enet_host_broadcast(host, 0, i->packet_);
                    } else {
                        enet_peer_send(i->peer_, 0, i->packet_);
                    }
                }
                packetstosend->clear();
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbDebugServer::sendDebuggerState() {
    ldb_mutexsentry sentry(eventMtx_);
    luadb::proto::RPCCall rpccall;
    rpccall.set_cmd(luadb::proto::eDebuggerCmd_RunningState);
    rpccall.set_runningstatedata(!isTrapped());
    broadcastPacket(rpccall);

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbDebugServer::addSearchPath(const ldb_char* str) {
    fileSearchPaths_.push_back(ldb_string(str));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbDebugServer::searchForPath(const ldb_char* inpath) {
    using namespace boost::filesystem;
    boost::system::error_code ec;
    ldb_string name(inpath);

    if (filenameAlias_.find(name) != filenameAlias_.end()) {
        return;
    }

    for (auto i=fileSearchPaths_.begin(), n=fileSearchPaths_.end(); i!=n; ++i) {
        auto canon = canonical(path(name), path(*i), ec); //canonical will fail if path doesn't exist
        if (!ec) {
            filenameAlias_[name] = canon.generic_string();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const ldb_string& ldbDebugServer::getPossibleFullPath(const ldb_string& sourcename) {
    auto itr=filenameAlias_.find(sourcename);
    if ( itr == filenameAlias_.end()) {
        return sourcename;
    }
    return itr->second;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbDebugServer::clearLocalsRegistery(lua_State* L) {
    for (auto i=localsRegFields_.begin(), n=localsRegFields_.end(); i!=n; ++i) {
        luaL_unref(L, LUA_REGISTRYINDEX, *i);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbDebugServer::broadcastPacket(const google::protobuf::MessageLite& msg) {
    auto packet = enet_packet_create(nullptr, msg.ByteSize(), ENET_PACKET_FLAG_RELIABLE);
    msg.SerializeToArray(packet->data, packet->dataLength);
    ldb_mutexsentry sentry(packetMtx_);
    packetsToSend_.emplace_back(nullptr, packet);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbDebugServer::sendPacket(ENetPeer* peer, const google::protobuf::MessageLite& msg) {
    auto packet = enet_packet_create(nullptr, msg.ByteSize(), ENET_PACKET_FLAG_RELIABLE);
    msg.SerializeToArray(packet->data, packet->dataLength);
    ldb_mutexsentry sentry(packetMtx_);
    packetsToSend_.emplace_back(peer, packet);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbDebugServer::initialiseRPCVar(lua_State* L, luadb::proto::RPCVar* rpcvar, const ldb_char* varname, ldb_intArray* refArray) {
    ldb_int stackidx = -1;
    ldb_int luatype = lua_type(L, stackidx);
    rpcvar->set_type(luatype);
    rpcvar->set_name(varname);
    switch (luatype) {
    case LUA_TNUMBER: {
        lua_Number num = lua_tonumber(L, stackidx);
        lua_pushvalue(L, stackidx);
        rpcvar->set_printableval(lua_tostring(L, -1));
        lua_pop(L, 1);
    } break;
    case LUA_TBOOLEAN: {
        rpcvar->set_printableval(lua_toboolean(L, stackidx) ? "true" : "false");
    } break;
    case LUA_TSTRING: {
        rpcvar->set_printableval(lua_tostring(L, stackidx));
    } break;
    case LUA_TTABLE: {
        ldb_int tableidx=0;
        ldb_int len=lua_rawlen(L, stackidx);
        /* table is in the stack at index 't' */
        lua_pushnil(L);  /* first key */
        while (lua_next(L,stackidx-1) != 0) {
            /* uses 'key' (at index -2) and 'value' (at index -1) */
            luadb::proto::RPCField* field = rpcvar->add_fields();
            field->set_type(lua_type(L, -1));
            field->set_printablekey(luaL_tolstring(L, -2, nullptr));
            lua_pop(L, 1);// pop the value from luaL_tolstring
            ldb_int fieldindex = LUA_REFNIL;
            if (lua_type(L, -1) == LUA_TTABLE) {
                lua_pushvalue(L, -1); //copy value to go into the registry
                fieldindex = luaL_ref(L, LUA_REGISTRYINDEX); // copy the value to registry for quick look up later
                refArray->push_back(fieldindex);
            }
            field->set_printableval(luaL_tolstring(L, -1, nullptr));
            lua_pop(L, 1);// pop the value from luaL_tolstring
            //removes 'value'; keeps 'key' for next iteration */
            lua_pop(L, 1);
            field->set_fieldindex(fieldindex);
        }
    } break;
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
    case LUA_TLIGHTUSERDATA:
    case LUA_TUSERDATA: {
        rpcvar->set_printableval(luaL_tolstring(L, stackidx, nullptr));
        lua_pop(L, 1);// pop the value from luaL_tolstring
    } break;
    default: break; // unhandled types
    }

    lua_pop(L, 1);
}
