cmake_minimum_required(VERSION 2.8)

set(PZ_GAME_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( PZ_GAME_INCLUDE_DIRS
    ${LIBHEART_INCLUDES}
    "${PZ_GAME_SOURCE_DIR}/src"
)

file(GLOB PZ_GAME_HDR_FILES
	"${PZ_GAME_SOURCE_DIR}/src/*.h"
)
file(GLOB PZ_GAME_SRC_FILES
    "${PZ_GAME_SOURCE_DIR}/src/*.cpp"
)

include_directories(${PZ_GAME_INCLUDE_DIRS})
link_directories(${LIBHEART_LIB_DIRS})
add_executable(project_z WIN32 ${PZ_GAME_HDR_FILES} ${PZ_GAME_SRC_FILES} ${PZ_GAME_PRECOMPILED_SRC_FILES})
target_link_libraries(project_z heart)
