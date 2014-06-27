cmake_minimum_required(VERSION 2.8)

ENABLE_PROFILE_IF_SET()

set(ENET_SRC_DIR
    "${EXTERNAL_SRC_ROOT}/enet"
)

file(GLOB_RECURSE ENET_SRC_FILES 
    "${ENET_SRC_DIR}/include/*.h"
    "${ENET_SRC_DIR}/*.c"
)
if (PLATFORM_WINDOWS)
	list(REMOVE_ITEM ENET_SRC_FILES "src/unix.c")
endif()
if (PLATFORM_LINUX)
	add_definitions(-DHAS_SOCKLEN_T)
	list(REMOVE_ITEM ENET_SRC_FILES "src/win32.c")
endif()

source_group(source FILES ${ENET_SRC_FILES})

include_directories(${ENET_INCLUDE_DIR})
add_library(enet ${ENET_SRC_FILES})
set(ENET_LIBS ${PROFILE_LIBS})
if (PLATFORM_WINDOWS)
    set(ENET_LIBS "${ENET_LIBS} ws2_32 Winmm")
endif()

target_link_libraries(enet ${ENET_LIBS})