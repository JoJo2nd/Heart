cmake_minimum_required(VERSION 2.8)

ENABLE_PROFILE_IF_SET()

if (MSVC)
    #add_definitions(/wd"4244")
    #add_definitions(/wd"4146")
    #add_definitions(/wd"4305")
endif()

set(CRYPTO_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( CRYPTO_INCLUDE_DIRS
    "${CRYPTO_SRC_DIR}/src"
)

file(GLOB CRYPTO_HDR_FILES
    "${CRYPTO_SRC_DIR}/src/*.h"
)

file(GLOB CRYPTO_SRC_FILES
    "${CRYPTO_SRC_DIR}/src/*.c"
	"${CRYPTO_SRC_DIR}/src/*.cpp"
)

source_group(include FILES ${CRYPTO_HDR_FILES})
source_group(source  FILES ${CRYPTO_SRC_FILES})

include_directories(${CRYPTO_INCLUDE_DIRS})
add_library(crypto ${CRYPTO_SRC_FILES} ${CRYPTO_HDR_FILES})
target_link_libraries(crypto ${PROFILE_LIBS})