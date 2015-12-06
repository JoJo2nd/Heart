cmake_minimum_required(VERSION 2.8)

set(UTILS_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( UTILS_INCLUDE_DIRS
    "${UTILS_SRC_DIR}/src"
)

file(GLOB UTILS_HDR_FILES
    "${UTILS_SRC_DIR}/src/*.h"
)

file(GLOB UTILS_SRC_FILES
    "${UTILS_SRC_DIR}/src/*.c"
	"${UTILS_SRC_DIR}/src/*.cpp"
)

source_group(include FILES ${UTILS_HDR_FILES})
source_group(source  FILES ${UTILS_SRC_FILES})

include_directories(${UTILS_INCLUDE_DIRS})
add_library(utils ${UTILS_SRC_FILES} ${UTILS_HDR_FILES})
target_link_libraries(utils ${PROFILE_LIBS})
