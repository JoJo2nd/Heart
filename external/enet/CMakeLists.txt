cmake_minimum_required(VERSION 2.8)

ENABLE_PROFILE_IF_SET()

set(ENET_SRC_DIR
    "${EXTERNAL_SRC_ROOT}/enet"
)

file(GLOB_RECURSE ENET_SRC_FILES 
    "${ENET_SRC_DIR}/include/*.h"
    "${ENET_SRC_DIR}/*.c"
)
list(REMOVE_ITEM ENET_SRC_FILES "src/unix.c")

source_group(source FILES ${ENET_SRC_FILES})

include_directories(${ENET_INCLUDE_DIR})
#link_directories(${MATERIAL_BUILDER_LIB_DIRS})
add_library(enet ${ENET_SRC_FILES})
target_link_libraries(enet ws2_32 Winmm ${PROFILE_LIBS})