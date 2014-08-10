cmake_minimum_required(VERSION 2.8)

if (MSVC)
    # Lua is using float (not doubles, which is it's normal mode) so 
    # it generates alot of warnings, hide them for the time being.
    add_definitions(/wd"4244")
    add_definitions(/wd"4146")
    add_definitions(/wd"4305")
endif()

ENABLE_PROFILE_IF_SET()

set(LUA_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( LUA_INCLUDE_DIRS
    "${LUA_SRC_DIR}/src"
)

file(GLOB LUA_SRC_FILES 
    "${LUA_SRC_DIR}/src/*.h"
    "${LUA_SRC_DIR}/src/*.c"
)

list(REMOVE_ITEM LUA_SRC_FILES "${LUA_SRC_DIR}/src/lua.c")
list(REMOVE_ITEM LUA_SRC_FILES "${LUA_SRC_DIR}/src/luac.c")

if (PLATFORM_LINUX)
    add_definitions(-DLUA_USE_POSIX -DLUA_USE_DLOPEN)
    set(LUA_LIBS m dl)
endif()

source_group(source  FILES ${LUA_SRC_FILES})

#set_source_files_properties(${LUA_SRC_FILES} PROPERTIES LANGUAGE CXX)

include_directories(${LUA_INCLUDE_DIRS})
add_library(lua52 SHARED ${LUA_SRC_FILES} ${LUA_HDR_FILES})
target_link_libraries(lua52 ${LUA_LIBS} ${PROFILE_LIBS})
set_property(TARGET lua52 PROPERTY DEBUG_POSTFIX)

ADD_CUSTOM_COMMAND(
	TARGET lua52
    POST_BUILD
	COMMAND python "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -l lua52 -s "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" -d "${CMAKE_SOURCE_DIR}/deploy/game"
	COMMAND python "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -l lua52 -s "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
    COMMAND python "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -l wx  -s "${wxLua_BIN_DIR_ROOT}/${CMAKE_CFG_INTDIR}" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
)
