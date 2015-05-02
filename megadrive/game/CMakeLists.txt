cmake_minimum_required(VERSION 2.8)

set(MD_GAME_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( MD_GAME_INCLUDE_DIRS
    ${LIBHEART_INCLUDES}
    "${MD_GAME_SOURCE_DIR}/src"
)

file(GLOB MD_GAME_HDR_FILES
	"${MD_GAME_SOURCE_DIR}/src/*.h"
)
file(GLOB MD_GAME_SRC_FILES
    "${MD_GAME_SOURCE_DIR}/src/*.cpp"
)

include_directories(${MD_GAME_INCLUDE_DIRS})
link_directories(${LIBHEART_LIB_DIRS})
add_executable(md_game WIN32 ${MD_GAME_HDR_FILES} ${MD_GAME_SRC_FILES} ${MD_GAME_PRECOMPILED_SRC_FILES})
target_link_libraries(md_game heart)