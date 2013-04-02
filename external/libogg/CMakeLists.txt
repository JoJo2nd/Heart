cmake_minimum_required(VERSION 2.8)

if (MSVC)
    #add_definitions(/wd"4244")
    #add_definitions(/wd"4146")
    #add_definitions(/wd"4305")
endif()

set(LIBOGG_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( LIBOGG_INCLUDE_DIRS
    "${LIBOGG_SRC_DIR}/include"
)

set( LIBOGG_HDR_FILES
    "${LIBOGG_SRC_DIR}/include/ogg/ogg.h"
    "${LIBOGG_SRC_DIR}/include/ogg/os_types.h"
)

set( LIBOGG_SRC_FILES
    "${LIBOGG_SRC_DIR}/src/bitwise.c"
    "${LIBOGG_SRC_DIR}/src/framing.c"
)

source_group(include FILES ${LIBOGG_HDR_FILES})
source_group(source  FILES ${LIBOGG_SRC_FILES})

include_directories(${LIBOGG_INCLUDE_DIRS})
add_library(libogg ${LIBOGG_SRC_FILES} ${LIBOGG_HDR_FILES})