cmake_minimum_required(VERSION 2.8)

if (MSVC)
    add_definitions(/wd"4996")
endif()

set(ZLIB_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( ZLIB_INCLUDE_DIRS
    "${ZLIB_SRC_DIR}/include"
)

set( ZLIB_HDR_FILES
    "${ZLIB_SRC_DIR}/include/crc32.h"
    "${ZLIB_SRC_DIR}/include/deflate.h"
    "${ZLIB_SRC_DIR}/include/inffast.h"
    "${ZLIB_SRC_DIR}/include/inffixed.h"
    "${ZLIB_SRC_DIR}/include/inflate.h"
    "${ZLIB_SRC_DIR}/include/inftrees.h"
    "${ZLIB_SRC_DIR}/include/trees.h"
    "${ZLIB_SRC_DIR}/include/zconf.h"
    "${ZLIB_SRC_DIR}/include/zlib.h"
    "${ZLIB_SRC_DIR}/include/zutil.h"
)

set( ZLIB_SRC_FILES
    "${ZLIB_SRC_DIR}/src/adler32.c"
    "${ZLIB_SRC_DIR}/src/compress.c"
    "${ZLIB_SRC_DIR}/src/crc32.c"
    "${ZLIB_SRC_DIR}/src/deflate.c"
    "${ZLIB_SRC_DIR}/src/gzio.c"
    "${ZLIB_SRC_DIR}/src/infback.c"
    "${ZLIB_SRC_DIR}/src/inffast.c"
    "${ZLIB_SRC_DIR}/src/inflate.c"
    "${ZLIB_SRC_DIR}/src/inftrees.c"
    "${ZLIB_SRC_DIR}/src/minigzip.c"
    "${ZLIB_SRC_DIR}/src/trees.c"
    "${ZLIB_SRC_DIR}/src/uncompr.c"
    "${ZLIB_SRC_DIR}/src/zutil.c"
)

source_group(include FILES ${ZLIB_HDR_FILES})
source_group(source  FILES ${ZLIB_SRC_FILES})

include_directories(${ZLIB_INCLUDE_DIRS})
add_library(zlib ${ZLIB_SRC_FILES} ${ZLIB_HDR_FILES})