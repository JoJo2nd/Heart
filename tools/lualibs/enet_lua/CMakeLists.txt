cmake_minimum_required(VERSION 2.8)

if (MSVC)
    #add_definitions(/wd"4244")
    #add_definitions(/wd"4146")
    #add_definitions(/wd"4305")
endif()

set(ENET_LUA_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( ENET_LUA_INCLUDE_DIRS
    "${ENET_LUA_SRC_DIR}/include"
	"${EXTERNAL_SRC_ROOT}/lua/src"
	"${ENET_INCLUDE_DIR}"
)

file(GLOB ENET_LUA_HDR_FILES 
    "${ENET_LUA_SRC_DIR}/include/*.h"
)

file(GLOB ENET_LUA_SRC_FILES
    "${ENET_LUA_SRC_DIR}/src/*.c"
)

source_group(include FILES ${ENET_LUA_HDR_FILES})
source_group(source  FILES ${ENET_LUA_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${ENET_LUA_INCLUDE_DIRS})
add_library(enet_lua SHARED ${ENET_LUA_SRC_FILES} ${ENET_LUA_HDR_FILES})
target_link_libraries(enet_lua lua52 enet)
set_property(TARGET enet_lua PROPERTY DEBUG_POSTFIX)

ADD_CUSTOM_COMMAND(
	TARGET enet_lua
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" enet_lua "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
)