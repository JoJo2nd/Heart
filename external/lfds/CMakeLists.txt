cmake_minimum_required(VERSION 2.8)

set(LFDS_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( LFDS_INCLUDE_DIRS
    "${LFDS_SRC_DIR}/include"
    "${LFDS_SRC_DIR}/src"
)

file(GLOB LFDS_HDR_FILES
    "${LFDS_SRC_DIR}/include/lfds/*.h"
)

file(GLOB LFDS_SRC_FILES
    "${LFDS_SRC_DIR}/src/*.c"
    "${LFDS_SRC_DIR}/src/*.cpp"
    "${LFDS_SRC_DIR}/src/lfds/*.cpp"
)

source_group(include FILES ${LFDS_HDR_FILES})
source_group(source  FILES ${LFDS_SRC_FILES})

include_directories(${LFDS_INCLUDE_DIRS})
add_library(lfds ${LFDS_SRC_FILES} ${LFDS_HDR_FILES})
target_link_libraries(lfds ${PROFILE_LIBS})
