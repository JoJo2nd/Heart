cmake_minimum_required(VERSION 2.8)

if (PLATFORM_LINUX)
#	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}")
endif()

set(LUA_FILESYSTEM_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
 
set( LUA_FILESYSTEM_INCLUDE_DIRS
    "${LUA_FILESYSTEM_SRC_DIR}/include"
	"${LUA_INCLUDE_DIR}"
	"${MINFS_INCLUDE_DIR}"
    "${CRYPTO_INCLUDE_DIR}"
)

file(GLOB LUA_FILESYSTEM_SRC_FILES
	"${LUA_FILESYSTEM_SRC_DIR}/src/*.h"
	"${LUA_FILESYSTEM_SRC_DIR}/src/*.c"
)

source_group(include FILES ${LUA_FILESYSTEM_HDR_FILES})
source_group(source  FILES ${LUA_FILESYSTEM_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${LUA_FILESYSTEM_INCLUDE_DIRS})
add_library(lua_filesystem SHARED ${LUA_FILESYSTEM_SRC_FILES} ${LUA_FILESYSTEM_HDR_FILES} ${LUA_BUIDLER_FILES})
target_link_libraries(lua_filesystem lua52 minfs crypto)
set_target_properties(lua_filesystem PROPERTIES OUTPUT_NAME filesystem)
set_property(TARGET lua_filesystem PROPERTY DEBUG_POSTFIX)

if (PLATFORM_WINDOWS)
#	ADD_CUSTOM_COMMAND(
#		TARGET lua_filesystem
#	    POST_BUILD
#	    COMMAND python
#	    ARGS "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -l filesystem -s "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
#	)
endif()
