cmake_minimum_required(VERSION 2.8)

set(BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( BUILDER_INCLUDE_DIRS
    "${BUILDER_SRC_DIR}/src"
	"${EXTERNAL_SRC_ROOT}/lua/src"
	"${EXTERNAL_SRC_ROOT}/crypto/src"
	"${TOOLS_SRC_ROOT}/lualibs/filesystem/src"
	"${TOOLS_SRC_ROOT}/lualibs/lua_process/src"
	"${TOOLS_SRC_ROOT}/proto/src"
	"${HEART_SRC_ROOT}"
	"${PROTOBUF_INCLUDE_DIR}"
	"${GENERATED_PROTO_FULL_INC_DIRS}"
	"${GETOPT_PORT_INCLUDE_DIR}"
	"${MINFS_INCLUDE_DIR}"
)

file(GLOB BUILDER_SRC_FILES
    "${BUILDER_SRC_DIR}/src/*.h"
    "${BUILDER_SRC_DIR}/src/*.cpp"
)
file(GLOB TEXTURE_SRC_FILES
    "${BUILDER_SRC_DIR}/src/texture/*.h"
    "${BUILDER_SRC_DIR}/src/texture/*.cpp"
)
file(GLOB MESH_SRC_FILES
    "${BUILDER_SRC_DIR}/src/mesh/*.h"
        "${BUILDER_SRC_DIR}/src/mesh/*.cpp"
)
file(GLOB COMMON_SRC_FILES
    "${BUILDER_SRC_DIR}/src/common/*.h"
    "${BUILDER_SRC_DIR}/src/common/*.cpp"
)
file(GLOB BUILDER_PRECOMPILED_SRC_FILES
    "${BUILDER_SRC_DIR}/src/precompiled/*.h"
    "${BUILDER_SRC_DIR}/src/precompiled/*.cpp"
)

file(GLOB BUILDER_SRC_FILES "${BUILDER_SRC_DIR}/src/*.cpp")

#add_custom_command(
#      OUTPUT "${BUILDER_SRC_DIR}/src/lua/builder_script.inl"
#      COMMAND python
#      ARGS "${TOOLS_SRC_ROOT}/utils/file_to_cpp.py" -i "${BUILDER_SRC_DIR}/src/lua/builder.lua" -o "${BUILDER_SRC_DIR}/src/lua/builder_script.inl" -d builder_script
#      DEPENDS "${BUILDER_SRC_DIR}/src/lua/builder.lua"
#      COMMENT "Converting Lua script to C++ inline file"
#      VERBATIM )

set_source_files_properties(${GENERATED_FILES} PROPERTIES GENERATED TRUE)

source_group(include FILES ${BUILDER_HDR_FILES})
source_group(source  FILES ${BUILDER_SRC_FILES})
source_group(texture_module FILES ${TEXTURE_SRC_FILES})
source_group(mesh FILES ${MESH_SRC_FILES})
source_group(common FILES ${COMMON_SRC_FILES} ${COMMON_HDR_FILES})
source_group(precompiled FILES ${BUILDER_PRECOMPILED_SRC_FILES})
source_group(lua FILES ${BUILDER_LUA_FILES})
source_group(generated FILES ${GENERATED_FILES})

set(ALL_SRC_FILES
        ${BUILDER_SRC_FILES}
        ${BUILDER_LUA_FILES}
	${GENERATED_FILES}
)


if (MSVC)
	add_definitions(/WX) # Warnings as errors
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

if (PLATFORM_LINUX)
#  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}")
endif()

include_directories(
	${BUILDER_INCLUDE_DIRS}
	${GENERATED_PROTO_FULL_INC_DIRS})
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
add_executable(builder ${ALL_SRC_FILES})
target_link_libraries(builder optimized ${PROTOBUF_LIBRARY} crypto lua52 getopt_port proto_full minfs heart_base)
target_link_libraries(builder debug ${PROTOBUF_LIBRARY_DEBUG} crypto lua52 getopt_port proto_full minfs heart_base)

if (PLATFORM_WINDOWS)
#ADD_CUSTOM_COMMAND(
#        TARGET BUILDER
#        POST_BUILD
#	COMMAND ROBOCOPY "${wxWidgets_LIB_DIR}" "${CMAKE_SOURCE_DIR}/deploy/tools/${CMAKE_CFG_INTDIR}" *.dll ${ROBOCOPY_OPTIONS} \n IF %ERRORLEVEL% LEQ 3 SET ERRORLEV=0
#)
endif()
