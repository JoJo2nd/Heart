cmake_minimum_required(VERSION 2.8)

set(PBSCAN_SRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( PBSCAN_INCLUDE_DIRS
  "${PBSCAN_SRC_DIR}/src"
    "${TOOLS_SRC_ROOT}/proto/src"
    "${PROTOBUF_INCLUDE_DIR}"
    "${GENERATED_PROTO_FULL_INC_DIRS}"
    "${GETOPT_PORT_INCLUDE_DIR}"
)

file(GLOB PBSCAN_SRC_FILES
    "${PBSCAN_SRC_DIR}/src/*.h"
    "${PBSCAN_SRC_DIR}/src/*.cpp"
)


source_group(include FILES ${PBSCAN_HDR_FILES})
source_group(source  FILES ${PBSCAN_SRC_FILES})

set(ALL_SRC_FILES
        ${PBSCAN_SRC_FILES}
)

if (MSVC)
    add_definitions(/WX) # Warnings as errors
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

include_directories(${PBSCAN_INCLUDE_DIRS})
link_directories(
    ${LIBHEART_LIB_DIRS}
)

add_executable(pbscan ${ALL_SRC_FILES})
#target_link_libraries(pbscan ${PROTOBUF_LIBRARY} crypto lua52 getopt_port proto_full)
target_link_libraries(pbscan crypto getopt_port proto_full)