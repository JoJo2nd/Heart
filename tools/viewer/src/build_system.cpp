/********************************************************************
    
    Copyright (c) 9:4:2014 James Moran
    
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

#include "build_system.h"
#include "lua_filesystem.h"
#include "lua_process.h"
#include "proto_lua.h"

namespace build {
namespace {

struct DataBuild {
    BuildLogHandler     msgProc_;
    std::string         buildPath_;
    std::string         outputPath_;
    void*               userPtr_;
};

std::thread         backgroundThread_;
std::atomic_bool    buildingFlag_;
DataBuild*          activeBuild_ = nullptr;
Report*             lastReport_ = nullptr;

#include "lua/builder_script.inl"

int luaReportPrint(lua_State* L) {
    bool error=false;
    BuildLogHandler ext_proc = (BuildLogHandler)lua_topointer(L, lua_upvalueindex(1));
    void* user_ptr = (void*)lua_topointer(L, lua_upvalueindex(2));
    { // scope for C++ objects
        std::string print_msg; // string to build up our message
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
            if (s == NULL) {
                error = true;
                break;
            }
            if (i>1) {
                print_msg += "\t";
            }
            print_msg += s;
            lua_pop(L, 1);  /* pop result */
        }
        print_msg += "\n";

        if (ext_proc && !error) {
            ext_proc(print_msg.c_str(), print_msg.length(), user_ptr);
        }
    }

    if (error) {
        return luaL_error(L, "'tostring' must return a string to 'print'");
    }
    return 0;
}

void luaBuildThread(const DataBuild* build_input, Report* build_output) {
    lua_State* L = luaL_newstate();

    //open the default libraries (filesystem, lua_process, proto_lua)
    luaL_openlibs(L);
    luaopen_filesystem(L);
    luaopen_lua_process(L);
    luaopen_proto_lua(L);

    //set the print function to call us...
    static const luaL_Reg printLib[] = {
        {"print", luaReportPrint},
        {NULL, NULL}
    };
    // push our version of print
    lua_pushglobaltable(L);
    lua_pushlightuserdata(L, build_input->msgProc_);
    lua_pushlightuserdata(L, build_input->userPtr_);
    luaL_setfuncs(L, printLib, 2);

    // push input parameters for build
    lua_pushstring(L, build_input->buildPath_.c_str());
    lua_setfield(L, -2, "in_data_path");
    lua_pushstring(L, build_input->outputPath_.c_str());
    lua_setfield(L, -2, "in_output_data_path");

    //do the build...
    int error = luaL_loadbuffer(L, builder_script_data, builder_script_data_len, "Data Builder Script");
    assert(error == LUA_OK);
    if (error == LUA_OK) {
        if (error = lua_pcall(L, 0, LUA_MULTRET, 0)) {
            const char* err_msg = lua_tostring(L, -1);
            //comms::channelMessage(verbosity, "target channel", "msg format", [args])
            printf(err_msg);
        }
    }

    //shut down...
    lua_close(L);

    buildingFlag_.store(false);
}

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ErrorCodeEnum beginDataBuild(const std::string& build_path, const std::string& output_path, BuildLogHandler msg_handler, void* user_ptr) {
    if (isBuildingData()) {
        return ErrorCode::AlreadyBuilding;
    }

    buildingFlag_.store(true);

    delete activeBuild_;
    activeBuild_ = new DataBuild();
    activeBuild_->buildPath_ = build_path;
    activeBuild_->outputPath_ = output_path;
    activeBuild_->msgProc_ = msg_handler;
    activeBuild_->userPtr_ = user_ptr;

    delete lastReport_;
    lastReport_ = new Report();

    backgroundThread_ = std::thread(luaBuildThread, activeBuild_, lastReport_);
    backgroundThread_.detach();

    return ErrorCode::OK;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool isBuildingData() {
    return buildingFlag_.load();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool dataBuildComplete() {
    return !isBuildingData();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ErrorCodeEnum getLastBuildReport(Report* out_report) {
    if (!out_report || !lastReport_) {
        return ErrorCode::NoReportAvailable;
    }

    *out_report = *lastReport_;
    delete lastReport_;
    lastReport_ = nullptr;
    return ErrorCode::NoReportAvailable;
}


}