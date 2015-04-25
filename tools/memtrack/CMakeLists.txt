cmake_minimum_required(VERSION 2.8)

set(MEMTRACK_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( MEMTRACK_INCLUDE_DIRS
    "${MEMTRACK_SRC_DIR}/include"
    "${GETOPT_PORT_INCLUDE_DIR}"
    "${MEMTRACKERLIB_SRC_DIR}"
)

file(GLOB MEMTRACK_SRC_FILES
    "${MEMTRACK_SRC_DIR}/src/*.h"
    "${MEMTRACK_SRC_DIR}/src/*.c"
    "${MEMTRACK_SRC_DIR}/src/*.cpp"
)

source_group(source  FILES ${MEMTRACK_SRC_FILES})

if (PLATFORM_WINDOWS)
	add_definitions(/WX) # Warnings as errors
    set(PLATFORM_LIBS dbghelp)
endif()

include_directories(${MEMTRACK_INCLUDE_DIRS})
add_executable(mem_track ${MEMTRACK_SRC_FILES} ${MEMTRACK_HDR_FILES})
target_link_libraries(mem_track crypto getopt_port ${PLATFORM_LIBS})
