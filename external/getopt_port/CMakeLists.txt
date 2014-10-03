cmake_minimum_required(VERSION 2.8)

set(GETOPT_PORT_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( GETOPT_PORT_INCLUDE_DIRS
    "${GETOPT_PORT_SRC_DIR}"
)

file(GLOB GETOPT_PORT_HDR_FILES
    "${GETOPT_PORT_SRC_DIR}/*.h"
)

file(GLOB GETOPT_PORT_SRC_FILES
    "${GETOPT_PORT_SRC_DIR}/*.c"
    "${GETOPT_PORT_SRC_DIR}/*.c"
)

source_group(include FILES ${GETOPT_PORT_HDR_FILES})
source_group(source  FILES ${GETOPT_PORT_SRC_FILES})

include_directories(${GETOPT_PORT_INCLUDE_DIRS})
add_library(getopt_port ${GETOPT_PORT_SRC_FILES} ${GETOPT_PORT_HDR_FILES})
target_link_libraries(getopt_port ${PROFILE_LIBS})