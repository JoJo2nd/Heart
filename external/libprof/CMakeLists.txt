cmake_minimum_required(VERSION 2.8)

ENABLE_LANGUAGE(ASM_MASM)

set(LIBPROF_SRC_DIR
    "${EXTERNAL_SRC_ROOT}/libprof"
)


file(GLOB_RECURSE LIBPROF_SRC_FILES 
    "${LIBPROF_SRC_DIR}/src/*.asm"
    "${LIBPROF_SRC_DIR}/src/*.c"
    "${LIBPROF_SRC_DIR}/src/*.h"
)

#set_source_files_properties(${HEART_NON_UNITY_FILES} PROPERTIES HEADER_FILE_ONLY 1)
source_group(source FILES ${LIBPROF_SRC_FILES})

message("${LIBPROF_SRC_FILES}")

#include_directories(${ENET_INCLUDE_DIR})
#link_directories(${MATERIAL_BUILDER_LIB_DIRS})
add_library(libprof ${LIBPROF_SRC_FILES})
#target_link_libraries(enet ws2_32)