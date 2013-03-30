cmake_minimum_required(VERSION 2.8)

set(LUA_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( LUA_INCLUDE_DIRS
    "${LUA_SRC_DIR}/src"
)

set( LUA_HDR_FILES
    "${LUA_SRC_DIR}/src/lapi.h"
    "${LUA_SRC_DIR}/src/lauxlib.h"
    "${LUA_SRC_DIR}/src/lcode.h"
    "${LUA_SRC_DIR}/src/lctype.h"
    "${LUA_SRC_DIR}/src/ldebug.h"
    "${LUA_SRC_DIR}/src/ldo.h"
    "${LUA_SRC_DIR}/src/lfunc.h"
    "${LUA_SRC_DIR}/src/lgc.h"
    "${LUA_SRC_DIR}/src/llex.h"
    "${LUA_SRC_DIR}/src/llimits.h"
    "${LUA_SRC_DIR}/src/lmem.h"
    "${LUA_SRC_DIR}/src/lobject.h"
    "${LUA_SRC_DIR}/src/lopcodes.h"
    "${LUA_SRC_DIR}/src/lparser.h"
    "${LUA_SRC_DIR}/src/lstate.h"
    "${LUA_SRC_DIR}/src/lstring.h"
    "${LUA_SRC_DIR}/src/ltable.h"
    "${LUA_SRC_DIR}/src/ltm.h"
    "${LUA_SRC_DIR}/src/lua.h"
    "${LUA_SRC_DIR}/src/luaconf.h"
    "${LUA_SRC_DIR}/src/lualib.h"
    "${LUA_SRC_DIR}/src/lundump.h"
    "${LUA_SRC_DIR}/src/lvm.h"
    "${LUA_SRC_DIR}/src/lzio.h"
)

set( LUA_SRC_FILES
    "${LUA_SRC_DIR}/src/lapi.c"
    "${LUA_SRC_DIR}/src/lauxlib.c"
    "${LUA_SRC_DIR}/src/lbaselib.c"
    "${LUA_SRC_DIR}/src/lbitlib.c"
    "${LUA_SRC_DIR}/src/lcode.c"
    "${LUA_SRC_DIR}/src/lcorolib.c"
    "${LUA_SRC_DIR}/src/lctype.c"
    "${LUA_SRC_DIR}/src/ldblib.c"
    "${LUA_SRC_DIR}/src/ldebug.c"
    "${LUA_SRC_DIR}/src/ldo.c"
    "${LUA_SRC_DIR}/src/ldump.c"
    "${LUA_SRC_DIR}/src/lfunc.c"
    "${LUA_SRC_DIR}/src/lgc.c"
    "${LUA_SRC_DIR}/src/linit.c"
    "${LUA_SRC_DIR}/src/liolib.c"
    "${LUA_SRC_DIR}/src/llex.c"
    "${LUA_SRC_DIR}/src/lmathlib.c"
    "${LUA_SRC_DIR}/src/lmem.c"
    "${LUA_SRC_DIR}/src/loadlib.c"
    "${LUA_SRC_DIR}/src/lobject.c"
    "${LUA_SRC_DIR}/src/lopcodes.c"
    "${LUA_SRC_DIR}/src/loslib.c"
    "${LUA_SRC_DIR}/src/lparser.c"
    "${LUA_SRC_DIR}/src/lstate.c"
    "${LUA_SRC_DIR}/src/lstring.c"
    "${LUA_SRC_DIR}/src/lstrlib.c"
    "${LUA_SRC_DIR}/src/ltable.c"
    "${LUA_SRC_DIR}/src/ltablib.c"
    "${LUA_SRC_DIR}/src/ltm.c"
    "${LUA_SRC_DIR}/src/lua.c"
    "${LUA_SRC_DIR}/src/lundump.c"
    "${LUA_SRC_DIR}/src/lvm.c"
    "${LUA_SRC_DIR}/src/lzio.c"
)

source_group(include FILES ${LUA_HDR_FILES})
source_group(source  FILES ${LUA_SRC_FILES})

include_directories(${LUA_INCLUDE_DIRS})
add_library(lua ${LUA_SRC_FILES} ${LUA_HDR_FILES})