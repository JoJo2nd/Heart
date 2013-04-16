cmake_minimum_required(VERSION 2.8)

if (MSVC)
    add_definitions(/wd"4244")
    add_definitions(/wd"4146")
    add_definitions(/wd"4305")
endif()

set(LUA_SA_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( LUA_SA_INCLUDE_DIRS
    "${LUA_SA_SRC_DIR}/include"
	"${EXTERNAL_SRC_ROOT}/lua/src"
)

set( LUA_SA_HDR_FILES )

set( LUA_SA_SRC_FILES
    "${LUA_SA_SRC_DIR}/src/lua.c"
)

source_group(include FILES ${LUA_SA_HDR_FILES})
source_group(source  FILES ${LUA_SA_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${LUA_SA_INCLUDE_DIRS})
add_executable(lua_standalone ${LUA_SA_SRC_FILES} ${LUA_SA_HDR_FILES})
target_link_libraries(lua_standalone lua)
set_property(TARGET lua_standalone PROPERTY DEBUG_POSTFIX _d)

set_target_properties(lua_standalone PROPERTIES OUTPUT_NAME luai)

ADD_CUSTOM_COMMAND(
	TARGET lua_standalone
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" luai "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/builder"
)