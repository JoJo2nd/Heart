cmake_minimum_required(VERSION 2.8)

if (MSVC)
    # Lua is using float (not doubles, which is it's normal mode) so 
    # it generates alot of warnings, hide them for the time being.
    add_definitions(/wd"4244")
    add_definitions(/wd"4146")
    add_definitions(/wd"4305")
endif()

set(LUAXML_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( LUAXML_INCLUDE_DIRS
	"${EXTERNAL_SRC_ROOT}/lua/src"
)
set( LUAXML_SRC_FILES
	"${LUAXML_SRC_DIR}/LuaXML_lib.c"
)
set( LUAMIME_LUA_FILES
	"${LUAXML_SRC_DIR}/LuaXML.lua"
)

source_group(source  FILES ${LUAXML_SRC_FILES})
source_group(lua	 FILES ${LUAXML_LUA_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${LUAXML_INCLUDE_DIRS})
add_library(lua_xml SHARED  ${LUAXML_SRC_FILES} ${LUAXML_LUA_FILES})
target_link_libraries(lua_xml lua52)
set_target_properties(lua_xml PROPERTIES OUTPUT_NAME LuaXML_lib)
