cmake_minimum_required(VERSION 2.8)

set(MESH_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( MESH_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
	${MINFS_INCLUDE_DIR}
    "${MESH_BUILDER_SRC_DIR}/src"
)
file(GLOB MESH_BUILDER_SRC_FILES 
    "${MESH_BUILDER_SRC_DIR}/src/*.h"
    "${MESH_BUILDER_SRC_DIR}/src/*.cpp"
)
set(MESH_BUILDER_LIB_DIRS
	"${EXTERNAL_SRC_ROOT}/openal/libs/Win32"
)
if (WIN32)
    if (CMAKE_CL_64)
        set(MESH_BUILDER_LIB_DIRS
            ${MESH_BUILDER_LIB_DIRS}
            "${assimp_LIB_DIR}/assimp_release-dll_x64"
        )
        set(FULL_ASSIMP_BIN_PATH
            "${assimp_BIN_DIR}/assimp_release-dll_x64/"
        )
    else()
        set(MESH_BUILDER_LIB_DIRS
            ${MESH_BUILDER_LIB_DIRS}
            "${assimp_LIB_DIR}/assimp_release-dll_win32"
        )
        set(FULL_ASSIMP_BIN_PATH
            "${assimp_BIN_DIR}/assimp_release-dll_x64/"
        )
    endif()
endif()

source_group(source  FILES ${MESH_BUILDER_SRC_FILES})

include_directories(${MESH_BUILDER_INCLUDE_DIRS})
link_directories(${MESH_BUILDER_LIB_DIRS})
add_library(mesh_builder SHARED ${MESH_BUILDER_SRC_FILES})
target_link_libraries(mesh_builder debug ${PROTOBUF_LIBRARY_DEBUG} lua52 crypto proto minfs)
target_link_libraries(mesh_builder optimized ${PROTOBUF_LIBRARY} lua52 crypto proto minfs)

set_target_properties(mesh_builder PROPERTIES PREFIX "")
set_target_properties(mesh_builder PROPERTIES OUTPUT_NAME mesh)

if (PLATFORM_LINUX)
    ADD_CUSTOM_COMMAND(
        TARGET shader_builder
        POST_BUILD
        WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND "ln"
        ARGS -fs "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/mesh.so" mesh.so
    )
endif()
