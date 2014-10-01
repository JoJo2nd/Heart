cmake_minimum_required(VERSION 2.8)

set(MINFS_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( MINFS_INCLUDE_DIRS
    "${MINFS_SRC_DIR}/src"
)

file(GLOB MINFS_HDR_FILES
    "${MINFS_SRC_DIR}/src/*.h"
)

file(GLOB MINFS_SRC_FILES
    "${MINFS_SRC_DIR}/src/*.c"
)

source_group(include FILES ${MINFS_HDR_FILES})
source_group(source  FILES ${MINFS_SRC_FILES})

include_directories(${MINFS_INCLUDE_DIRS})
add_library(minfs ${MINFS_SRC_FILES} ${MINFS_HDR_FILES})
target_link_libraries(minfs)