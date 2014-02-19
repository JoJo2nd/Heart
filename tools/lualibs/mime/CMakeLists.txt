cmake_minimum_required(VERSION 2.8)

if (MSVC)
    # Lua is using float (not doubles, which is it's normal mode) so 
    # it generates alot of warnings, hide them for the time being.
    add_definitions(/wd"4244")
    add_definitions(/wd"4146")
    add_definitions(/wd"4305")
endif()

set(LUASOCKET_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( LUASOCKET_INCLUDE_DIRS
    "${LUA_SRC_DIR}/src"
	"${EXTERNAL_SRC_ROOT}/lua/src"
)

set( LUAMIME_HDR_FILES
	"${LUASOCKET_SRC_DIR}/src/mime.h"
)
set( LUAMIME_SRC_FILES
	"${LUASOCKET_SRC_DIR}/src/mime.c"
)
set( LUAMIME_LUA_FILES
	"${LUASOCKET_SRC_DIR}/lua/mime.lua"
)

add_definitions(-DLUASOCKET_EXPORTS -DMIME_EXPORTS -DLUA_COMPAT_MODULE)

source_group(include FILES ${LUAMIME_HDR_FILES})
source_group(source  FILES ${LUAMIME_SRC_FILES})
source_group(lua	 FILES ${LUAMIME_LUA_FILES})

if (WIN32)
	set(PLAT_SOCKET_LIB ws2_32) #winsock2
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${LUASOCKET_INCLUDE_DIRS})
add_library(lua_mime SHARED ${LUAMIME_SRC_FILES} ${LUAMIME_HDR_FILES} ${LUAMIME_LUA_FILES})
target_link_libraries(lua_mime lua52 ${PLAT_SOCKET_LIB})
set_target_properties(lua_mime PROPERTIES OUTPUT_NAME core)

ADD_CUSTOM_COMMAND(
	TARGET lua_mime
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" core "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/builder/mime"
	COMMAND ROBOCOPY "${LUASOCKET_SRC_DIR}/lua" "${CMAKE_SOURCE_DIR}/deploy/builder/lua" *.lua ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
)