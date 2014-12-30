cmake_minimum_required(VERSION 2.8)

set(MEMTRACKERLIB_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( MEMTRACKERLIB_INCLUDE_DIRS
    "${MEMTRACKERLIB_SRC_DIR}/include"
)

file(GLOB MEMTRACKERLIB_SRC_FILES
    "${MEMTRACKERLIB_SRC_DIR}/src/*.h"
    "${MEMTRACKERLIB_SRC_DIR}/src/*.c"
    "${MEMTRACKERLIB_SRC_DIR}/src/*.cpp"
)

source_group(source FILES ${MEMTRACKERLIB_SRC_FILES})

if (PLATFORM_WINDOWS)
	add_definitions(/WX) # Warnings as errors
    set(PLATFORM_LIBS dbghelp)
endif()

include_directories(${MEMTRACKERLIB_INCLUDE_DIRS})
add_library(memtrackerlib SHARED ${MEMTRACKERLIB_SRC_FILES} ${MEMTRACKERLIB_HDR_FILES})
target_link_libraries(memtrackerlib ${PLATFORM_LIBS})
