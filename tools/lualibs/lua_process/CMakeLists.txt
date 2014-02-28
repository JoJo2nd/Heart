cmake_minimum_required(VERSION 2.8)

if (MSVC)
    #add_definitions(/wd"4244")
    #add_definitions(/wd"4146")
    #add_definitions(/wd"4305")
endif()

set(LUAPROCESS_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( LUAPROCESS_INCLUDE_DIRS
    "${LUAPROCESS_SRC_DIR}/include"
	"${EXTERNAL_SRC_ROOT}/lua/src"
	"${ENET_INCLUDE_DIR}"
)

file(GLOB LUAPROCESS_HDR_FILES 
    "${LUAPROCESS_SRC_DIR}/include/*.h"
)

file(GLOB LUAPROCESS_SRC_FILES
    "${LUAPROCESS_SRC_DIR}/src/*.c"
)

source_group(include FILES ${LUAPROCESS_HDR_FILES})
source_group(source  FILES ${LUAPROCESS_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${LUAPROCESS_INCLUDE_DIRS})
add_library(lua_process SHARED ${LUAPROCESS_SRC_FILES} ${LUAPROCESS_HDR_FILES})
target_link_libraries(lua_process lua52)
set_property(TARGET lua_process PROPERTY DEBUG_POSTFIX)

ADD_CUSTOM_COMMAND(
	TARGET lua_process
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" lua_process "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
)