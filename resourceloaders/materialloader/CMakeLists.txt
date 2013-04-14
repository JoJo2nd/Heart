cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.53.0 REQUIRED COMPONENTS filesystem system)

if (MSVC)
    #add_definitions(/wd"4244")
    #add_definitions(/wd"4146")
    #add_definitions(/wd"4305")
endif()

set(MATERIAL_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( MATERIAL_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
    "${MATERIAL_BUILDER_SRC_DIR}/include"
)
set( MATERIAL_BUILDER_HDR_FILES 
    "${MATERIAL_BUILDER_SRC_DIR}/include/materialloader.h"
)
set( MATERIAL_BUILDER_SRC_FILES
    "${MATERIAL_BUILDER_SRC_DIR}/src/materialloader.cpp"
)
set(MATERIAL_BUILDER_LIB_DIRS
	"${EXTERNAL_SRC_ROOT}/openal/libs/Win32"
)

source_group(include FILES ${MATERIAL_BUILDER_HDR_FILES})
source_group(source  FILES ${MATERIAL_BUILDER_SRC_FILES})

include_directories(${MATERIAL_BUILDER_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
link_directories(${MATERIAL_BUILDER_LIB_DIRS})
add_library(material_builder SHARED ${MATERIAL_BUILDER_SRC_FILES} ${MATERIAL_BUILDER_HDR_FILES})
target_link_libraries(material_builder lua crypto ${Boost_LIBRARIES} ${PLATFORM_LIBS})

set_target_properties(material_builder PROPERTIES OUTPUT_NAME material)

ADD_CUSTOM_COMMAND(
	TARGET material_builder
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" material "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/builder/material"
)