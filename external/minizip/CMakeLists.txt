cmake_minimum_required(VERSION 2.8)

set(MINIZIP_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( MINIZIP_INCLUDE_DIRS
    "${MINIZIP_SRC_DIR}/include"
    "${EXTERNAL_SRC_ROOT}/zlib/include"
)

set( MINIZIP_HDR_FILES
    "${MINIZIP_SRC_DIR}/include/crypt.h"
    "${MINIZIP_SRC_DIR}/include/ioapi.h"
    "${MINIZIP_SRC_DIR}/include/mztools.h"
    "${MINIZIP_SRC_DIR}/include/unzip.h"
    "${MINIZIP_SRC_DIR}/include/zip.h"
)

set( MINIZIP_SRC_FILES
    "${MINIZIP_SRC_DIR}/src/ioapi.c"
    "${MINIZIP_SRC_DIR}/src/mztools.c"
    "${MINIZIP_SRC_DIR}/src/unzip.c"
    "${MINIZIP_SRC_DIR}/src/zip.c"
)

if (PLATFORM_WINDOWS)
    set( MINIZIP_HDR_FILES
        ${MINIZIP_HDR_FILES}
        "${MINIZIP_SRC_DIR}/include/iowin32.h"
    )
    set( MINIZIP_SRC_FILES
        ${MINIZIP_SRC_FILES}
        "${MINIZIP_SRC_DIR}/src/iowin32.c"
    )
endif()

source_group(include FILES ${MINIZIP_HDR_FILES})
source_group(source  FILES ${MINIZIP_SRC_FILES})

include_directories(${MINIZIP_INCLUDE_DIRS})
add_library(minizip ${MINIZIP_SRC_FILES} ${MINIZIP_HDR_FILES})
target_link_libraries(minizip zlib)