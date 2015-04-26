cmake_minimum_required(VERSION 2.8)

set(IMGUI_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( IMGUI_INCLUDE_DIRS
    "${IMGUI_SRC_DIR}"
)

file(GLOB IMGUI_HDR_FILES
    "${IMGUI_SRC_DIR}/src/*.h"
)

file(GLOB IMGUI_SRC_FILES
    "${IMGUI_SRC_DIR}/src/*.c"
    "${IMGUI_SRC_DIR}/src/*.cpp"
)

source_group(include FILES ${IMGUI_HDR_FILES})
source_group(source  FILES ${IMGUI_SRC_FILES})

include_directories(${IMGUI_INCLUDE_DIRS})
add_library(imgui ${IMGUI_SRC_FILES} ${IMGUI_HDR_FILES})
target_link_libraries(imgui ${PROFILE_LIBS})