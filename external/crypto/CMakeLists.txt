cmake_minimum_required(VERSION 2.8)

if (MSVC)
    #add_definitions(/wd"4244")
    #add_definitions(/wd"4146")
    #add_definitions(/wd"4305")
endif()

set(CRYPTO_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( CRYPTO_INCLUDE_DIRS
    "${CRYPTO_SRC_DIR}/include"
)

set( CRYPTO_HDR_FILES
    "${CRYPTO_SRC_DIR}/include/cryptoCommon.h"
    "${CRYPTO_SRC_DIR}/include/cryptoCRC32.h"
    "${CRYPTO_SRC_DIR}/include/cryptoMD5.h"
)

set( CRYPTO_SRC_FILES
    "${CRYPTO_SRC_DIR}/src/cryptoCRC32.c"
    "${CRYPTO_SRC_DIR}/src/cryptoMD5.c"
)

source_group(include FILES ${CRYPTO_HDR_FILES})
source_group(source  FILES ${CRYPTO_SRC_FILES})

include_directories(${CRYPTO_INCLUDE_DIRS})
add_library(crypto ${CRYPTO_SRC_FILES} ${CRYPTO_HDR_FILES})