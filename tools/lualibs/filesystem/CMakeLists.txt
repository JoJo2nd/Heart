cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.53.0 REQUIRED COMPONENTS filesystem system)

option(BUILD_LUA_FILESYSTEM_SHARED_LIB "Build lua file system as a Shared library - (TODO: Make this work)" ON)

if (MSVC)
    #add_definitions(/wd"4244")
    #add_definitions(/wd"4146")
    #add_definitions(/wd"4305")
endif()

if (BUILD_LUA_FILESYSTEM_SHARED_LIB)
	add_definitions(-DluaFILESYSTEM_COMPILE_SHARED_LIB)
endif()
#if (MSVC)
#	add_definitions(-D_ITERATOR_DEBUG_LEVEL=2)
#endif()

set(LUA_FILESYSTEM_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( LUA_FILESYSTEM_INCLUDE_DIRS
    "${LUA_FILESYSTEM_SRC_DIR}/include"
	"${EXTERNAL_SRC_ROOT}/lua/src"
)

set( LUA_FILESYSTEM_HDR_FILES 
    "${LUA_FILESYSTEM_SRC_DIR}/include/lua_filesystem.h"
)

set( LUA_FILESYSTEM_SRC_FILES
    "${LUA_FILESYSTEM_SRC_DIR}/src/lua_filesystem.cpp"
)

source_group(include FILES ${LUA_FILESYSTEM_HDR_FILES})
source_group(source  FILES ${LUA_FILESYSTEM_SRC_FILES})

add_definitions(-DLUA_CORE)

include_directories(${LUA_FILESYSTEM_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
add_library(lua_filesystem SHARED ${LUA_FILESYSTEM_SRC_FILES} ${LUA_FILESYSTEM_HDR_FILES})
target_link_libraries(lua_filesystem lua ${Boost_LIBRARIES})
set_target_properties(lua_filesystem PROPERTIES OUTPUT_NAME filesystem)

ADD_CUSTOM_COMMAND(
	TARGET lua_filesystem
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" filesystem "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/builder/filesystem"
)