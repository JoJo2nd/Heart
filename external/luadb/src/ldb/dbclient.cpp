/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "dbclient.h"
#include "osfuncs.h"
#include "luadb_rpc.pb.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void initVarDesc(ldbVarDesc* var, const luadb::proto::RPCVar& rpcvar) {
    using namespace luadb;

    var->name_=rpcvar.name();
    var->type_=(LuaType)rpcvar.type();
    if (rpcvar.has_printableval()) {
        var->printableValue_=rpcvar.printableval();
    }
    for(ldb_uint ifield=0, nfield=rpcvar.fields_size(); ifield<nfield; ++ifield) {
        const proto::RPCField& rpcfield=rpcvar.fields(ifield);
        var->fields_.emplace_back();
        auto field=var->fields_.rbegin();
        field->entryIndex_=rpcfield.fieldindex();
        field->printableKey_=rpcfield.printablekey();
        field->type_=(LuaType)rpcfield.type();
        if (rpcfield.has_printableval()) {
            field->printableValue_=rpcfield.printableval();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbClient::connect(const ldb_string& addressstr, ldb_int port) {
    ENetAddress enetaddress;
    ENetEvent event;

    if (enet_address_set_host(&enetaddress, addressstr.c_str()) < 0) {
        return false;
    }
    enetaddress.port = (short)port;
    ldb_uint address=enetaddress.host;
    printf("Connecting to %u.%u.%u.%u : %u\n", (address&0xFF), (address&0xFF00)>>8, (address&0xFF0000)>>16, (address&0xFF000000)>>24, enetaddress.port);

    if (!client_) {
        client_ = enet_host_create(nullptr, 1, 1, 0, 0);
        peer_=nullptr;
        if (!client_) {
            disconnect();
            return false;
        }
        printf("Create client @ 0x%p\n", client_);
    }
    if (!peer_) {
        ldbOSSleep(1.f);
        peer_ = enet_host_connect(client_, &enetaddress, 1, 0);

        while (enet_host_service(client_, &event, 0) >= 0) {
            if (event.type == ENET_EVENT_TYPE_CONNECT) {
                // connected OK
                address=event.peer->address.host;
                printf("Connection from %u.%u.%u.%u : %u\n", (address&0xFF), (address&0xFF00)>>8, (address&0xFF0000)>>16, (address&0xFF000000)>>24, event.peer->address.port);
                rpcOpNumber_ = 1;
                debuggerRunning_ = false;
                debuggerConnected_ = true;
                debuggerExit_ = false;
                localsStackLevel_ = -1;
                return true;
            } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                // failed
                printf("disconnect...\n");
                disconnect();
                return false;
            }
        }
    }
    printf("Should never get here...");
    return false;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbClient::disconnect() {
    ENetEvent event;
    debuggerConnected_ = false;
    debuggerExit_ = true;
    if (peer_) {
        enet_peer_disconnect(peer_, 0);

        /* 
            Allow up to 3 seconds for the disconnect to succeed
            and drop any packets received packets.
        */
        while (enet_host_service(client_, &event, 3000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy (event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                return;
            default:
                break;
            }
        }
        /* We've arrived here, so the disconnect attempt didn't */
        /* succeed yet.  Force the connection down.             */
        enet_peer_reset(peer_);
        peer_=nullptr;
    }

    if (client_) {
        enet_host_destroy(client_);
        client_=nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbClient::tickSingleEvent(ldb_uint timeout, ldbTickResults* outresults) {
    ENetEvent enetevent;

    if (!isConnected() || debuggerExit() || !client_) {
        return;
    }

    if (enet_host_service(client_, &enetevent, timeout) > 0) {
        switch(enetevent.type) {
        case ENET_EVENT_TYPE_NONE: break;
        case ENET_EVENT_TYPE_CONNECT: {
            ldb_uint address=enetevent.peer->address.host;
            printf("Connection from %u.%u.%u.%u : %u\n", (address&0xFF), (address&0xFF00)>>8, (address&0xFF0000)>>16, (address&0xFF000000)>>24, enetevent.peer->address.port);
        } break;
        case ENET_EVENT_TYPE_DISCONNECT: {
            ldb_uint address=enetevent.peer->address.host;
            disconnect();
            printf("Disconnection from %u.%u.%u.%u : %u\n", (address&0xFF), (address&0xFF00)>>8, (address&0xFF0000)>>16, (address&0xFF000000)>>24, enetevent.peer->address.port);
            return;
        } break;
        case ENET_EVENT_TYPE_RECEIVE: {
            if (enetevent.packet) {
                handlePacket(client_, enetevent.peer, enetevent.packet, outresults);
                enet_packet_destroy(enetevent.packet);
            }
        } break;
        default: break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbClient::tickMultipleEvents(ldb_uint timeout, ldbTickResults* outresults) {
    if (!isConnected() || debuggerExit() || !client_) {
        return;
    }

    ldb_uint32 time=ldbOSGetSysTimems();
    do {
        tickSingleEvent(timeout, outresults);
        ldb_uint32 timediff=ldbOSGetSysTimems()-time;
        time=ldbOSGetSysTimems();
        timeout = timediff > timeout ? 0 : timeout - timediff;
    } while (timeout > 0);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbClient::handlePacket(ENetHost* client, ENetPeer * peer, ENetPacket * packet, ldbTickResults* outresults) {
    using namespace luadb;

    proto::RPCCall rpccall;
    rpccall.ParseFromArray(packet->data, packet->dataLength);

    switch(rpccall.cmd()) {
    case proto::eDebuggerCmd_Run: {
    } break;
    case proto::eDebuggerCmd_Step: {
    } break;
    case proto::eDebuggerCmd_StepIn: {
    } break;
    case proto::eDebuggerCmd_StepOut: {
    } break;
    case proto::eDebuggerCmd_DebuggerExit: {
        disconnect();
    } break;
    case proto::eDebuggerCmd_RemoveBreakpoint:
    case proto::eDebuggerCmd_SetBreakpoint: {
        if (rpccall.has_breakpointdata() && rpccall.breakpointdata().has_breakpointset() && 
            rpccall.breakpointdata().breakpointset() && rpccall.opnumber() == rpcOpNumber_) {
            rpcOpComplete_ = true;
        }
    } break;
    case proto::eDebuggerCmd_Backtrace: {
        if (rpccall.has_backtracedata() && rpccall.opnumber() == rpcOpNumber_) {
            //fill the back trace
            stack_.clear();
            stack_.resize(rpccall.backtracedata().stackbacktrace_size());
            const proto::RPCBacktrace& bt = rpccall.backtracedata();
            for (ldb_uint i=0, n=rpccall.backtracedata().stackbacktrace_size(); i<n; ++i) {
                const proto::RPCStackLevel& l = bt.stackbacktrace(i);
                if (l.has_linenumber()) {
                    stack_[i].currentLine_ = l.linenumber();
                } else {
                    stack_[i].currentLine_ = -1;
                }
                if (l.has_filename()) {
                    stack_[i].source_ = l.filename();
                }
                if (l.has_printfilename()) {
                    stack_[i].shortSrc_ = l.printfilename();
                }
                if (l.has_name()) {
                    stack_[i].name_ = l.name();
                }
                if (l.has_namewhat()) {
                    if (l.name() == "global") {
                        stack_[i].nameType_ = eNameType_Global;
                    } else if (l.name() == "local") {
                        stack_[i].nameType_ = eNameType_Local;
                    } else if (l.name() == "method") {
                        stack_[i].nameType_ = eNameType_Method;
                    } else if (l.name() == "field") {
                        stack_[i].nameType_ = eNameType_Field;
                    } else if (l.name() == "upvalue") {
                        stack_[i].nameType_ = eNameType_UpValue;
                    } else {
                        stack_[i].nameType_ = eNameType_None;
                    }
                }
                if (l.has_iscfunction() && l.iscfunction()) {
                    stack_[i].funcType_ = eFunctionType_C;
                } else if (l.has_ismain() && l.ismain()) {
                    stack_[i].funcType_ = eFunctionType_main;
                } else {
                    stack_[i].funcType_ = eFunctionType_Lua;
                }

            }
            rpcOpComplete_ = true;
        }
    } break;
    case proto::eDebuggerCmd_GetLocals: {
        if (rpccall.has_localsstacklevel() && rpccall.opnumber() == rpcOpNumber_) {

            locals_.clear();
            for (ldb_uint i=0, n=rpccall.locals_size(); i<n; ++i) {
                const proto::RPCVar& rpcvar=rpccall.locals(i);
                locals_.emplace_back();
                auto var=locals_.rbegin();
                initVarDesc(&(*var), rpcvar);
            }
            localsStackLevel_ = rpccall.localsstacklevel();
            rpcOpComplete_ = true;
        }
    } break;
    case proto::eDebuggerCmd_GetWatch: {
        if (rpccall.has_watchvar() && rpccall.opnumber() == rpcOpNumber_ && outresults && outresults->outResult) {
            initVarDesc(outresults->outResult, rpccall.watchvar());
        }
        rpcOpComplete_ = true;
    } break;
    case proto::eDebuggerCmd_GetVarMoreInfo: {
        if (rpccall.has_moreinfovar() && outresults && outresults->outResult) {
            ldbVarDesc* ovar = outresults->outResult;
            initVarDesc(ovar, rpccall.moreinfovar());
            rpcOpComplete_ = true;
        }
    } break;
    case proto::eDebuggerCmd_RunningState: {
        if (rpccall.has_runningstatedata()) {
            debuggerRunning_ = rpccall.runningstatedata();
            localsStackLevel_ = -1;
        }
    } break;
    case proto::eDebuggerCmd_LineInfo: {
        if (rpccall.has_linemessagedata()) {
            currentFile_ = rpccall.linemessagedata().absolutefilepath();
            currentLine_ = rpccall.linemessagedata().linenumber();
        }
    } break;
    case proto::eDebuggerCmd_DebugConsole: {
        if (rpccall.has_debugconsolemessage()) {
            if (debugMsgDelegate_) {
                debugMsgDelegate_(rpccall.debugconsolemessage().c_str());
            }
        }
    } break;
    case proto::eDebuggerCmd_Noop:
        break;
    default: 
        assert(0 && "Unexpected Debugger Cmd");
        break;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbClient::run() {
    using namespace luadb;

    proto::RPCCall rpccall;
    rpccall.set_cmd(proto::eDebuggerCmd_Run);
    sendPacket(peer_, rpccall);

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbClient::trap() {
    using namespace luadb;

    proto::RPCCall rpccall;
    rpccall.set_cmd(proto::eDebuggerCmd_Trap);
    sendPacket(peer_, rpccall);

    return true;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbClient::sendPacket(ENetPeer* peer, const luadb::proto::RPCCall& rpccall) {
    ldb_int msgsize = rpccall.ByteSize();
    ENetPacket* packet = enet_packet_create(nullptr, msgsize, ENET_PACKET_FLAG_RELIABLE);
    rpccall.SerializeToArray(packet->data, msgsize);
    enet_peer_send(peer, 0, packet);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbClient::step() {
    using namespace luadb;

    proto::RPCCall rpccall;
    rpccall.set_cmd(proto::eDebuggerCmd_Step);
    sendPacket(peer_, rpccall);

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbClient::stepIn() {
    using namespace luadb;

    proto::RPCCall rpccall;
    rpccall.set_cmd(proto::eDebuggerCmd_StepIn);
    sendPacket(peer_, rpccall);

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbClient::stepOut() {
    using namespace luadb;

    proto::RPCCall rpccall;
    rpccall.set_cmd(proto::eDebuggerCmd_StepOut);
    sendPacket(peer_, rpccall);

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbClient::setBreakpoint(const ldb_string& filepath, ldb_uint line, ldb_bool set) {
    using namespace luadb;
    ldbTickResults results;
    proto::RPCCall rpccall;
    rpccall.set_cmd(set ? proto::eDebuggerCmd_SetBreakpoint :  proto::eDebuggerCmd_RemoveBreakpoint);
    rpccall.mutable_breakpointdata()->set_absolutefilepath(filepath);
    rpccall.mutable_breakpointdata()->set_linenumber(line);
    rpccall.set_opnumber(rpcOpNumber_);
    rpcOpComplete_ = false;

    sendPacket(peer_, rpccall);

    while (isConnected() && !rpcOpComplete_) {
        tickSingleEvent(1000, &results);
    }

    ++rpcOpNumber_;
    return isConnected();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbClient::getBacktrace() {
    using namespace luadb;
    ldbTickResults results;
    proto::RPCCall rpccall;
    rpccall.set_cmd(proto::eDebuggerCmd_Backtrace);
    rpccall.set_opnumber(rpcOpNumber_);
    rpcOpComplete_ = false;

    sendPacket(peer_, rpccall);

    while(isConnected() && !rpcOpComplete_) {
        tickSingleEvent(1000, &results);
    }

    rpcOpComplete_ |= true;
    return isConnected();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_debugMsgDelegate ldbClient::setMessageDelegate(const ldb_debugMsgDelegate& val) {
    ldb_debugMsgDelegate ret = debugMsgDelegate_;
    debugMsgDelegate_ = val;
    return ret;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ldbClient::addSearchPath(const ldb_char* path) {
    using namespace luadb;

    proto::RPCCall rpccall;
    rpccall.set_cmd(proto::eDebuggerCmd_AddSearchPath);
    rpccall.set_searchpath(path);
    sendPacket(peer_, rpccall);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbClient::getLocals(ldb_int stacklevel, std::vector< ldbVarDesc >* outlocals) {
    using namespace luadb;

    if (localsStackLevel_ != stacklevel) {
        ldbTickResults results;
        proto::RPCCall rpccall;
        rpccall.set_cmd(proto::eDebuggerCmd_GetLocals);
        rpccall.set_opnumber(rpcOpNumber_);
        rpccall.set_localsstacklevel(stacklevel);
        rpcOpComplete_ = false;

        sendPacket(peer_, rpccall);

        while(isConnected() && !rpcOpComplete_) {
            tickSingleEvent(1000, &results);
        }

        rpcOpComplete_ |= true;
    }

    *outlocals = locals_;

    return isConnected();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbClient::getMoreInfo(ldb_int varEntryID, ldbVarDesc* outvar) {
    using namespace luadb;
    ldbTickResults results;
    proto::RPCCall rpccall;
    rpccall.set_cmd(proto::eDebuggerCmd_GetVarMoreInfo);
    rpccall.set_opnumber(rpcOpNumber_);
    rpccall.set_varmoreinfoid(varEntryID);
    rpcOpComplete_ = false;

    sendPacket(peer_, rpccall);
    results.outResult = outvar;

    while(isConnected() && !rpcOpComplete_) {
        tickSingleEvent(1000, &results);
    }

    rpcOpComplete_ |= true;

    return isConnected();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ldb_bool ldbClient::getWatch(const ldb_char* varname, ldb_int stacklevel, ldbVarDesc* outvar) {
    using namespace luadb;
    ldbTickResults results;
    proto::RPCCall rpccall;
    rpccall.set_cmd(proto::eDebuggerCmd_GetWatch);
    rpccall.set_opnumber(rpcOpNumber_);
    rpccall.set_watchvarstring(varname);
    rpccall.set_localsstacklevel(stacklevel);
    rpcOpComplete_ = false;

    sendPacket(peer_, rpccall);
    results.outResult = outvar;

    while(isConnected() && !rpcOpComplete_) {
        tickSingleEvent(1000, &results);
    }

    rpcOpComplete_ |= true;

    return isConnected();
}

