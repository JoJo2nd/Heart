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

file(GLOB LUA_SA_SRC_FILES 
	"${LUA_SA_SRC_DIR}/src/*.h"
    "${LUA_SA_SRC_DIR}/src/*.c"
	"${LUA_SA_SRC_DIR}/src/*.cpp"
	"${LUA_SA_SRC_DIR}/src/*.cxx"
)
file(GLOB LUA_SA_SRC_LUA_FILES "${LUA_SA_SRC_DIR}/src/lua/*.lua")

source_group(source  FILES ${LUA_SA_SRC_FILES})
source_group(lua FILES ${LUA_SA_SRC_LUA_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

# stop cmake failing when trying to find compiler for lua files
set_source_files_properties(${LUA_SA_SRC_LUA_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

include_directories(${LUA_SA_INCLUDE_DIRS})
add_executable(lua_standalone ${LUA_SA_SRC_FILES} ${LUA_SA_SRC_LUA_FILES})
target_link_libraries(lua_standalone lua52)
#set_property(TARGET lua_standalone PROPERTY DEBUG_POSTFIX _d)

set_target_properties(lua_standalone PROPERTIES OUTPUT_NAME lua)

ADD_CUSTOM_COMMAND(
	TARGET lua_standalone
    POST_BUILD
	COMMAND python
	ARGS "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.py" -l lua -s "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
	COMMAND python
	ARGS "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_files.py" -r .*\\.lua -s "${LUA_SA_SRC_DIR}/src/lua" -d "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
)