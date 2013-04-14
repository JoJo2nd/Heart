cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.53.0 REQUIRED COMPONENTS filesystem system)

if (MSVC)
    #add_definitions(/wd"4244")
    #add_definitions(/wd"4146")
    #add_definitions(/wd"4305")
endif()

set(MESH_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( MESH_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
    "${MESH_BUILDER_SRC_DIR}/include"
)
set( MESH_BUILDER_HDR_FILES 
    "${MESH_BUILDER_SRC_DIR}/include/meshloader.h"
	"${MESH_BUILDER_SRC_DIR}/include/MeshDataStructs.h"
)
set( MESH_BUILDER_SRC_FILES
    "${MESH_BUILDER_SRC_DIR}/src/meshloader.cpp"
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

source_group(include FILES ${MESH_BUILDER_HDR_FILES})
source_group(source  FILES ${MESH_BUILDER_SRC_FILES})

include_directories(${MESH_BUILDER_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
link_directories(${MESH_BUILDER_LIB_DIRS})
add_library(mesh_builder SHARED ${MESH_BUILDER_SRC_FILES} ${MESH_BUILDER_HDR_FILES})
target_link_libraries(mesh_builder lua crypto ${Boost_LIBRARIES} ${PLATFORM_LIBS})

set_target_properties(mesh_builder PROPERTIES OUTPUT_NAME mesh)

ADD_CUSTOM_COMMAND(
	TARGET mesh_builder
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" mesh "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/builder/mesh"
)