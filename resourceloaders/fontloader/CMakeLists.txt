cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.53.0 REQUIRED COMPONENTS filesystem system)

if (MSVC)
    #add_definitions(/wd"4244")
    #add_definitions(/wd"4146")
    #add_definitions(/wd"4305")
endif()

set(FONT_BUILDER_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set( FONT_BUILDER_INCLUDE_DIRS
	${LIBHEART_INCLUDES}
    "${FONT_BUILDER_SRC_DIR}/include"
)
set( FONT_BUILDER_HDR_FILES 
    "${FONT_BUILDER_SRC_DIR}/include/fontloader.h"
)
set( FONT_BUILDER_SRC_FILES
    "${FONT_BUILDER_SRC_DIR}/src/fontloader.cpp"
)
set(FONT_BUILDER_LIB_DIRS
	"${EXTERNAL_SRC_ROOT}/openal/libs/Win32"
)
source_group(include FILES ${FONT_BUILDER_HDR_FILES})
source_group(source  FILES ${FONT_BUILDER_SRC_FILES})

if (MSVC)
	add_definitions(/WX) # Warnings as errors
endif()

include_directories(${FONT_BUILDER_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
link_directories(${FONT_BUILDER_LIB_DIRS})
add_library(font_builder SHARED ${FONT_BUILDER_SRC_FILES} ${FONT_BUILDER_HDR_FILES})
target_link_libraries(font_builder lua52 crypto ${Boost_LIBRARIES} ${PLATFORM_LIBS})

set_target_properties(font_builder PROPERTIES OUTPUT_NAME font)

ADD_CUSTOM_COMMAND(
	TARGET font_builder
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/build/deploy_scripts/deploy_lib.bat" font "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}" "${CMAKE_SOURCE_DIR}/deploy/builder/font"
)