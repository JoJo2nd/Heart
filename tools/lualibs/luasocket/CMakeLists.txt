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

set( LUASOCKET_HDR_FILES
    "${LUASOCKET_SRC_DIR}/src/auxiliar.h"
    "${LUASOCKET_SRC_DIR}/src/buffer.h"
    "${LUASOCKET_SRC_DIR}/src/except.h"
    "${LUASOCKET_SRC_DIR}/src/inet.h"
    "${LUASOCKET_SRC_DIR}/src/io.h"
    "${LUASOCKET_SRC_DIR}/src/luasocket.h"
    "${LUASOCKET_SRC_DIR}/src/options.h"
    "${LUASOCKET_SRC_DIR}/src/select.h"
    "${LUASOCKET_SRC_DIR}/src/socket.h"
    "${LUASOCKET_SRC_DIR}/src/tcp.h"
    "${LUASOCKET_SRC_DIR}/src/timeout.h"
    "${LUASOCKET_SRC_DIR}/src/udp.h"
    "${LUASOCKET_SRC_DIR}/src/usocket.h"
    "${LUASOCKET_SRC_DIR}/src/wsocket.h"
)
set( LUASOCKET_SRC_FILES
    "${LUASOCKET_SRC_DIR}/src/auxiliar.c"
    "${LUASOCKET_SRC_DIR}/src/buffer.c"
    "${LUASOCKET_SRC_DIR}/src/except.c"
    "${LUASOCKET_SRC_DIR}/src/inet.c"
    "${LUASOCKET_SRC_DIR}/src/io.c"
    "${LUASOCKET_SRC_DIR}/src/luasocket.c"
    "${LUASOCKET_SRC_DIR}/src/options.c"
    "${LUASOCKET_SRC_DIR}/src/select.c"
    "${LUASOCKET_SRC_DIR}/src/tcp.c"
    "${LUASOCKET_SRC_DIR}/src/timeout.c"
    "${LUASOCKET_SRC_DIR}/src/udp.c"
    "${LUASOCKET_SRC_DIR}/src/wsocket.c"
)
set( LUASOCKET_LUA_FILES
    "${LUASOCKET_SRC_DIR}/lua/ftp.lua"
    "${LUASOCKET_SRC_DIR}/lua/http.lua"
    "${LUASOCKET_SRC_DIR}/lua/ltn12.lua"
    "${LUASOCKET_SRC_DIR}/lua/smtp.lua"
    "${LUASOCKET_SRC_DIR}/lua/socket.lua"
    "${LUASOCKET_SRC_DIR}/lua/tp.lua"
    "${LUASOCKET_SRC_DIR}/lua/url.lua"
)

add_definitions(-DLUASOCKET_EXPORTS -DMIME_EXPORTS -DLUA_COMPAT_MODULE)

source_group(include FILES ${LUASOCKET_HDR_FILES})
source_group(source  FILES ${LUASOCKET_SRC_FILES})
source_group(lua	 FILES ${LUASOCKET_LUA_FILES})

if (WIN32)
	set(PLAT_SOCKET_LIB ws2_32) #winsock2
endif()

include_directories(${LUASOCKET_INCLUDE_DIRS})
add_library(lua_socket SHARED ${LUASOCKET_SRC_FILES} ${LUASOCKET_HDR_FILES} ${LUASOCKET_LUA_FILES})
target_link_libraries(lua_socket lua52 ${PLAT_SOCKET_LIB})
set_target_properties(lua_socket PROPERTIES OUTPUT_NAME core)

ADD_CUSTOM_COMMAND(
	TARGET lua_socket
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" core "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/builder/socket"
	COMMAND ROBOCOPY "${LUASOCKET_SRC_DIR}/lua" "${CMAKE_SOURCE_DIR}/deploy/builder/lua" *.lua ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
	COMMAND ROBOCOPY "${LUASOCKET_SRC_DIR}/lua" "${CMAKE_SOURCE_DIR}/deploy/builder/socket" "headers.lua" "tp.lua" "url.lua" ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
)