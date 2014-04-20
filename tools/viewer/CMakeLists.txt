cmake_minimum_required(VERSION 2.8)

add_definitions(-DWXUSINGDLL)

set(VIEWER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( VIEWER_INCLUDE_DIRS
    "${VIEWER_SRC_DIR}/src"
	"${EXTERNAL_SRC_ROOT}/lua/src"
	"${EXTERNAL_SRC_ROOT}/crypto/src"
	"${TOOLS_SRC_ROOT}/lualibs/filesystem/src"
	"${TOOLS_SRC_ROOT}/lualibs/lua_process/src"
	"${TOOLS_SRC_ROOT}/proto/src"
	"${PROTOBUF_INCLUDE_DIR}"
	"${GENERATED_PROTO_FULL_INC_DIRS}"
)

file(GLOB VIEWER_SRC_FILES
    "${VIEWER_SRC_DIR}/src/*.h"
    "${VIEWER_SRC_DIR}/src/*.cpp"
)
file(GLOB TEXTURE_SRC_FILES
    "${VIEWER_SRC_DIR}/src/texture/*.h"
    "${VIEWER_SRC_DIR}/src/texture/*.cpp"
)
file(GLOB MESH_SRC_FILES
    "${VIEWER_SRC_DIR}/src/mesh/*.h"
	"${VIEWER_SRC_DIR}/src/mesh/*.cpp"
)
file(GLOB COMMON_SRC_FILES
    "${VIEWER_SRC_DIR}/src/common/*.h"
    "${VIEWER_SRC_DIR}/src/common/*.cpp"
)
file(GLOB VIEWER_PRECOMPILED_SRC_FILES
    "${VIEWER_SRC_DIR}/src/precompiled/*.h"
    "${VIEWER_SRC_DIR}/src/precompiled/*.cpp"
)
file(GLOB VIEWER_LUA_FILES "${VIEWER_SRC_DIR}/src/lua/*.lua")

set(GENERATED_FILES "${VIEWER_SRC_DIR}/src/lua/builder_script.inl")

add_custom_command(
      OUTPUT "${VIEWER_SRC_DIR}/src/lua/builder_script.inl"
	  COMMAND python
      ARGS "${TOOLS_SRC_ROOT}/utils/file_to_cpp.py" -i "${VIEWER_SRC_DIR}/src/lua/builder.lua" -o "${VIEWER_SRC_DIR}/src/lua/builder_script.inl" -d builder_script
      DEPENDS "${VIEWER_SRC_DIR}/src/lua/builder.lua"
      COMMENT "Converting Lua script to C++ inline file"
      VERBATIM )

set_source_files_properties(${VIEWER_LUA_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(${GENERATED_FILES} PROPERTIES GENERATED TRUE)

source_group(include FILES ${VIEWER_HDR_FILES})
source_group(source  FILES ${VIEWER_SRC_FILES})
source_group(texture_module FILES ${TEXTURE_SRC_FILES})
source_group(mesh FILES ${MESH_SRC_FILES})
source_group(common FILES ${COMMON_SRC_FILES} ${COMMON_HDR_FILES})
source_group(precompiled FILES ${VIEWER_PRECOMPILED_SRC_FILES})
source_group(lua FILES ${VIEWER_LUA_FILES})
source_group(generated FILES ${GENERATED_FILES})

set(ALL_SRC_FILES
	${VIEWER_SRC_FILES} 
    ${COMMON_SRC_FILES}
	${VIEWER_LUA_FILES}
	${GENERATED_FILES}
)

ADD_MSVC_PRECOMPILED_HEADER(
	"${VIEWER_SRC_DIR}/src/precompiled/precompiled.h" 
	"${VIEWER_SRC_DIR}/src/precompiled/precompiled.cpp" 
	"${ALL_SRC_FILES}"
)

if (MSVC)
	add_definitions(/WX) # Warnings as errors
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

include_directories(${VIEWER_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
link_directories(
	${LIBHEART_LIB_DIRS}
	"${EXTERNAL_SRC_ROOT}/freeimage/Dist"
)
if (BUILD_64_BIT)
	link_directories( "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x64" )
	set(freeimage_BIN_DIR "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x64")
else()
	link_directories( "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x86" )
	set(freeimage_BIN_DIR "${EXTERNAL_SRC_ROOT}/freeimage/Dist/x86")
endif()
add_executable(viewer WIN32 ${ALL_SRC_FILES} ${VIEWER_PRECOMPILED_SRC_FILES})
set_property(TARGET viewer PROPERTY DEBUG_POSTFIX)
target_link_libraries(viewer crypto lua52 lua_filesystem lua_process proto_lua proto_full ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES})

ADD_CUSTOM_COMMAND(
	TARGET viewer
    POST_BUILD
	COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" viewer "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}"
	COMMAND ROBOCOPY "${wxWidgets_LIB_DIR}" "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}" *.dll ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
)